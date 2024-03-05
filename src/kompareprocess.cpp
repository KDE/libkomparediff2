/*
    SPDX-FileCopyrightText: 2001-2005,2009 Otto Bruggeman <bruggie@gmail.com>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2007-2008 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kompareprocess.h"

// lib
#include "diffsettings.h"
#include <komparediff2_logging.h>
// KF
#include <KIO/Global>
// Qt
#include <QDir>
#include <QStringList>
#include <QUrl>

namespace
{
/// TODO: This should be replaced to QDir::relativeFilePath
static QString constructRelativePath(const QString &from, const QString &to)
{
    QUrl fromURL(from);
    QUrl toURL(to);
    QUrl root;
    int upLevels = 0;

    // Find a common root.
    root = fromURL;
    while (root.isValid() && !root.isParentOf(toURL)) {
        root = KIO::upUrl(root);
        ++upLevels;
    }

    if (!root.isValid())
        return to;

    QString relative;
    for (; upLevels > 0; --upLevels) {
        relative += QStringLiteral("../");
    }

    relative += QString(to).remove(0, root.path().length());
    return relative;
}
}

KompareProcess::KompareProcess(KompareDiff2::DiffSettings *diffSettings,
                               KompareDiff2::DiffMode diffMode,
                               const QString &source,
                               const QString &destination,
                               const QString &dir,
                               KompareDiff2::Mode mode)
    : KProcess()
    , m_diffSettings(diffSettings)
    , m_diffMode(diffMode)
    , m_mode(mode)
{
    // connect the signal that indicates that the process has exited
    connect(this, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &KompareProcess::slotFinished);

    setEnv(QStringLiteral("LANG"), QStringLiteral("C"));

    // Write command and options
    if (m_diffMode == KompareDiff2::Default) {
        writeDefaultCommandLine();
    } else {
        writeCommandLine();
    }

    if (!dir.isEmpty()) {
        setWorkingDirectory(dir);
    }

    // Write file names
    *this << QStringLiteral("--");

    // Add the option for diff to read from stdin(QIODevice::write), and save a pointer to the string
    if (m_mode == KompareDiff2::ComparingStringFile) {
        *this << QStringLiteral("-");
        m_customString = source;
    } else {
        *this << constructRelativePath(dir, source);
    }

    if (m_mode == KompareDiff2::ComparingFileString) {
        *this << QStringLiteral("-");
        m_customString = destination;
    } else {
        *this << constructRelativePath(dir, destination);
    }
}

void KompareProcess::writeDefaultCommandLine()
{
    if (!m_diffSettings || m_diffSettings->m_diffProgram.isEmpty()) {
        *this << QStringLiteral("diff") << QStringLiteral("-dr");
    } else {
        *this << m_diffSettings->m_diffProgram << QStringLiteral("-dr");
    }

    *this << QStringLiteral("-U") << QString::number(m_diffSettings->m_linesOfContext);
}

void KompareProcess::writeCommandLine()
{
    // load the executable into the KProcess
    if (m_diffSettings->m_diffProgram.isEmpty()) {
        qCDebug(KOMPAREDIFF2_LOG) << "Using the first diff in the path...";
        *this << QStringLiteral("diff");
    } else {
        qCDebug(KOMPAREDIFF2_LOG) << "Using a user specified diff, namely: " << m_diffSettings->m_diffProgram;
        *this << m_diffSettings->m_diffProgram;
    }

    switch (m_diffSettings->m_format) {
    case KompareDiff2::Unified:
        *this << QStringLiteral("-U") << QString::number(m_diffSettings->m_linesOfContext);
        break;
    case KompareDiff2::Context:
        *this << QStringLiteral("-C") << QString::number(m_diffSettings->m_linesOfContext);
        break;
    case KompareDiff2::RCS:
        *this << QStringLiteral("-n");
        break;
    case KompareDiff2::Ed:
        *this << QStringLiteral("-e");
        break;
    case KompareDiff2::SideBySide:
        *this << QStringLiteral("-y");
        break;
    case KompareDiff2::Normal:
    case KompareDiff2::UnknownFormat:
    default:
        break;
    }

    if (m_diffSettings->m_largeFiles
// default diff does not have -H on OpenBSD
// so don't pass this option unless the user overrode the default program
#if defined(__OpenBSD__)
        && !m_diffSettings->m_diffProgram.isEmpty()
#endif
    ) {
        *this << QStringLiteral("-H");
    }

    if (m_diffSettings->m_ignoreWhiteSpace) {
        *this << QStringLiteral("-b");
    }

    if (m_diffSettings->m_ignoreAllWhiteSpace) {
        *this << QStringLiteral("-w");
    }

    if (m_diffSettings->m_ignoreEmptyLines) {
        *this << QStringLiteral("-B");
    }

    if (m_diffSettings->m_ignoreChangesDueToTabExpansion) {
        *this << QStringLiteral("-E");
    }

    if (m_diffSettings->m_createSmallerDiff) {
        *this << QStringLiteral("-d");
    }

    if (m_diffSettings->m_ignoreChangesInCase) {
        *this << QStringLiteral("-i");
    }

    if (m_diffSettings->m_ignoreRegExp && !m_diffSettings->m_ignoreRegExpText.isEmpty()) {
        *this << QStringLiteral("-I") << m_diffSettings->m_ignoreRegExpText;
    }

    if (m_diffSettings->m_showCFunctionChange) {
        *this << QStringLiteral("-p");
    }

    if (m_diffSettings->m_convertTabsToSpaces) {
        *this << QStringLiteral("-t");
    }

    if (m_diffSettings->m_recursive) {
        *this << QStringLiteral("-r");
    }

    if (m_diffSettings->m_newFiles) {
        *this << QStringLiteral("-N");
    }

// This option is more trouble than it is worth... please do not ever enable it unless you want really weird crashes
//  if ( m_diffSettings->m_allText )
//  {
//      *this << QStringLiteral("-a");
//  }

    if (m_diffSettings->m_excludeFilePattern) {
        for (const QString &it : std::as_const(m_diffSettings->m_excludeFilePatternList)) {
            *this << QStringLiteral("-x") << it;
        }
    }

    if (m_diffSettings->m_excludeFilesFile && !m_diffSettings->m_excludeFilesFileURL.isEmpty()) {
        *this << QStringLiteral("-X") << m_diffSettings->m_excludeFilesFileURL;
    }
}

KompareProcess::~KompareProcess() = default;

void KompareProcess::setEncoding(const QString &encoding)
{
    if (!encoding.compare(QLatin1String("default"), Qt::CaseInsensitive)) {
        m_textDecoder = QStringDecoder(QStringDecoder::System);
        m_textEncoder = QStringEncoder(QStringEncoder::System);
    } else {
        m_textDecoder = QStringDecoder(encoding.toUtf8().constData());
        m_textEncoder = QStringEncoder(encoding.toUtf8().constData());
        if (!m_textDecoder.isValid() || !m_textEncoder.isValid()) {
            qCDebug(KOMPAREDIFF2_LOG) << "Using locale codec as backup...";
            m_textDecoder = QStringDecoder(QStringDecoder::System);
            m_textEncoder = QStringEncoder(QStringEncoder::System);
        }
    }
}

void KompareProcess::start()
{
#ifndef NDEBUG
    QString cmdLine;
    const QStringList program = KProcess::program();
    for (const QString &arg : program)
        cmdLine += QLatin1Char('\"') + arg + QLatin1String("\" ");
    qCDebug(KOMPAREDIFF2_LOG) << cmdLine;
#endif
    setOutputChannelMode(SeparateChannels);
    setNextOpenMode(QIODevice::ReadWrite);
    KProcess::start();

    // If we have a string to compare against input it now
    if ((m_mode == KompareDiff2::ComparingStringFile) || (m_mode == KompareDiff2::ComparingFileString))
        write(m_textEncoder.encode(m_customString));
    closeWriteChannel();
}

void KompareProcess::slotFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    // add all output to m_stdout/m_stderr
    m_textDecoder.resetState();
    m_stdout = m_textDecoder.decode(readAllStandardOutput());
    m_textDecoder.resetState();
    m_stderr = m_textDecoder.decode(readAllStandardError());

    // exit code of 0: no differences
    //              1: some differences
    //              2: error but there may be differences !
    qCDebug(KOMPAREDIFF2_LOG) << "Exited with exit code : " << exitCode;
    Q_EMIT diffHasFinished(exitStatus == NormalExit && exitCode != 0);
}

#include "moc_kompareprocess.cpp"

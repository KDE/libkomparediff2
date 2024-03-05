/*
    SPDX-FileCopyrightText: 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2008 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREPROCESS_H
#define KOMPAREPROCESS_H

// lib
#include "global.h"
// KF
#include <KProcess>
// Qt
#include <QStringDecoder>
#include <QStringEncoder>
// Std
#include <memory>

namespace KompareDiff2
{
class DiffSettings;
}

class KompareProcess : public KProcess
{
    Q_OBJECT

public:
    KompareProcess(KompareDiff2::DiffSettings *diffSettings,
                   KompareDiff2::DiffMode mode,
                   const QString &source,
                   const QString &destination,
                   const QString &directory = QString(),
                   KompareDiff2::Mode = KompareDiff2::UnknownMode);
    ~KompareProcess() override;

public:
    void start();

    QString diffOutput() const
    {
        return m_stdout;
    }
    QString stdOut() const
    {
        return m_stdout;
    }
    QString stdErr() const
    {
        return m_stderr;
    }

    void setEncoding(const QString &encoding);

Q_SIGNALS:
    void diffHasFinished(bool finishedNormally);

private:
    void writeDefaultCommandLine();
    void writeCommandLine();

private Q_SLOTS:
    void slotFinished(int, QProcess::ExitStatus);

private:
    KompareDiff2::DiffSettings *m_diffSettings;
    const KompareDiff2::DiffMode m_diffMode;
    const KompareDiff2::Mode m_mode;
    QString m_customString; // Used when a comparison between a file and a string is requested
    QString m_stdout;
    QString m_stderr;
    QStringDecoder m_textDecoder;
    QStringEncoder m_textEncoder;
};

#endif

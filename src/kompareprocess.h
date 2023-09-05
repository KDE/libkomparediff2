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
// Std
#include <memory>

class QTextDecoder;
class QTextCodec;

namespace KompareDiff2 {
class DiffSettings;
}

class KompareProcess : public KProcess
{
    Q_OBJECT

public:
    KompareProcess(KompareDiff2::DiffSettings* diffSettings,
                   enum KompareDiff2::DiffMode mode, const QString& source,
                   const QString& destination, const QString& directory = QString(), enum KompareDiff2::Mode = KompareDiff2::UnknownMode);
    ~KompareProcess() override;

    void start();

    QString diffOutput() { return m_stdout; }
    QString stdOut()     { return m_stdout; }
    QString stdErr()     { return m_stderr; }

    void setEncoding(const QString& encoding);

Q_SIGNALS:
    void diffHasFinished(bool finishedNormally);

protected:
    void writeDefaultCommandLine();
    void writeCommandLine();

protected Q_SLOTS:
    void slotFinished(int, QProcess::ExitStatus);

private:
    KompareDiff2::DiffSettings* m_diffSettings;
    enum KompareDiff2::DiffMode m_mode;
    const QString*         m_customString; // Used when a comparison between a file and a string is requested
    QString                m_stdout;
    QString                m_stderr;
    std::unique_ptr<QTextDecoder>  m_textDecoder;
    QTextCodec*            m_codec = nullptr;
};

#endif

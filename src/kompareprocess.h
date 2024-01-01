/*
SPDX-FileCopyrightText: 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
SPDX-FileCopyrightText: 2008 Kevin Kofler <kevin.kofler@chello.at>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREPROCESS_H
#define KOMPAREPROCESS_H

#include <KProcess>

#include "kompare.h"

class QTextDecoder;
class QTextCodec;

class DiffSettings;

class KompareProcess : public KProcess
{
    Q_OBJECT

public:
    KompareProcess(DiffSettings* diffSettings, enum Kompare::DiffMode mode, const QString& source,
                   const QString& destination, const QString& directory = QString(), enum Kompare::Mode = Kompare::UnknownMode);
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
    DiffSettings*          m_diffSettings;
    const Kompare::DiffMode m_diffMode;
    const Kompare::Mode    m_mode;
    QString                m_customString; // Used when a comparison between a file and a string is requested
    QString                m_stdout;
    QString                m_stderr;
    QTextDecoder*          m_textDecoder;
    QTextCodec*            m_codec;
};

#endif

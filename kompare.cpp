/*
SPDX-FileCopyrightText: 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kompare.h"

Kompare::Info::Info(Kompare::Mode _mode, Kompare::DiffMode _diffMode, Kompare::Format _format, Kompare::Generator _generator, QUrl _source, QUrl _destination, QString _localSource, QString _localDestination, QTemporaryDir* _sourceQTempDir, QTemporaryDir* _destinationQTempDir, uint _depth, bool _applied)
    : mode(_mode)
    , diffMode(_diffMode)
    , format(_format)
    , generator(_generator)
    , source(_source)
    , destination(_destination)
    , localSource(_localSource)
    , localDestination(_localDestination)
    , sourceQTempDir(_sourceQTempDir)
    , destinationQTempDir(_destinationQTempDir)
    , depth(_depth)
    , applied(_applied)
{
}


void Kompare::Info::swapSourceWithDestination()
{
    QUrl url = source;
    source = destination;
    destination = url;

    QString string = localSource;
    localSource = localDestination;
    localDestination = string;

    QTemporaryDir* tmpDir = sourceQTempDir;
    sourceQTempDir = destinationQTempDir;
    destinationQTempDir = tmpDir;
}

/*
SPDX-FileCopyrightText: 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_INFO_H
#define KOMPAREDIFF2_INFO_H

#include "komparediff2_export.h"
#include "global.h"

#include <QUrl>

// Forward declaration needed
class QTemporaryDir;

namespace KompareDiff2
{

/**
 * @class Info info.h <KompareDiff2/Info>
 *
 * Info.
 */
class KOMPAREDIFF2_EXPORT Info
{
public:
    Info(
        enum Mode _mode = UnknownMode,
        enum DiffMode _diffMode = UnknownDiffMode,
        enum Format _format = UnknownFormat,
        enum Generator _generator = UnknownGenerator,
        QUrl _source = QUrl(),
        QUrl _destination = QUrl(),
        const QString& _localSource = QString(),
        const QString& _localDestination = QString(),
        QTemporaryDir* _sourceQTempDir = nullptr,
        QTemporaryDir* _destinationQTempDir = nullptr,
        uint _depth = 0,
        bool _applied = true
    );

    void swapSourceWithDestination();

    enum Mode      mode;
    enum DiffMode  diffMode;
    enum Format    format;
    enum Generator generator;
    QUrl           source;
    QUrl           destination;
    QString        localSource;
    QString        localDestination;
    QTemporaryDir*      sourceQTempDir;
    QTemporaryDir*      destinationQTempDir;
    uint           depth;
    bool           applied;
};

} // End of namespace KompareDiff2

#endif

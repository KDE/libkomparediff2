/*
    SPDX-FileCopyrightText: 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_INFO_H
#define KOMPAREDIFF2_INFO_H

// lib
#include "global.h"
#include "komparediff2_export.h"
// Qt
#include <QUrl>

// Forward declaration needed
class QTemporaryDir;

namespace KompareDiff2
{

/*!
 * \inmodule KompareDiff2
 * \class KompareDiff2::Info
 * \inheaderfile KompareDiff2/Info
 * \brief Info.
 */
class KOMPAREDIFF2_EXPORT Info
{
public:
    /*!
     */
    Info(Mode _mode = UnknownMode,
         DiffMode _diffMode = UnknownDiffMode,
         Format _format = UnknownFormat,
         Generator _generator = UnknownGenerator,
         const QUrl &_source = QUrl(),
         const QUrl &_destination = QUrl(),
         const QString &_localSource = QString(),
         const QString &_localDestination = QString(),
         QTemporaryDir *_sourceQTempDir = nullptr,
         QTemporaryDir *_destinationQTempDir = nullptr,
         uint _depth = 0,
         bool _applied = true);

    /*!
     */
    void swapSourceWithDestination();

    /*!
     */
    Mode mode;
    /*!
     */
    DiffMode diffMode;
    /*!
     */
    Format format;
    /*!
     */
    Generator generator;
    /*!
     */
    QUrl source;
    /*!
     */
    QUrl destination;
    /*!
     */
    QString localSource;
    /*!
     */
    QString localDestination;
    /*!
     */
    QTemporaryDir *sourceQTempDir;
    /*!
     */
    QTemporaryDir *destinationQTempDir;
    /*!
     */
    uint depth;
    /*!
     */
    bool applied;
};

} // End of namespace KompareDiff2

#endif

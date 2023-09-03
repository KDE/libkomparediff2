/*
SPDX-FileCopyrightText: 2001-2004,2009 Otto Bruggeman <bruggie@gmail.com>
SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_DIFFMODEL_P_H
#define KOMPAREDIFF2_DIFFMODEL_P_H

// lib
#include "diffhunk.h"

namespace KompareDiff2
{

class DiffModelPrivate
{
public:
    DiffModelPrivate() = default;
    DiffModelPrivate(const QString& source, const QString& destination);
    ~DiffModelPrivate();

public:
    DiffModelPrivate& operator=(const DiffModelPrivate& other);

public:
    QString source;
    QString destination;

    QString sourcePath;
    QString destinationPath;

    QString sourceFile;
    QString destinationFile;

    QString sourceTimestamp;
    QString destinationTimestamp;

    QString sourceRevision;
    QString destinationRevision;

    DiffHunkList hunks;
    DifferenceList differences;

    int appliedCount = 0;

    int  diffIndex = 0;
    Difference* selectedDifference = nullptr;

    bool blended = false;
};

inline
DiffModelPrivate::DiffModelPrivate(const QString& source, const QString& destination)
    : source(source)
    , destination(destination)
{
}

inline
DiffModelPrivate::~DiffModelPrivate()
{
    selectedDifference = nullptr;

    qDeleteAll(hunks);
    qDeleteAll(differences);
}

inline
DiffModelPrivate& DiffModelPrivate::operator=(const DiffModelPrivate& other)
{
    source = other.source;
    sourcePath = other.sourcePath;
    sourceFile = other.sourceFile;
    sourceTimestamp = other.sourceTimestamp;
    sourceRevision = other.sourceRevision;

    destination = other.destination;
    destinationPath = other.destinationPath;
    destinationFile = other.destinationFile;
    destinationTimestamp = other.destinationTimestamp;
    destinationRevision = other.destinationRevision;

    appliedCount = other.appliedCount;

    diffIndex = other.diffIndex;
    selectedDifference = other.selectedDifference;

    return *this;
}

}

#endif

/*
SPDX-FileCopyrightText: 2001-2004,2009 Otto Bruggeman <bruggie@gmail.com>
SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_DIFFHUNK_P_H
#define KOMPAREDIFF2_DIFFHUNK_P_H

// lib
#include "diffhunk.h"
#include "difference.h"

namespace KompareDiff2
{

class DiffHunkPrivate
{
public:
    DiffHunkPrivate(int sourceLine, int destinationLine, const QString& function, DiffHunk::Type type);

public:
    int sourceLine;
    int destinationLine;
    DifferenceList differences;
    QString function;
    DiffHunk::Type type;
};

inline
DiffHunkPrivate::DiffHunkPrivate(int sourceLine, int destinationLine, const QString& function, DiffHunk::Type type)
    : sourceLine(sourceLine)
    , destinationLine(destinationLine)
    , function(function)
    , type(type)
{
}

}

#endif

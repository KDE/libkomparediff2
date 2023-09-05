/*
    SPDX-FileCopyrightText: 2001-2004,2009 Otto Bruggeman <bruggie@gmail.com>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_DIFFERENCE_P_H
#define KOMPAREDIFF2_DIFFERENCE_P_H

// lib
#include "differencestring.h"

namespace KompareDiff2
{

class DifferencePrivate
{
public:
    DifferencePrivate(int sourceLineNo, int destinationLineNo, int type);
    ~DifferencePrivate();

public:
    int type;

    int sourceLineNo;
    int destinationLineNo;
    int trackingDestinationLineNo;

    DifferenceStringList sourceLines;
    DifferenceStringList destinationLines;

    bool applied = false;
    bool conflicts = false;
    bool unsaved = false;
};

DifferencePrivate::DifferencePrivate(int sourceLineNo, int destinationLineNo, int type)
    : type(type)
    , sourceLineNo(sourceLineNo)
    , destinationLineNo(destinationLineNo)
    , trackingDestinationLineNo(sourceLineNo) // The whole patch starts as unapplied
{
}

DifferencePrivate::~DifferencePrivate()
{
    qDeleteAll(sourceLines);
    qDeleteAll(destinationLines);
}

}

#endif

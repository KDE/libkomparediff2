/*
SPDX-FileCopyrightText: 2001-2004,2009 Otto Bruggeman <bruggie@gmail.com>
SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_DIFFHUNK_H
#define KOMPAREDIFF2_DIFFHUNK_H

// lib
#include "komparediff2_export.h"
#include "difference.h"
// Std
#include <memory>

namespace KompareDiff2
{

class Difference;
class DiffHunkPrivate;

/**
 * DiffHunk
 */
class KOMPAREDIFF2_EXPORT DiffHunk
{
public:
    enum Type { Normal, AddedByBlend };

public:
    DiffHunk(int sourceLine, int destinationLine, const QString& function = QString(), Type type = Normal);
    ~DiffHunk();

    DifferenceList differences() const;
    QString function() const;

    int sourceLineNumber() const;
    int destinationLineNumber() const;

    int sourceLineCount() const;
    int destinationLineCount() const;

    Type type() const;
    void setType(Type type);

    void add(Difference* diff);

    QString recreateHunk() const;

private:
    Q_DECLARE_PRIVATE(DiffHunk)
    std::unique_ptr<DiffHunkPrivate> const d_ptr;
};

using DiffHunkList =              QList<DiffHunk*>;
using DiffHunkListIterator =      QList<DiffHunk*>::iterator;
using DiffHunkListConstIterator = QList<DiffHunk*>::const_iterator;

} // End of namespace KompareDiff2

#endif

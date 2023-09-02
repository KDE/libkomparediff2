/*
SPDX-FileCopyrightText: 2001-2004,2009 Otto Bruggeman <bruggie@gmail.com>
SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_DIFFHUNK_H
#define KOMPAREDIFF2_DIFFHUNK_H

#include "difference.h"


namespace KompareDiff2
{

class Difference;

/**
 * DiffHunk
 */
class DiffHunk
{
public:
    enum Type { Normal, AddedByBlend };

public:
    DiffHunk(int sourceLine, int destinationLine, const QString& function = QString(), Type type = Normal);
    ~DiffHunk();

    const DifferenceList& differences() const { return m_differences; };
    const QString& function() const           { return m_function; };

    int sourceLineNumber() const      { return m_sourceLine; };
    int destinationLineNumber() const { return m_destinationLine; };

    int sourceLineCount() const;
    int destinationLineCount() const;

    Type type() const       { return m_type; }
    void setType(Type type) { m_type = type; }

    void add(Difference* diff);

    QString recreateHunk() const;

private:
    int            m_sourceLine;
    int            m_destinationLine;
    DifferenceList m_differences;
    QString        m_function;
    Type           m_type;
};

using DiffHunkList =              QList<DiffHunk*>;
using DiffHunkListIterator =      QList<DiffHunk*>::iterator;
using DiffHunkListConstIterator = QList<DiffHunk*>::const_iterator;

} // End of namespace KompareDiff2

#endif

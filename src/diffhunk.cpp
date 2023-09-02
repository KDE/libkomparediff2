/*
SPDX-FileCopyrightText: 2001-2004,2009 Otto Bruggeman <bruggie@gmail.com>
SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "diffhunk.h"

#include <komparediffdebug.h>

using namespace KompareDiff2;

DiffHunk::DiffHunk(int sourceLine, int destinationLine, const QString& function, Type type) :
    m_sourceLine(sourceLine),
    m_destinationLine(destinationLine),
    m_function(function),
    m_type(type)
{
}

DiffHunk::~DiffHunk()
{
}

void DiffHunk::add(Difference* diff)
{
    m_differences.append(diff);
}

int DiffHunk::sourceLineCount() const
{
    DifferenceListConstIterator diffIt = m_differences.begin();
    DifferenceListConstIterator dEnd   = m_differences.end();

    int lineCount = 0;

    for (; diffIt != dEnd; ++diffIt)
        lineCount += (*diffIt)->sourceLineCount();

    return lineCount;
}

int DiffHunk::destinationLineCount() const
{
    DifferenceListConstIterator diffIt = m_differences.begin();
    DifferenceListConstIterator dEnd   = m_differences.end();

    int lineCount = 0;

    for (; diffIt != dEnd; ++diffIt)
        lineCount += (*diffIt)->destinationLineCount();

    return lineCount;
}

QString DiffHunk::recreateHunk() const
{
    QString hunk;
    QString differences;

    // recreate body
    DifferenceListConstIterator diffIt = m_differences.begin();
    DifferenceListConstIterator dEnd   = m_differences.end();

    int slc = 0; // source line count
    int dlc = 0; // destination line count
    for (; diffIt != dEnd; ++diffIt)
    {
        switch ((*diffIt)->type())
        {
        case Difference::Unchanged:
        case Difference::Change:
            slc += (*diffIt)->sourceLineCount();
            dlc += (*diffIt)->destinationLineCount();
            break;
        case Difference::Insert:
            dlc += (*diffIt)->destinationLineCount();
            break;
        case Difference::Delete:
            slc += (*diffIt)->sourceLineCount();
            break;
        }
        differences += (*diffIt)->recreateDifference();
    }

    // recreate header
    hunk += QStringLiteral("@@ -%1,%3 +%2,%4 @@")
            .arg(m_sourceLine)
            .arg(m_destinationLine)
            .arg(slc)
            .arg(dlc);

    if (!m_function.isEmpty())
        hunk += QLatin1Char(' ') + m_function;

    hunk += QLatin1Char('\n');

    hunk += differences;

    qCDebug(LIBKOMPAREDIFF2) << hunk;
    return hunk;
}

/*
SPDX-FileCopyrightText: 2001-2004,2009 Otto Bruggeman <bruggie@gmail.com>
SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "diffhunk.h"
#include "diffhunk_p.h"

// lib
#include <komparediff2_logging.h>

using namespace KompareDiff2;

DiffHunk::DiffHunk(int sourceLine, int destinationLine, const QString& function, Type type)
    : d_ptr(new DiffHunkPrivate(sourceLine, destinationLine, function, type))
{
}

DiffHunk::~DiffHunk() = default;

DifferenceList DiffHunk::differences() const
{
    Q_D(const DiffHunk);

    return d->differences;
};

QString DiffHunk::function() const
{
    Q_D(const DiffHunk);

    return d->function;
};

int DiffHunk::sourceLineNumber() const
{
    Q_D(const DiffHunk);

    return d->sourceLine;
};

int DiffHunk::destinationLineNumber() const
{
    Q_D(const DiffHunk);

    return d->destinationLine;
};

DiffHunk::Type DiffHunk::type() const
{
    Q_D(const DiffHunk);

    return d->type;
}

void DiffHunk::setType(Type type)
{
    Q_D(DiffHunk);

    d->type = type;
}

void DiffHunk::add(Difference* diff)
{
    Q_D(DiffHunk);

    d->differences.append(diff);
}

int DiffHunk::sourceLineCount() const
{
    Q_D(const DiffHunk);

    DifferenceListConstIterator diffIt = d->differences.begin();
    DifferenceListConstIterator dEnd   = d->differences.end();

    int lineCount = 0;

    for (; diffIt != dEnd; ++diffIt)
        lineCount += (*diffIt)->sourceLineCount();

    return lineCount;
}

int DiffHunk::destinationLineCount() const
{
    Q_D(const DiffHunk);

    DifferenceListConstIterator diffIt = d->differences.begin();
    DifferenceListConstIterator dEnd   = d->differences.end();

    int lineCount = 0;

    for (; diffIt != dEnd; ++diffIt)
        lineCount += (*diffIt)->destinationLineCount();

    return lineCount;
}

QString DiffHunk::recreateHunk() const
{
    Q_D(const DiffHunk);

    QString hunk;
    QString differences;

    // recreate body
    DifferenceListConstIterator diffIt = d->differences.begin();
    DifferenceListConstIterator dEnd   = d->differences.end();

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
            .arg(d->sourceLine)
            .arg(d->destinationLine)
            .arg(slc)
            .arg(dlc);

    if (!d->function.isEmpty())
        hunk += QLatin1Char(' ') + d->function;

    hunk += QLatin1Char('\n');

    hunk += differences;

    qCDebug(KOMPAREDIFF2_LOG) << hunk;
    return hunk;
}

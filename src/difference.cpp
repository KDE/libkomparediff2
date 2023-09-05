/*
SPDX-FileCopyrightText: 2001-2004,2009 Otto Bruggeman <bruggie@gmail.com>
SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "difference.h"
#include "difference_p.h"

// lib
#include "differencestringpair.h"
#include "levenshteintable.h"

using namespace KompareDiff2;

Difference::Difference(int sourceLineNo, int destinationLineNo, int type) :
    d_ptr(new DifferencePrivate(sourceLineNo, destinationLineNo, type))
{
}

Difference::~Difference() = default;

int Difference::type() const
{
    Q_D(const Difference);

    return d->type;
};

int Difference::sourceLineNumber() const
{
    Q_D(const Difference);

    return d->sourceLineNo;
}

int Difference::destinationLineNumber() const
{
    Q_D(const Difference);

    return d->destinationLineNo;
}

int Difference::trackingDestinationLineNumber() const
{
    Q_D(const Difference);

    return d->trackingDestinationLineNo;
}

void Difference::setTrackingDestinationLineNumber(int i)
{
    Q_D(Difference);

    d->trackingDestinationLineNo = i;
}

DifferenceString* Difference::sourceLineAt(int i) const
{
    Q_D(const Difference);

    return d->sourceLines[i];
}

DifferenceString* Difference::destinationLineAt(int i) const
{
    Q_D(const Difference);

    return d->destinationLines[i];
}

DifferenceStringList Difference::sourceLines() const
{
    Q_D(const Difference);

    return d->sourceLines;
}

DifferenceStringList Difference::destinationLines() const
{
    Q_D(const Difference);

    return d->destinationLines;
}

bool Difference::hasConflict() const
{
    Q_D(const Difference);

    return d->conflicts;
}

void Difference::setConflict(bool conflicts)
{
    Q_D(Difference);

    d->conflicts = conflicts;
}

bool Difference::isUnsaved() const
{
    Q_D(const Difference);

    return d->unsaved;
}

void Difference::setUnsaved(bool unsaved)
{
    Q_D(Difference);

    d->unsaved = unsaved;
}

bool Difference::applied() const
{
    Q_D(const Difference);

    return d->applied;
}

void Difference::setType(int type)
{
    Q_D(Difference);

    d->type = type;
}

void Difference::addSourceLine(const QString &line)
{
    Q_D(Difference);

    d->sourceLines.append(new DifferenceString(line));
}

void Difference::addDestinationLine(const QString &line)
{
    Q_D(Difference);

    d->destinationLines.append(new DifferenceString(line));
}

int Difference::sourceLineCount() const
{
    Q_D(const Difference);

    return d->sourceLines.count();
}

int Difference::destinationLineCount() const
{
    Q_D(const Difference);

    return d->destinationLines.count();
}

int Difference::sourceLineEnd() const
{
    Q_D(const Difference);

    return d->sourceLineNo + d->sourceLines.count();
}

int Difference::destinationLineEnd() const
{
    Q_D(const Difference);

    return d->destinationLineNo + d->destinationLines.count();
}

int Difference::trackingDestinationLineEnd() const
{
    Q_D(const Difference);

    return d->trackingDestinationLineNo + d->destinationLines.count();
}

void Difference::apply(bool apply)
{
    Q_D(Difference);

    if (apply == d->applied) {
        return;
    }

    d->applied = apply;
    d->unsaved = !d->unsaved;
    Q_EMIT differenceApplied(this);
}

void Difference::applyQuietly(bool apply)
{
    Q_D(Difference);

    if (d->applied == apply) {
        return;
    }

    d->unsaved = !d->unsaved;
    d->applied = apply;
}

void Difference::determineInlineDifferences()
{
    Q_D(Difference);

    if (d->type != Difference::Change)
        return;

    // Do nothing for now when the slc != dlc
    // One could try to find the closest matching destination string for any
    // of the source strings but this is compute intensive
    int slc = sourceLineCount();

    if (slc != destinationLineCount())
        return;

    LevenshteinTable<DifferenceStringPair> table;

    for (int i = 0; i < slc; ++i)
    {
        DifferenceString* sl = sourceLineAt(i);
        DifferenceString* dl = destinationLineAt(i);
        DifferenceStringPair* pair = new DifferenceStringPair(sl, dl);

        // return value 0 means something went wrong creating the table so don't bother finding markers
        if (table.createTable(pair) != 0)
            table.createListsOfMarkers();
    }
}

QString Difference::recreateDifference() const
{
    Q_D(const Difference);

    QString difference;

    // source
    DifferenceStringListConstIterator stringIt = d->sourceLines.begin();
    DifferenceStringListConstIterator sEnd     = d->sourceLines.end();

    for (; stringIt != sEnd; ++stringIt)
    {
        switch (d->type)
        {
        case Change:
        case Delete:
            difference += QLatin1Char('-');
            break;
        default:
            // Insert but this is not possible in source
            // Unchanged will be handled in destination
            // since they are the same
//             qCDebug(KOMPAREDIFF2_LOG) << "Go away, nothing to do for you in source...";
            continue;
        }
        difference += (*stringIt)->string();
    }

    //destination
    stringIt = d->destinationLines.begin();
    sEnd     = d->destinationLines.end();

    for (; stringIt != sEnd; ++stringIt)
    {
        switch (d->type)
        {
        case Change:
        case Insert:
            difference += QLatin1Char('+');
            break;
        case Unchanged:
            difference += QLatin1Char(' ');
            break;
        default: // Delete but this is not possible in destination
//             qCDebug(KOMPAREDIFF2_LOG) << "Go away, nothing to do for you in destination...";
            continue;
        }
        difference += (*stringIt)->string();
    }

    return difference;
}

#include "moc_difference.cpp"

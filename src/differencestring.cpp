/*
SPDX-FileCopyrightText: 2001-2004,2009 Otto Bruggeman <bruggie@gmail.com>
SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "differencestring.h"
#include "differencestring_p.h"

namespace KompareDiff2
{

DifferenceString::DifferenceString()
    : d_ptr(new DifferenceStringPrivate())
{
//         qCDebug(LIBKOMPAREDIFF2) << "DifferenceString::DifferenceString()";
}

DifferenceString::DifferenceString(const QString& string, const MarkerList& markerList)
    : d_ptr(new DifferenceStringPrivate(string, markerList))
{
//         qCDebug(LIBKOMPAREDIFF2) << "DifferenceString::DifferenceString( " << string << ", " << markerList << " )";
}

DifferenceString::DifferenceString(const DifferenceString& ds)
    :d_ptr (new DifferenceStringPrivate(*ds.d_ptr))
{
//         qCDebug(LIBKOMPAREDIFF2) << "DifferenceString::DifferenceString( const DifferenceString& " << ds << " )";
}

DifferenceString::~DifferenceString() = default;

bool DifferenceString::operator==(const DifferenceString& ks) const
{
    Q_D(const DifferenceString);

    return (*d == *ks.d_ptr);
}

const QString& DifferenceString::string() const
{
    Q_D(const DifferenceString);

    return d->string;
}

const QString& DifferenceString::conflictString() const
{
    Q_D(const DifferenceString);

    return d->conflict;
}

const MarkerList& DifferenceString::markerList() const
{
    Q_D(const DifferenceString);

    return d->markerList;
}

void DifferenceString::setString(const QString& string)
{
    Q_D(DifferenceString);

    d->string = string;
    d->calculateHash();
}

void DifferenceString::setConflictString(const QString& conflict)
{
    Q_D(DifferenceString);

    d->conflict = conflict;
}

void DifferenceString::setMarkerList(const MarkerList& markerList)
{
    Q_D(DifferenceString);

    d->markerList = markerList;
}

void DifferenceString::prepend(Marker* marker)
{
    Q_D(DifferenceString);

    d->markerList.prepend(marker);
}

}

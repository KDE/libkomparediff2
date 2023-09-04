/*
SPDX-FileCopyrightText: 2001-2004,2009 Otto Bruggeman <bruggie@gmail.com>
SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_DIFFERENCESTRING_H
#define KOMPAREDIFF2_DIFFERENCESTRING_H

// lib
#include "komparediff2_export.h"
#include "marker.h"
// Qt
#include <QList>
#include <QString>
// Std
#include <memory>

namespace KompareDiff2
{
class DifferenceStringPrivate;

/**
 * @class DifferenceString differencestring.h <KompareDiff2/DifferenceString>
 *
 * A difference string.
 */
class KOMPAREDIFF2_EXPORT DifferenceString
{
public:
    DifferenceString();
    explicit DifferenceString(const QString& string, const MarkerList& markerList = MarkerList());
    DifferenceString(const DifferenceString& ds);
    ~DifferenceString();

public:
    bool operator==(const DifferenceString& ks) const;

    QString string() const;
    QString conflictString() const;
    MarkerList markerList() const;
    void setString(const QString& string);
    void setConflictString(const QString& conflict);
    void setMarkerList(const MarkerList& markerList);
    void prepend(Marker* marker);

private:
    Q_DECLARE_PRIVATE(DifferenceString)
    std::unique_ptr<DifferenceStringPrivate> const d_ptr;
};

using DifferenceStringList =              QList<DifferenceString*>;
using DifferenceStringListIterator =      QList<DifferenceString*>::iterator;
using DifferenceStringListConstIterator = QList<DifferenceString*>::const_iterator;

}

#endif

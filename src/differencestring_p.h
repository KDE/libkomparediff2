/*
SPDX-FileCopyrightText: 2001-2004,2009 Otto Bruggeman <bruggie@gmail.com>
SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_DIFFERENCESTRING_P_H
#define KOMPAREDIFF2_DIFFERENCESTRING_P_H

// lib
#include "marker.h"
// Qt
#include <QString>

namespace KompareDiff2
{

class DifferenceStringPrivate
{
public:
    DifferenceStringPrivate() = default;
    DifferenceStringPrivate(const QString& string, const MarkerList& markerList);
    DifferenceStringPrivate(const DifferenceStringPrivate& ds) = default;

    ~DifferenceStringPrivate();

    bool operator==(const DifferenceStringPrivate& ks) const;

public:
    void calculateHash();

public:
    QString      string;
    QString      conflict;
    unsigned int hash;
    MarkerList   markerList;
};

inline
DifferenceStringPrivate::DifferenceStringPrivate(const QString& string, const MarkerList& markerList)
    : string(string)
    , markerList(markerList)
{
    calculateHash();
}

inline
DifferenceStringPrivate::~DifferenceStringPrivate()
{
    qDeleteAll(markerList);
}

inline
bool DifferenceStringPrivate::operator==(const DifferenceStringPrivate& ks) const
{
    if (hash != ks.hash) {
        return false;
    }
    return (string == ks.string);
}

inline
void DifferenceStringPrivate::calculateHash()
{
    unsigned short const* str = reinterpret_cast<unsigned short const*>(string.unicode());
    const unsigned int len = string.length();

    hash = 1315423911;

    for (unsigned int i = 0; i < len; ++i) {
        hash ^= (hash << 5) + str[i] + (hash >> 2);
    }
}

}

#endif

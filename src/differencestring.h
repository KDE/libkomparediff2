/*
SPDX-FileCopyrightText: 2001-2004,2009 Otto Bruggeman <bruggie@gmail.com>
SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_DIFFERENCESTRING_H
#define KOMPAREDIFF2_DIFFERENCESTRING_H

#include <QVector>
#include <QString>

#include "komparediff2_export.h"
#include "marker.h"

namespace KompareDiff2
{

/**
 * @class DifferenceString differencestring.h <KompareDiff2/DifferenceString>
 *
 * A difference string.
 */
class KOMPAREDIFF2_EXPORT DifferenceString
{
public:
    DifferenceString()
    {
//         qCDebug(LIBKOMPAREDIFF2) << "DifferenceString::DifferenceString()";
    }
    explicit DifferenceString(const QString& string, const MarkerList& markerList = MarkerList()) :
        m_string(string),
        m_markerList(markerList)
    {
//         qCDebug(LIBKOMPAREDIFF2) << "DifferenceString::DifferenceString( " << string << ", " << markerList << " )";
        calculateHash();
    }
    DifferenceString(const DifferenceString& ds) :
        m_string(ds.m_string),
        m_conflict(ds.m_conflict),
        m_hash(ds.m_hash),
        m_markerList(ds.m_markerList)
    {
//         qCDebug(LIBKOMPAREDIFF2) << "DifferenceString::DifferenceString( const DifferenceString& " << ds << " )";
    }
    ~DifferenceString()
    {
        qDeleteAll(m_markerList);
    }

public:
    const QString& string() const
    {
        return m_string;
    }
    const QString& conflictString() const
    {
        return m_conflict;
    }
    const MarkerList& markerList()
    {
        return m_markerList;
    }
    void setString(const QString& string)
    {
        m_string = string;
        calculateHash();
    }
    void setConflictString(const QString& conflict)
    {
        m_conflict = conflict;
    }
    void setMarkerList(const MarkerList& markerList)
    {
        m_markerList = markerList;
    }
    void prepend(Marker* marker)
    {
        m_markerList.prepend(marker);
    }
    bool operator==(const DifferenceString& ks)
    {
        if (m_hash != ks.m_hash)
            return false;
        return m_string == ks.m_string;
    }

protected:
    void calculateHash()
    {
        unsigned short const* str = reinterpret_cast<unsigned short const*>(m_string.unicode());
        const unsigned int len = m_string.length();

        m_hash = 1315423911;

        for (unsigned int i = 0; i < len; ++i)
        {
            m_hash ^= (m_hash << 5) + str[i] + (m_hash >> 2);
        }
    }

private:
    QString      m_string;
    QString      m_conflict;
    unsigned int m_hash;
    MarkerList   m_markerList;
};

using DifferenceStringList =              QVector<DifferenceString*>;
using DifferenceStringListIterator =      QVector<DifferenceString*>::iterator;
using DifferenceStringListConstIterator = QVector<DifferenceString*>::const_iterator;

}

#endif

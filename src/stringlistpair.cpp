/*
SPDX-FileCopyrightText: 2011 Dmitry Risenberg <dmitry.risenberg@gmail.com>

SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "stringlistpair.h"

// Qt
#include <QHash>

using namespace KompareDiff2;

unsigned int StringListPair::lengthFirst() const
{
    return m_lengthFirst;
}

unsigned int StringListPair::lengthSecond() const
{
    return m_lengthSecond;
}

MarkerList StringListPair::markerListFirst() const
{
    return m_markersFirst;
}

MarkerList StringListPair::markerListSecond() const
{
    return m_markersSecond;
}

void StringListPair::prependFirst(Marker* marker)
{
    m_markersFirst.prepend(marker);
}

void StringListPair::prependSecond(Marker* marker)
{
    m_markersSecond.prepend(marker);
}

StringListPair::StringListPair(const QStringList& first, const QStringList& second)
    : m_first(first)
    , m_second(second)
    // Do not forget about 1 virtual element - see LevenshteinTable
    , m_lengthFirst(first.length() + 1)
    , m_lengthSecond(second.length() + 1)
    , m_hashesFirst(m_lengthFirst)
    , m_hashesSecond(m_lengthSecond)
{

    m_hashesFirst[0] = qHash(QString());
    for (unsigned int i = 1; i < m_lengthFirst; ++i) {
        m_hashesFirst[i] = qHash(first[i - 1]);
    }
    m_hashesSecond[0] = qHash(QString());
    for (unsigned int i = 1; i < m_lengthSecond; ++i) {
        m_hashesSecond[i] = qHash(second[i - 1]);
    }
}

StringListPair::~StringListPair() = default;

bool StringListPair::equal(unsigned int firstIndex, unsigned int secondIndex) const
{
    if (m_hashesFirst[firstIndex] != m_hashesSecond[secondIndex]) {
        return false;
    }
    if (firstIndex == 0 || secondIndex == 0) {
        return firstIndex == 0 && secondIndex == 0;
    }
    return m_first[firstIndex - 1] == m_second[secondIndex - 1];
}

bool StringListPair::needFineGrainedOutput(unsigned int) const
{
    return true;
}

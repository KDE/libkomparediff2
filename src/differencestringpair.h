/*
SPDX-FileCopyrightText: 2011 Dmitry Risenberg <dmitry.risenberg@gmail.com>

SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOMPAREDIFF2_DIFFERENCESTRINGPAIR_H
#define KOMPAREDIFF2_DIFFERENCESTRINGPAIR_H

#include <QString>

#include "difference.h"

namespace KompareDiff2 {

class Marker;
class DifferenceString;

class DifferenceStringPair {
public:
    DifferenceStringPair(DifferenceString* first, DifferenceString* second)
        : m_first(first), m_second(second),
          m_strFirst(QLatin1Char(' ') + first->string()), m_strSecond(QLatin1Char(' ') + second->string()),
          m_lengthFirst(m_strFirst.length()), m_lengthSecond(m_strSecond.length()),
          m_arrayFirst(m_strFirst.unicode()), m_arraySecond(m_strSecond.unicode())
    {
        // Actual contents must be indented by 1
    }
    bool equal(unsigned int firstIndex, unsigned int secondIndex) const
    {
        return m_arrayFirst[firstIndex] == m_arraySecond[secondIndex];
    }
    unsigned int lengthFirst() const
    {
        return m_lengthFirst;
    }
    unsigned int lengthSecond() const
    {
        return m_lengthSecond;
    }
    MarkerList markerListFirst() const
    {
        return m_first->markerList();
    }
    MarkerList markerListSecond() const
    {
        return m_second->markerList();
    }
    void prependFirst(Marker* marker)
    {
        m_first->prepend(marker);
    }
    void prependSecond(Marker* marker)
    {
        m_second->prepend(marker);
    }
    bool needFineGrainedOutput(unsigned int difference) const
    {
        return difference <= qMax(m_lengthFirst, m_lengthSecond) / 2;
    }
    const static bool allowReplace = true;
private:
    DifferenceString* m_first;
    DifferenceString* m_second;
    QString m_strFirst;
    QString m_strSecond;
    unsigned int m_lengthFirst;
    unsigned int m_lengthSecond;
    const QChar* m_arrayFirst;
    const QChar* m_arraySecond;
};

}

#endif // KOMPAREDIFF2_DIFFERENCESTRINGPAIR_H

/*
SPDX-FileCopyrightText: 2011 Dmitry Risenberg <dmitry.risenberg@gmail.com>

SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_STRINGLISTPAIR_H
#define KOMPAREDIFF2_STRINGLISTPAIR_H

#include <QStringList>
#include "marker.h"

namespace Diff2 {

class StringListPair {
public:
    StringListPair(const QStringList& first, const QStringList& second);
    ~StringListPair();
    bool equal(unsigned int firstIndex, unsigned int secondIndex) const;
    unsigned int lengthFirst() const;
    unsigned int lengthSecond() const;
    MarkerList markerListFirst() const;
    MarkerList markerListSecond() const;
    void prependFirst(Marker* marker);
    void prependSecond(Marker* marker);
    bool needFineGrainedOutput(unsigned int difference) const;

    const static bool allowReplace = false;
private:
    const QStringList m_first;
    const QStringList m_second;
    unsigned int m_lengthFirst;
    unsigned int m_lengthSecond;
    unsigned int* m_hashesFirst;
    unsigned int* m_hashesSecond;
    MarkerList m_markersFirst;
    MarkerList m_markersSecond;
};

}

#endif // KOMPAREDIFF2_STRINGLISTPAIR_H

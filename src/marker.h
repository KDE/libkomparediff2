/*
SPDX-FileCopyrightText: 2011 Dmitry Risenberg <dmitry.risenberg@gmail.com>

SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MARKER_H
#define MARKER_H

#include <QList>
#include "diff2_export.h"

namespace Diff2 {

/**
 * A Marker.
 */
class DIFF2_EXPORT Marker
{
public:
    enum Type { Start = 0, End = 1 };

public:
    Marker()
    {
        m_type = Marker::Start;
        m_offset = 0;
    }
    Marker(enum Marker::Type type, unsigned int offset)
    {
        m_type = type;
        m_offset = offset;
    }
    ~Marker() {}

public:
    enum Marker::Type type()   const { return m_type; }
    unsigned int      offset() const { return m_offset; }

    void setType(enum Marker::Type type) { m_type   = type; }
    void setOffset(unsigned int offset)  { m_offset = offset; }

    bool operator == (const Marker& rhs) const {
        return this->type() == rhs.type() && this->offset() == rhs.offset();
    }

private:
    enum Marker::Type m_type;
    unsigned int      m_offset;
};

using MarkerList =              QList<Marker*>;
using MarkerListIterator =      QList<Marker*>::iterator;
using MarkerListConstIterator = QList<Marker*>::const_iterator;

}

#endif // MARKER_H

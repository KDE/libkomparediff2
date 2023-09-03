/*
SPDX-FileCopyrightText: 2001-2004,2009 Otto Bruggeman <bruggie@gmail.com>
SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_DIFFERENCE_H
#define KOMPAREDIFF2_DIFFERENCE_H

#include <QObject>

#include "komparediff2_export.h"
#include "differencestring.h"

// #include <komparediffdebug.h>

namespace KompareDiff2
{

/**
 * @class Difference difference.h <KompareDiff2/Difference>
 *
 * A difference.
 */
class KOMPAREDIFF2_EXPORT Difference : public QObject
{
    Q_OBJECT
public:
    enum Type { Change, Insert, Delete, Unchanged };

public:
    Difference(int sourceLineNo, int destinationLineNo, int type = Difference::Unchanged);
    ~Difference() override;

public:
    int type() const { return m_type; };

    int sourceLineNumber() const { return m_sourceLineNo; }
    int destinationLineNumber() const { return m_destinationLineNo; }

    int sourceLineCount() const;
    int destinationLineCount() const;

    int sourceLineEnd() const;
    int destinationLineEnd() const;

    /// Destination line number that tracks applying/unapplying of other differences
    /// Essentially a line number in a patch consisting of applied diffs only
    int trackingDestinationLineNumber() const { return m_trackingDestinationLineNo; }
    int trackingDestinationLineEnd() const;
    void setTrackingDestinationLineNumber(int i) { m_trackingDestinationLineNo = i; }

    DifferenceString* sourceLineAt(int i) const { return m_sourceLines[i]; }
    DifferenceString* destinationLineAt(int i) const { return m_destinationLines[i]; }

    const DifferenceStringList sourceLines() const { return m_sourceLines; }
    const DifferenceStringList destinationLines() const { return m_destinationLines; }

    bool hasConflict() const
    {
        return m_conflicts;
    }
    void setConflict(bool conflicts)
    {
        m_conflicts = conflicts;
    }

    bool isUnsaved() const
    {
        return m_unsaved;
    }
    void setUnsaved(bool unsaved)
    {
        m_unsaved = unsaved;
    }

    void apply(bool apply);
    /// Apply without emitting any signals
    void applyQuietly(bool apply);
    bool applied() const { return m_applied; }

    void setType(int type) { m_type = type; }

    void addSourceLine(QString line);
    void addDestinationLine(QString line);

    /** This method will calculate the differences between the individual strings and store them as Markers */
    void determineInlineDifferences();

    QString recreateDifference() const;

Q_SIGNALS:
    void differenceApplied(Difference*);

private:
    int                   m_type;

    int                   m_sourceLineNo;
    int                   m_destinationLineNo;
    int                   m_trackingDestinationLineNo;

    DifferenceStringList  m_sourceLines;
    DifferenceStringList  m_destinationLines;

    bool                  m_applied;
    bool                  m_conflicts;
    bool                  m_unsaved;
};

using DifferenceList =              QList<Difference*>;
using DifferenceListIterator =      QList<Difference*>::iterator;
using DifferenceListConstIterator = QList<Difference*>::const_iterator;

} // End of namespace KompareDiff2

#endif


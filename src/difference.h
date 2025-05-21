/*
    SPDX-FileCopyrightText: 2001-2004,2009 Otto Bruggeman <bruggie@gmail.com>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_DIFFERENCE_H
#define KOMPAREDIFF2_DIFFERENCE_H

// lib
#include "differencestring.h"
#include "komparediff2_export.h"
// Qt
#include <QObject>
// Std
#include <memory>

namespace KompareDiff2
{
class DifferencePrivate;

/*!
 * \inmodule KompareDiff2
 * \class KompareDiff2::Difference
 * \inheaderfile KompareDiff2/Difference
 * \brief A difference.
 */
class KOMPAREDIFF2_EXPORT Difference : public QObject
{
    Q_OBJECT
public:
    /*!
     * \enum KompareDiff2::Difference::Type
     * \value Change
     * \value Insert
     * \value Delete
     * \value Unchanged
     */
    enum Type {
        Change,
        Insert,
        Delete,
        Unchanged,
    };

public:
    /*!
     */
    Difference(int sourceLineNo, int destinationLineNo, int type = Difference::Unchanged);
    /*!
     */
    ~Difference() override;

public:
    /*!
     */
    int type() const;

    /*!
     */
    int sourceLineNumber() const;
    /*!
     */
    int destinationLineNumber() const;

    /*!
     */
    int sourceLineCount() const;
    /*!
     */
    int destinationLineCount() const;

    /*!
     */
    int sourceLineEnd() const;
    /*!
     */
    int destinationLineEnd() const;

    /// Destination line number that tracks applying/unapplying of other differences
    /// Essentially a line number in a patch consisting of applied diffs only
    /*!
     */
    int trackingDestinationLineNumber() const;
    /*!
     */
    int trackingDestinationLineEnd() const;
    /*!
     */
    void setTrackingDestinationLineNumber(int i);

    /*!
     */
    DifferenceString *sourceLineAt(int i) const;
    /*!
     */
    DifferenceString *destinationLineAt(int i) const;

    /*!
     */
    DifferenceStringList sourceLines() const;
    /*!
     */
    DifferenceStringList destinationLines() const;

    /*!
     */
    bool hasConflict() const;
    /*!
     */
    void setConflict(bool conflicts);

    /*!
     */
    bool isUnsaved() const;
    /*!
     */
    void setUnsaved(bool unsaved);

    /*!
     */
    void apply(bool apply);
    /*! Apply without emitting any signals. */
    void applyQuietly(bool apply);
    /*!
     */
    bool applied() const;

    /*!
     */
    void setType(int type);

    /*!
     */
    void addSourceLine(const QString &line);
    /*!
     */
    void addDestinationLine(const QString &line);

    /*! Calculates the differences between the individual strings and store them as Markers. */
    void determineInlineDifferences();

    /*!
     */
    QString recreateDifference() const;

Q_SIGNALS:

    /*!
     */
    void differenceApplied(KompareDiff2::Difference *);

private:
    Q_DECLARE_PRIVATE(Difference)
    std::unique_ptr<DifferencePrivate> const d_ptr;
};

using DifferenceList =              QList<Difference *>;
using DifferenceListIterator =      QList<Difference *>::iterator;
using DifferenceListConstIterator = QList<Difference *>::const_iterator;

} // End of namespace KompareDiff2

#endif

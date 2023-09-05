/*
    SPDX-FileCopyrightText: 2001-2009 Otto Bruggeman <bruggie@gmail.com>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "diffmodel.h"
#include "diffmodel_p.h"

// lib
#include "difference.h"
#include "levenshteintable.h"
#include "parserbase.h"
#include "stringlistpair.h"
#include <komparediff2_logging.h>
// Std
#include <algorithm>

using namespace KompareDiff2;

/**  */
DiffModel::DiffModel(const QString &source, const QString &destination)
    : d_ptr(new DiffModelPrivate(source, destination))
{
    Q_D(DiffModel);

    d->splitSourceInPathAndFileName();
    d->splitDestinationInPathAndFileName();
}

DiffModel::DiffModel()
    : d_ptr(new DiffModelPrivate())
{
}

DiffModel::~DiffModel() = default;

DiffModel &DiffModel::operator=(const DiffModel &model)
{
    Q_D(DiffModel);

    if (&model != this) // Guard from self-assignment
    {
        *d = *model.d_ptr;
    }

    return *this;
}

bool DiffModel::operator<(const DiffModel &model) const
{
    if (localeAwareCompareSource(model) < 0)
        return true;
    return false;
}

int DiffModel::hunkCount() const
{
    Q_D(const DiffModel);

    return d->hunks.count();
}

int DiffModel::differenceCount() const
{
    Q_D(const DiffModel);

    return d->differences.count();
}

int DiffModel::appliedCount() const
{
    Q_D(const DiffModel);

    return d->appliedCount;
}

DiffHunk *DiffModel::hunkAt(int i)
{
    Q_D(DiffModel);

    return (d->hunks.at(i));
}

const Difference *DiffModel::differenceAt(int i) const
{
    Q_D(const DiffModel);

    return (d->differences.at(i));
}

Difference *DiffModel::differenceAt(int i)
{
    Q_D(DiffModel);

    return (d->differences.at(i));
}

DiffHunkList *DiffModel::hunks()
{
    Q_D(DiffModel);

    return &d->hunks;
}

const DiffHunkList *DiffModel::hunks() const
{
    Q_D(const DiffModel);

    return &d->hunks;
}

DifferenceList *DiffModel::differences()
{
    Q_D(DiffModel);

    return &d->differences;
}

const DifferenceList *DiffModel::differences() const
{
    Q_D(const DiffModel);

    return &d->differences;
}

int DiffModel::findDifference(Difference *diff) const
{
    Q_D(const DiffModel);

    return d->differences.indexOf(diff);
}

int DiffModel::localeAwareCompareSource(const DiffModel &model) const
{
    Q_D(const DiffModel);

    qCDebug(KOMPAREDIFF2_LOG) << "Path: " << model.d_ptr->sourcePath;
    qCDebug(KOMPAREDIFF2_LOG) << "File: " << model.d_ptr->sourceFile;

    int result = d->sourcePath.localeAwareCompare(model.d_ptr->sourcePath);

    if (result == 0)
        return d->sourceFile.localeAwareCompare(model.d_ptr->sourceFile);

    return result;
}

QString DiffModel::recreateDiff() const
{
    Q_D(const DiffModel);

    // For now we'll always return a diff in the diff format
    QString diff;

    // recreate header
    const QChar tab = QLatin1Char('\t');
    const QChar nl = QLatin1Char('\n');
    diff += QStringLiteral("--- %1\t%2").arg(ParserBase::escapePath(d->source), d->sourceTimestamp);
    if (!d->sourceRevision.isEmpty())
        diff += tab + d->sourceRevision;
    diff += nl;
    diff += QStringLiteral("+++ %1\t%2").arg(ParserBase::escapePath(d->destination), d->destinationTimestamp);
    if (!d->destinationRevision.isEmpty())
        diff += tab + d->destinationRevision;
    diff += nl;

    // recreate body by iterating over the hunks
    DiffHunkListConstIterator hunkIt = d->hunks.begin();
    DiffHunkListConstIterator hEnd = d->hunks.end();

    for (; hunkIt != hEnd; ++hunkIt) {
        if ((*hunkIt)->type() != DiffHunk::AddedByBlend)
            diff += (*hunkIt)->recreateHunk();
    }

    return diff;
}

Difference *DiffModel::firstDifference()
{
    Q_D(DiffModel);

    qCDebug(KOMPAREDIFF2_LOG) << "DiffModel::firstDifference()";
    d->diffIndex = 0;
    qCDebug(KOMPAREDIFF2_LOG) << "d->diffIndex = " << d->diffIndex;

    d->selectedDifference = d->differences[d->diffIndex];

    return d->selectedDifference;
}

Difference *DiffModel::lastDifference()
{
    Q_D(DiffModel);

    qCDebug(KOMPAREDIFF2_LOG) << "DiffModel::lastDifference()";
    d->diffIndex = d->differences.count() - 1;
    qCDebug(KOMPAREDIFF2_LOG) << "d->diffIndex = " << d->diffIndex;

    d->selectedDifference = d->differences[d->diffIndex];

    return d->selectedDifference;
}

Difference *DiffModel::prevDifference()
{
    Q_D(DiffModel);

    qCDebug(KOMPAREDIFF2_LOG) << "DiffModel::prevDifference()";
    if (d->diffIndex > 0 && --d->diffIndex < d->differences.count()) {
        qCDebug(KOMPAREDIFF2_LOG) << "d->diffIndex = " << d->diffIndex;
        d->selectedDifference = d->differences[d->diffIndex];
    } else {
        d->selectedDifference = nullptr;
        d->diffIndex = 0;
        qCDebug(KOMPAREDIFF2_LOG) << "d->diffIndex = " << d->diffIndex;
    }

    return d->selectedDifference;
}

Difference *DiffModel::nextDifference()
{
    Q_D(DiffModel);

    qCDebug(KOMPAREDIFF2_LOG) << "DiffModel::nextDifference()";
    if (++d->diffIndex < d->differences.count()) {
        qCDebug(KOMPAREDIFF2_LOG) << "d->diffIndex = " << d->diffIndex;
        d->selectedDifference = d->differences[d->diffIndex];
    } else {
        d->selectedDifference = nullptr;
        d->diffIndex = 0; // just for safety...
        qCDebug(KOMPAREDIFF2_LOG) << "d->diffIndex = " << d->diffIndex;
    }

    return d->selectedDifference;
}

QString DiffModel::source() const
{
    Q_D(const DiffModel);

    return d->source;
}

QString DiffModel::destination() const
{
    Q_D(const DiffModel);

    return d->destination;
}

QString DiffModel::sourceFile() const
{
    Q_D(const DiffModel);

    return d->sourceFile;
}

QString DiffModel::destinationFile() const
{
    Q_D(const DiffModel);

    return d->destinationFile;
}

QString DiffModel::sourcePath() const
{
    Q_D(const DiffModel);

    return d->sourcePath;
}

QString DiffModel::destinationPath() const
{
    Q_D(const DiffModel);

    return d->destinationPath;
}

QString DiffModel::sourceTimestamp() const
{
    Q_D(const DiffModel);

    return d->sourceTimestamp;
}

QString DiffModel::destinationTimestamp() const
{
    Q_D(const DiffModel);

    return d->destinationTimestamp;
}

QString DiffModel::sourceRevision() const
{
    Q_D(const DiffModel);

    return d->sourceRevision;
}

QString DiffModel::destinationRevision() const
{
    Q_D(const DiffModel);

    return d->destinationRevision;
}

void DiffModel::setSourceFile(const QString &path)
{
    Q_D(DiffModel);

    d->source = path;
    d->splitSourceInPathAndFileName();
}

void DiffModel::setDestinationFile(const QString &path)
{
    Q_D(DiffModel);

    d->destination = path;
    d->splitDestinationInPathAndFileName();
}

void DiffModel::setSourceTimestamp(const QString &timestamp)
{
    Q_D(DiffModel);

    d->sourceTimestamp = timestamp;
}

void DiffModel::setDestinationTimestamp(const QString &timestamp)
{
    Q_D(DiffModel);

    d->destinationTimestamp = timestamp;
}

void DiffModel::setSourceRevision(const QString &revision)
{
    Q_D(DiffModel);

    d->sourceRevision = revision;
}

void DiffModel::setDestinationRevision(const QString &revision)
{
    Q_D(DiffModel);

    d->destinationRevision = revision;
}

bool DiffModel::isBlended() const
{
    Q_D(const DiffModel);

    return d->blended;
}

void DiffModel::setBlended(bool blended)
{
    Q_D(DiffModel);

    d->blended = blended;
}

void DiffModel::addHunk(DiffHunk *hunk)
{
    Q_D(DiffModel);

    d->hunks.append(hunk);
}

void DiffModel::addDiff(Difference *diff)
{
    Q_D(DiffModel);

    d->differences.append(diff);
    connect(diff, &Difference::differenceApplied, this, &DiffModel::slotDifferenceApplied);
}

int DiffModel::diffIndex() const
{
    Q_D(const DiffModel);

    return d->diffIndex;
}

void DiffModel::setDiffIndex(int diffIndex)
{
    Q_D(DiffModel);

    d->diffIndex = diffIndex;
}

bool DiffModel::hasUnsavedChanges() const
{
    Q_D(const DiffModel);

    return std::any_of(d->differences.constBegin(), d->differences.constEnd(), [] (const Difference* diff) {
        return diff->isUnsaved();
    });
}

void DiffModel::applyDifference(bool apply)
{
    Q_D(DiffModel);

    bool appliedState = d->selectedDifference->applied();
    if (appliedState == apply) {
        return;
    }
    if (apply && !d->selectedDifference->applied())
        ++d->appliedCount;
    else if (!apply && d->selectedDifference->applied())
        --d->appliedCount;

    d->selectedDifference->apply(apply);
}

static int GetDifferenceDelta(Difference *diff)
{
    int delta = diff->destinationLineCount() - diff->sourceLineCount();
    if (!diff->applied()) {
        delta = -delta;
    }
    return delta;
}

void DiffModel::slotDifferenceApplied(Difference *diff)
{
    Q_D(DiffModel);

    int delta = GetDifferenceDelta(diff);
    for (Difference *current : std::as_const(d->differences)) {
        if (current->destinationLineNumber() > diff->destinationLineNumber()) {
            current->setTrackingDestinationLineNumber(current->trackingDestinationLineNumber() + delta);
        }
    }
}

void DiffModel::applyAllDifferences(bool apply)
{
    Q_D(DiffModel);

    if (apply) {
        d->appliedCount = d->differences.count();
    } else {
        d->appliedCount = 0;
    }

    DifferenceListIterator diffIt = d->differences.begin();
    DifferenceListIterator dEnd = d->differences.end();

    int totalDelta = 0;
    for (; diffIt != dEnd; ++diffIt) {
        (*diffIt)->setTrackingDestinationLineNumber((*diffIt)->trackingDestinationLineNumber() + totalDelta);
        bool appliedState = (*diffIt)->applied();
        if (appliedState == apply) {
            continue;
        }
        (*diffIt)->applyQuietly(apply);
        int currentDelta = GetDifferenceDelta(*diffIt);
        totalDelta += currentDelta;
    }
}

bool DiffModel::setSelectedDifference(Difference *diff)
{
    Q_D(DiffModel);

    qCDebug(KOMPAREDIFF2_LOG) << "diff = " << diff;
    qCDebug(KOMPAREDIFF2_LOG) << "d->selectedDifference = " << d->selectedDifference;

    if (diff != d->selectedDifference) {
        if ((d->differences.indexOf(diff)) == -1)
            return false;
        // Do not set d->diffIndex if it cant be found
        d->diffIndex = d->differences.indexOf(diff);
        qCDebug(KOMPAREDIFF2_LOG) << "d->diffIndex = " << d->diffIndex;
        d->selectedDifference = diff;
    }

    return true;
}

QPair<QList<Difference *>, QList<Difference *>> DiffModel::linesChanged(const QStringList &oldLines, const QStringList &newLines, int editLineNumber)
{
    Q_D(DiffModel);

    // These two will be returned as the function result
    QList<Difference *> inserted;
    QList<Difference *> removed;
    if (oldLines.size() == 0 && newLines.size() == 0) {
        return qMakePair(QList<Difference *>(), QList<Difference *>());
    }
    int editLineEnd = editLineNumber + oldLines.size();
    // Find the range of differences [iterBegin, iterEnd) that should be updated
    // TODO: assume that differences are ordered by starting line. Check that this is always the case
    DifferenceList applied;
    DifferenceListIterator iterBegin; // first diff ending a line before editLineNo or later
    for (iterBegin = d->differences.begin(); iterBegin != d->differences.end(); ++iterBegin) {
        // If the difference ends a line before the edit starts, they should be merged if this difference is applied.
        // Also it should be merged if it starts on editLineNumber, otherwise there will be two markers for the same line
        int lineAfterLast = (*iterBegin)->trackingDestinationLineEnd();
        if (lineAfterLast > editLineNumber
            || (lineAfterLast == editLineNumber && ((*iterBegin)->applied() || (*iterBegin)->trackingDestinationLineNumber() == editLineNumber))) {
            break;
        }
    }
    DifferenceListIterator iterEnd;
    for (iterEnd = iterBegin; iterEnd != d->differences.end(); ++iterEnd) {
        // If the difference starts a line after the edit ends, it should still be merged if it is applied
        int firstLine = (*iterEnd)->trackingDestinationLineNumber();
        if (firstLine > editLineEnd || (!(*iterEnd)->applied() && firstLine == editLineEnd)) {
            break;
        }
        if ((*iterEnd)->applied()) {
            applied.append(*iterEnd);
        }
    }

    // Compute line numbers in source and destination to which the for diff line sequences (will be created later)
    int sourceLineNumber;
    int destinationLineNumber;
    if (iterBegin == d->differences.end()) { // All existing diffs are after the change
        destinationLineNumber = editLineNumber;
        if (!d->differences.isEmpty()) {
            sourceLineNumber = d->differences.last()->sourceLineEnd() - (d->differences.last()->trackingDestinationLineEnd() - editLineNumber);
        } else {
            sourceLineNumber = destinationLineNumber;
        }
    } else if (!(*iterBegin)->applied() || (*iterBegin)->trackingDestinationLineNumber() >= editLineNumber) {
        destinationLineNumber = editLineNumber;
        sourceLineNumber = (*iterBegin)->sourceLineNumber() - ((*iterBegin)->trackingDestinationLineNumber() - editLineNumber);
    } else {
        sourceLineNumber = (*iterBegin)->sourceLineNumber();
        destinationLineNumber = (*iterBegin)->trackingDestinationLineNumber();
    }

    // Only the applied differences are of interest, unapplied can be safely removed
    DifferenceListConstIterator appliedBegin = applied.constBegin();
    DifferenceListConstIterator appliedEnd = applied.constEnd();

    // Now create a sequence of lines for the destination file and the corresponding lines in source
    QStringList sourceLines;
    QStringList destinationLines;
    DifferenceListIterator insertPosition; // where to insert the created diffs
    if (appliedBegin == appliedEnd) {
        destinationLines = newLines;
        sourceLines = oldLines;
    } else {
        // Create the destination line sequence
        int firstDestinationLineNumber = (*appliedBegin)->trackingDestinationLineNumber();
        for (int lineNumber = firstDestinationLineNumber; lineNumber < editLineNumber; ++lineNumber) {
            destinationLines.append((*appliedBegin)->destinationLineAt(lineNumber - firstDestinationLineNumber)->string());
        }
        for (const QString &line : newLines) {
            destinationLines.append(line);
        }
        DifferenceListConstIterator appliedLast = appliedEnd;
        --appliedLast;
        int lastDestinationLineNumber = (*appliedLast)->trackingDestinationLineNumber();
        for (int lineNumber = editLineEnd; lineNumber < (*appliedLast)->trackingDestinationLineEnd(); ++lineNumber) {
            destinationLines.append((*appliedLast)->destinationLineAt(lineNumber - lastDestinationLineNumber)->string());
        }

        // Create the source line sequence
        if ((*appliedBegin)->trackingDestinationLineNumber() >= editLineNumber) {
            for (int i = editLineNumber; i < (*appliedBegin)->trackingDestinationLineNumber(); ++i) {
                sourceLines.append(oldLines.at(i - editLineNumber));
            }
        }

        for (DifferenceListConstIterator iter = appliedBegin; iter != appliedEnd;) {
            int startPos = (*iter)->trackingDestinationLineNumber();
            if ((*iter)->applied()) {
                for (int i = 0; i < (*iter)->sourceLineCount(); ++i) {
                    sourceLines.append((*iter)->sourceLineAt(i)->string());
                }
                startPos = (*iter)->trackingDestinationLineEnd();
            } else if (startPos < editLineNumber) {
                startPos = editLineNumber;
            }
            ++iter;
            int endPos = (iter == appliedEnd) ? editLineEnd : (*iter)->trackingDestinationLineNumber();
            for (int i = startPos; i < endPos; ++i) {
                sourceLines.append(oldLines.at(i - editLineNumber));
            }
        }
    }

    for (DifferenceListIterator iter = iterBegin; iter != iterEnd; ++iter) {
        removed << *iter;
    }
    insertPosition = d->differences.erase(iterBegin, iterEnd);

    // Compute the Levenshtein table for two line sequences and construct the shortest possible edit script
    StringListPair *pair = new StringListPair(sourceLines, destinationLines);
    LevenshteinTable<StringListPair> table;
    table.createTable(pair);
    table.createListsOfMarkers();
    MarkerList sourceMarkers = pair->markerListFirst();
    MarkerList destinationMarkers = pair->markerListSecond();

    int currentSourceListLine = 0;
    int currentDestinationListLine = 0;
    MarkerListConstIterator sourceMarkerIter = sourceMarkers.constBegin();
    MarkerListConstIterator destinationMarkerIter = destinationMarkers.constBegin();
    const int terminatorLineNumber = sourceLines.size() + destinationLines.size() + 1; // A virtual offset for simpler computation - stands for infinity

    // Process marker lists, converting pairs of Start-End markers into differences.
    // Marker in source list only stands for deletion, in source and destination lists - for change, in destination list only - for insertion.
    while (sourceMarkerIter != sourceMarkers.constEnd() || destinationMarkerIter != destinationMarkers.constEnd()) {
        int nextSourceListLine = sourceMarkerIter != sourceMarkers.constEnd() ? (*sourceMarkerIter)->offset() : terminatorLineNumber;
        int nextDestinationListLine = destinationMarkerIter != destinationMarkers.constEnd() ? (*destinationMarkerIter)->offset() : terminatorLineNumber;

        // Advance to the nearest marker
        int linesToSkip = qMin(nextDestinationListLine - currentDestinationListLine, nextSourceListLine - currentSourceListLine);
        currentSourceListLine += linesToSkip;
        currentDestinationListLine += linesToSkip;
        Difference *diff = new Difference(sourceLineNumber + currentSourceListLine, destinationLineNumber + currentDestinationListLine);
        if (nextSourceListLine == currentSourceListLine) {
            DiffModelPrivate::processStartMarker(diff, sourceLines, sourceMarkerIter, currentSourceListLine, true);
        }
        if (nextDestinationListLine == currentDestinationListLine) {
            DiffModelPrivate::processStartMarker(diff, destinationLines, destinationMarkerIter, currentDestinationListLine, false);
        }
        DiffModelPrivate::computeDiffStats(diff);
        Q_ASSERT(diff->type() != Difference::Unchanged);
        diff->applyQuietly(true);
        diff->setTrackingDestinationLineNumber(diff->destinationLineNumber());
        insertPosition = d->differences.insert(insertPosition, diff);
        ++insertPosition;
        inserted << diff;
    }
    // Update line numbers for differences that are after the edit
    for (; insertPosition != d->differences.end(); ++insertPosition) {
        (*insertPosition)->setTrackingDestinationLineNumber((*insertPosition)->trackingDestinationLineNumber() + (newLines.size() - oldLines.size()));
    }
    return qMakePair(inserted, removed);
}

#include "moc_diffmodel.cpp"

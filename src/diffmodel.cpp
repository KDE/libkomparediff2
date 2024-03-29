/*
SPDX-FileCopyrightText: 2001-2009 Otto Bruggeman <bruggie@gmail.com>
SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "diffmodel.h"

#include <komparediffdebug.h>
#include "difference.h"
#include "levenshteintable.h"
#include "stringlistpair.h"
#include "parserbase.h"

using namespace Diff2;

/**  */
DiffModel::DiffModel(const QString& source, const QString& destination) :
    m_source(source),
    m_destination(destination),
    m_sourcePath(),
    m_destinationPath(),
    m_sourceFile(),
    m_destinationFile(),
    m_sourceTimestamp(),
    m_destinationTimestamp(),
    m_sourceRevision(),
    m_destinationRevision(),
    m_appliedCount(0),
    m_diffIndex(0),
    m_selectedDifference(nullptr),
    m_blended(false)
{
    splitSourceInPathAndFileName();
    splitDestinationInPathAndFileName();
}

DiffModel::DiffModel() :
    m_source(),
    m_destination(),
    m_sourcePath(),
    m_destinationPath(),
    m_sourceFile(),
    m_destinationFile(),
    m_sourceTimestamp(),
    m_destinationTimestamp(),
    m_sourceRevision(),
    m_destinationRevision(),
    m_appliedCount(0),
    m_diffIndex(0),
    m_selectedDifference(nullptr),
    m_blended(false)
{
}

/**  */
DiffModel::~DiffModel()
{
    m_selectedDifference = nullptr;

    qDeleteAll(m_hunks);
    qDeleteAll(m_differences);
}

void DiffModel::splitSourceInPathAndFileName()
{
    int pos;

    if ((pos = m_source.lastIndexOf(QLatin1Char('/'))) >= 0)
        m_sourcePath = m_source.mid(0, pos + 1);

    if ((pos = m_source.lastIndexOf(QLatin1Char('/'))) >= 0)
        m_sourceFile = m_source.mid(pos + 1, m_source.length() - pos);
    else
        m_sourceFile = m_source;

    qCDebug(LIBKOMPAREDIFF2) << m_source << " was split into " << m_sourcePath << " and " << m_sourceFile;
}

void DiffModel::splitDestinationInPathAndFileName()
{
    int pos;

    if ((pos = m_destination.lastIndexOf(QLatin1Char('/'))) >= 0)
        m_destinationPath = m_destination.mid(0, pos + 1);

    if ((pos = m_destination.lastIndexOf(QLatin1Char('/'))) >= 0)
        m_destinationFile = m_destination.mid(pos + 1, m_destination.length() - pos);
    else
        m_destinationFile = m_destination;

    qCDebug(LIBKOMPAREDIFF2) << m_destination << " was split into " << m_destinationPath << " and " << m_destinationFile;
}

DiffModel& DiffModel::operator=(const DiffModel& model)
{
    if (&model != this)   // Guard from self-assignment
    {
        m_source = model.m_source;
        m_destination = model.m_destination;
        m_sourcePath = model.m_sourcePath;
        m_sourceFile = model.m_sourceFile;
        m_sourceTimestamp = model.m_sourceTimestamp;
        m_sourceRevision = model.m_sourceRevision;
        m_destinationPath = model.m_destinationPath;
        m_destinationFile = model.m_destinationFile;
        m_destinationTimestamp = model.m_destinationTimestamp;
        m_destinationRevision = model.m_destinationRevision;
        m_appliedCount = model.m_appliedCount;

        m_diffIndex = model.m_diffIndex;
        m_selectedDifference = model.m_selectedDifference;
    }

    return *this;
}

bool DiffModel::operator<(const DiffModel& model)
{
    if (localeAwareCompareSource(model) < 0)
        return true;
    return false;
}

int DiffModel::localeAwareCompareSource(const DiffModel& model)
{
    qCDebug(LIBKOMPAREDIFF2) << "Path: " << model.m_sourcePath;
    qCDebug(LIBKOMPAREDIFF2) << "File: " << model.m_sourceFile;

    int result = m_sourcePath.localeAwareCompare(model.m_sourcePath);

    if (result == 0)
        return m_sourceFile.localeAwareCompare(model.m_sourceFile);

    return result;
}

QString DiffModel::recreateDiff() const
{
    // For now we'll always return a diff in the diff format
    QString diff;

    // recreate header
    const QChar tab = QLatin1Char('\t');
    const QChar nl  = QLatin1Char('\n');
    diff += QStringLiteral("--- %1\t%2").arg(ParserBase::escapePath(m_source), m_sourceTimestamp);
    if (!m_sourceRevision.isEmpty())
        diff += tab + m_sourceRevision;
    diff += nl;
    diff += QStringLiteral("+++ %1\t%2").arg(ParserBase::escapePath(m_destination), m_destinationTimestamp);
    if (!m_destinationRevision.isEmpty())
        diff += tab + m_destinationRevision;
    diff += nl;

    // recreate body by iterating over the hunks
    DiffHunkListConstIterator hunkIt = m_hunks.begin();
    DiffHunkListConstIterator hEnd   = m_hunks.end();

    for (; hunkIt != hEnd; ++hunkIt)
    {
        if ((*hunkIt)->type() != DiffHunk::AddedByBlend)
            diff += (*hunkIt)->recreateHunk();
    }

    return diff;
}

Difference* DiffModel::firstDifference()
{
    qCDebug(LIBKOMPAREDIFF2) << "DiffModel::firstDifference()";
    m_diffIndex = 0;
    qCDebug(LIBKOMPAREDIFF2) << "m_diffIndex = " << m_diffIndex;

    m_selectedDifference = m_differences[ m_diffIndex ];

    return m_selectedDifference;
}

Difference* DiffModel::lastDifference()
{
    qCDebug(LIBKOMPAREDIFF2) << "DiffModel::lastDifference()";
    m_diffIndex = m_differences.count() - 1;
    qCDebug(LIBKOMPAREDIFF2) << "m_diffIndex = " << m_diffIndex;

    m_selectedDifference = m_differences[ m_diffIndex ];

    return m_selectedDifference;
}

Difference* DiffModel::prevDifference()
{
    qCDebug(LIBKOMPAREDIFF2) << "DiffModel::prevDifference()";
    if (m_diffIndex > 0 && --m_diffIndex < m_differences.count())
    {
        qCDebug(LIBKOMPAREDIFF2) << "m_diffIndex = " << m_diffIndex;
        m_selectedDifference = m_differences[ m_diffIndex ];
    }
    else
    {
        m_selectedDifference = nullptr;
        m_diffIndex = 0;
        qCDebug(LIBKOMPAREDIFF2) << "m_diffIndex = " << m_diffIndex;
    }

    return m_selectedDifference;
}

Difference* DiffModel::nextDifference()
{
    qCDebug(LIBKOMPAREDIFF2) << "DiffModel::nextDifference()";
    if (++m_diffIndex < m_differences.count())
    {
        qCDebug(LIBKOMPAREDIFF2) << "m_diffIndex = " << m_diffIndex;
        m_selectedDifference = m_differences[ m_diffIndex ];
    }
    else
    {
        m_selectedDifference = nullptr;
        m_diffIndex = 0; // just for safety...
        qCDebug(LIBKOMPAREDIFF2) << "m_diffIndex = " << m_diffIndex;
    }

    return m_selectedDifference;
}

const QString DiffModel::sourceFile() const
{
    return m_sourceFile;
}

const QString DiffModel::destinationFile() const
{
    return m_destinationFile;
}

const QString DiffModel::sourcePath() const
{
    return m_sourcePath;
}

const QString DiffModel::destinationPath() const
{
    return m_destinationPath;
}

void DiffModel::setSourceFile(QString path)
{
    m_source = path;
    splitSourceInPathAndFileName();
}

void DiffModel::setDestinationFile(QString path)
{
    m_destination = path;
    splitDestinationInPathAndFileName();
}

void DiffModel::setSourceTimestamp(QString timestamp)
{
    m_sourceTimestamp = timestamp;
}

void DiffModel::setDestinationTimestamp(QString timestamp)
{
    m_destinationTimestamp = timestamp;
}

void DiffModel::setSourceRevision(QString revision)
{
    m_sourceRevision = revision;
}

void DiffModel::setDestinationRevision(QString revision)
{
    m_destinationRevision = revision;
}

void DiffModel::addHunk(DiffHunk* hunk)
{
    m_hunks.append(hunk);
}

void DiffModel::addDiff(Difference* diff)
{
    m_differences.append(diff);
    connect(diff, &Difference::differenceApplied,
            this, &DiffModel::slotDifferenceApplied);
}

bool DiffModel::hasUnsavedChanges(void) const
{
    DifferenceListConstIterator diffIt = m_differences.begin();
    DifferenceListConstIterator endIt  = m_differences.end();

    for (; diffIt != endIt; ++diffIt)
    {
        if ((*diffIt)->isUnsaved())
            return true;
    }

    return false;
}

void DiffModel::applyDifference(bool apply)
{
    bool appliedState = m_selectedDifference->applied();
    if (appliedState == apply)
    {
        return;
    }
    if (apply && !m_selectedDifference->applied())
        ++m_appliedCount;
    else if (!apply && m_selectedDifference->applied())
        --m_appliedCount;

    m_selectedDifference->apply(apply);
}

static int GetDifferenceDelta(Difference* diff)
{
    int delta = diff->destinationLineCount() - diff->sourceLineCount();
    if (!diff->applied())
    {
        delta = -delta;
    }
    return delta;
}

void DiffModel::slotDifferenceApplied(Difference* diff)
{
    int delta = GetDifferenceDelta(diff);
    for (Difference *current : std::as_const(m_differences)) {
        if (current->destinationLineNumber() > diff->destinationLineNumber())
        {
            current->setTrackingDestinationLineNumber(current->trackingDestinationLineNumber() + delta);
        }
    }
}

void DiffModel::applyAllDifferences(bool apply)
{
    if (apply)
    {
        m_appliedCount = m_differences.count();
    }
    else
    {
        m_appliedCount = 0;
    }

    DifferenceListIterator diffIt = m_differences.begin();
    DifferenceListIterator dEnd   = m_differences.end();

    int totalDelta = 0;
    for (; diffIt != dEnd; ++diffIt)
    {
        (*diffIt)->setTrackingDestinationLineNumber((*diffIt)->trackingDestinationLineNumber() + totalDelta);
        bool appliedState = (*diffIt)->applied();
        if (appliedState == apply)
        {
            continue;
        }
        (*diffIt)->applyQuietly(apply);
        int currentDelta = GetDifferenceDelta(*diffIt);
        totalDelta += currentDelta;
    }
}

bool DiffModel::setSelectedDifference(Difference* diff)
{
    qCDebug(LIBKOMPAREDIFF2) << "diff = " << diff;
    qCDebug(LIBKOMPAREDIFF2) << "m_selectedDifference = " << m_selectedDifference;

    if (diff != m_selectedDifference)
    {
        if ((m_differences.indexOf(diff)) == -1)
            return false;
        // Do not set m_diffIndex if it cant be found
        m_diffIndex = m_differences.indexOf(diff);
        qCDebug(LIBKOMPAREDIFF2) << "m_diffIndex = " << m_diffIndex;
        m_selectedDifference = diff;
    }

    return true;
}

QPair<QList<Difference*>, QList<Difference*> > DiffModel::linesChanged(const QStringList& oldLines, const QStringList& newLines, int editLineNumber)
{
    // These two will be returned as the function result
    QList<Difference*> inserted;
    QList<Difference*> removed;
    if (oldLines.size() == 0 && newLines.size() == 0) {
        return qMakePair(QList<Difference*>(), QList<Difference*>());
    }
    int editLineEnd = editLineNumber + oldLines.size();
    // Find the range of differences [iterBegin, iterEnd) that should be updated
    // TODO: assume that differences are ordered by starting line. Check that this is always the case
    DifferenceList applied;
    DifferenceListIterator iterBegin; // first diff ending a line before editLineNo or later
    for (iterBegin = m_differences.begin(); iterBegin != m_differences.end(); ++iterBegin) {
        // If the difference ends a line before the edit starts, they should be merged if this difference is applied.
        // Also it should be merged if it starts on editLineNumber, otherwise there will be two markers for the same line
        int lineAfterLast = (*iterBegin)->trackingDestinationLineEnd();
        if (lineAfterLast > editLineNumber || (lineAfterLast == editLineNumber &&
                                               ((*iterBegin)->applied() || (*iterBegin)->trackingDestinationLineNumber() == editLineNumber))) {
            break;
        }
    }
    DifferenceListIterator iterEnd;
    for (iterEnd = iterBegin; iterEnd != m_differences.end(); ++iterEnd) {
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
    if (iterBegin == m_differences.end()) {    // All existing diffs are after the change
        destinationLineNumber = editLineNumber;
        if (!m_differences.isEmpty()) {
            sourceLineNumber = m_differences.last()->sourceLineEnd() - (m_differences.last()->trackingDestinationLineEnd() - editLineNumber);
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
    DifferenceListIterator insertPosition;  // where to insert the created diffs
    if (appliedBegin == appliedEnd) {
        destinationLines = newLines;
        sourceLines = oldLines;
    } else {
        // Create the destination line sequence
        int firstDestinationLineNumber = (*appliedBegin)->trackingDestinationLineNumber();
        for (int lineNumber = firstDestinationLineNumber; lineNumber < editLineNumber; ++lineNumber) {
            destinationLines.append((*appliedBegin)->destinationLineAt(lineNumber - firstDestinationLineNumber)->string());
        }
        for (const QString& line : newLines) {
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
    insertPosition = m_differences.erase(iterBegin, iterEnd);

    // Compute the Levenshtein table for two line sequences and construct the shortest possible edit script
    StringListPair* pair = new StringListPair(sourceLines, destinationLines);
    LevenshteinTable<StringListPair> table;
    table.createTable(pair);
    table.createListsOfMarkers();
    MarkerList sourceMarkers = pair->markerListFirst();
    MarkerList destinationMarkers = pair->markerListSecond();

    int currentSourceListLine = 0;
    int currentDestinationListLine = 0;
    MarkerListConstIterator sourceMarkerIter = sourceMarkers.constBegin();
    MarkerListConstIterator destinationMarkerIter = destinationMarkers.constBegin();
    const int terminatorLineNumber = sourceLines.size() + destinationLines.size() + 1;    // A virtual offset for simpler computation - stands for infinity

    // Process marker lists, converting pairs of Start-End markers into differences.
    // Marker in source list only stands for deletion, in source and destination lists - for change, in destination list only - for insertion.
    while (sourceMarkerIter != sourceMarkers.constEnd() || destinationMarkerIter != destinationMarkers.constEnd()) {
        int nextSourceListLine = sourceMarkerIter != sourceMarkers.constEnd() ? (*sourceMarkerIter)->offset() : terminatorLineNumber;
        int nextDestinationListLine = destinationMarkerIter != destinationMarkers.constEnd() ? (*destinationMarkerIter)->offset() : terminatorLineNumber;

        // Advance to the nearest marker
        int linesToSkip = qMin(nextDestinationListLine - currentDestinationListLine, nextSourceListLine - currentSourceListLine);
        currentSourceListLine += linesToSkip;
        currentDestinationListLine += linesToSkip;
        Difference* diff = new Difference(sourceLineNumber + currentSourceListLine, destinationLineNumber + currentDestinationListLine);
        if (nextSourceListLine == currentSourceListLine) {
            processStartMarker(diff, sourceLines, sourceMarkerIter, currentSourceListLine, true);
        }
        if (nextDestinationListLine == currentDestinationListLine) {
            processStartMarker(diff, destinationLines, destinationMarkerIter, currentDestinationListLine, false);
        }
        computeDiffStats(diff);
        Q_ASSERT(diff->type() != Difference::Unchanged);
        diff->applyQuietly(true);
        diff->setTrackingDestinationLineNumber(diff->destinationLineNumber());
        insertPosition = m_differences.insert(insertPosition, diff);
        ++insertPosition;
        inserted << diff;
    }
    // Update line numbers for differences that are after the edit
    for (; insertPosition != m_differences.end(); ++insertPosition) {
        (*insertPosition)->setTrackingDestinationLineNumber((*insertPosition)->trackingDestinationLineNumber() + (newLines.size() - oldLines.size()));
    }
    return qMakePair(inserted, removed);
}

// Some common computing after diff contents have been filled.
void DiffModel::computeDiffStats(Difference* diff)
{
    if (diff->sourceLineCount() > 0 && diff->destinationLineCount() > 0) {
        diff->setType(Difference::Change);
    } else if (diff->sourceLineCount() > 0) {
        diff->setType(Difference::Delete);
    } else if (diff->destinationLineCount() > 0) {
        diff->setType(Difference::Insert);
    }
    diff->determineInlineDifferences();
}

// Helper method to extract duplicate code from DiffModel::linesChanged
void DiffModel::processStartMarker(Difference* diff, const QStringList& lines, MarkerListConstIterator& currentMarker, int& currentListLine, bool isSource)
{
    Q_ASSERT((*currentMarker)->type() == Marker::Start);
    ++currentMarker;
    Q_ASSERT((*currentMarker)->type() == Marker::End);
    int nextDestinationListLine = (*currentMarker)->offset();
    for (; currentListLine < nextDestinationListLine; ++currentListLine) {
        if (isSource) {
            diff->addSourceLine(lines.at(currentListLine));
        } else {
            diff->addDestinationLine(lines.at(currentListLine));
        }
    }
    ++currentMarker;
    currentListLine = nextDestinationListLine;
}

#include "moc_diffmodel.cpp"

/* vim: set ts=4 sw=4 noet: */

/*
SPDX-FileCopyrightText: 2001-2009 Otto Bruggeman <bruggie@gmail.com>
SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "diffmodel_p.h"

// lib
#include <komparediff2_logging.h>

using namespace KompareDiff2;

void DiffModelPrivate::splitSourceInPathAndFileName()
{
    int pos;

    if ((pos = source.lastIndexOf(QLatin1Char('/'))) >= 0)
        sourcePath = source.mid(0, pos + 1);

    if ((pos = source.lastIndexOf(QLatin1Char('/'))) >= 0)
        sourceFile = source.mid(pos + 1, source.length() - pos);
    else
        sourceFile = source;

    qCDebug(KOMPAREDIFF2_LOG) << source << " was split into " << sourcePath << " and " << sourceFile;
}

void DiffModelPrivate::splitDestinationInPathAndFileName()
{
    int pos;

    if ((pos = destination.lastIndexOf(QLatin1Char('/'))) >= 0)
        destinationPath = destination.mid(0, pos + 1);

    if ((pos = destination.lastIndexOf(QLatin1Char('/'))) >= 0)
        destinationFile = destination.mid(pos + 1, destination.length() - pos);
    else
        destinationFile = destination;

    qCDebug(KOMPAREDIFF2_LOG) << destination << " was split into " << destinationPath << " and " << destinationFile;
}

// Some common computing after diff contents have been filled.
void DiffModelPrivate::computeDiffStats(Difference* diff)
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
void DiffModelPrivate::processStartMarker(Difference* diff, const QStringList& lines,
                                          MarkerListConstIterator& currentMarker, int& currentListLine,
                                          bool isSource)
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

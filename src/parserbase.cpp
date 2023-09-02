/*
SPDX-FileCopyrightText: 2002-2004,2009 Otto Bruggeman <bruggie@gmail.com>
SPDX-FileCopyrightText: 2007,2010 Kevin Kofler  <kevin.kofler@chello.at>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "parserbase.h"

#include <QObject>

#include <komparediffdebug.h>
#include "diffmodellist.h"
#include "diffmodel.h"
#include "diffhunk.h"
#include "difference.h"
#include "komparemodellist.h"

using namespace KompareDiff2;

// static
QString ParserBase::unescapePath(QString path)
{
    // If path contains spaces, it is enclosed in quotes
    if (path.startsWith(QLatin1Char('"')) && path.endsWith(QLatin1Char('"')))
        path = path.mid(1, path.size() - 2);

    // Unescape quotes
    path.replace(QLatin1String("\\\""), QLatin1String("\""));

#ifndef Q_OS_WIN
    // Unescape backquotes
    path.replace(QLatin1String("\\\\"), QLatin1String("\\"));
#endif

    return path;
}

// static
QString ParserBase::escapePath(QString path)
{
#ifndef Q_OS_WIN
    // Escape backquotes
    path.replace(QLatin1String("\\"), QLatin1String("\\\\"));
#endif

    // Escape quotes
    path.replace(QLatin1String("\""), QLatin1String("\\\""));

    // Enclose in quotes if path contains space
    if (path.contains(QLatin1Char(' ')))
        path = QLatin1Char('"') + path + QLatin1Char('"');

    return path;
}

ParserBase::ParserBase(const KompareModelList* list, const QStringList& diff) :
    m_diffLines(diff),
    m_currentModel(nullptr),
    m_models(nullptr),
    m_diffIterator(m_diffLines.begin()),
    m_singleFileDiff(false),
    m_malformed(false),
    m_list(list)
{
//     qCDebug(LIBKOMPAREDIFF2) << diff;
//     qCDebug(LIBKOMPAREDIFF2) << m_diffLines;
    m_models = new DiffModelList();

    // used in contexthunkheader
    m_contextHunkHeader1.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("\\*{15} ?(.*)\\n")));  // capture is for function name
    m_contextHunkHeader2.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("\\*\\*\\* ([0-9]+),([0-9]+) \\*\\*\\*\\*.*\\n")));
    // used in contexthunkbody
    m_contextHunkHeader3.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("--- ([0-9]+),([0-9]+) ----\\n")));

    m_contextHunkBodyRemoved.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("- (.*)\\n")));
    m_contextHunkBodyAdded.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("\\+ (.*)\\n")));
    m_contextHunkBodyChanged.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("! (.*)\\n")));
    m_contextHunkBodyContext.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("  (.*)\\n")));
    m_contextHunkBodyLine.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("[-\\+! ] (.*)\\n")));

    // This regexp sucks... i'll see what happens
    m_normalDiffHeader.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("diff (?:(?:-|--)[a-zA-Z0-9=\\\"]+ )*(?:|-- +)(.*) +(.*)\\n")));

    m_normalHunkHeaderAdded.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("([0-9]+)a([0-9]+)(|,[0-9]+)(.*)\\n")));
    m_normalHunkHeaderRemoved.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("([0-9]+)(|,[0-9]+)d([0-9]+)(.*)\\n")));
    m_normalHunkHeaderChanged.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("([0-9]+)(|,[0-9]+)c([0-9]+)(|,[0-9]+)(.*)\\n")));

    m_normalHunkBodyRemoved.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("< (.*)\\n")));
    m_normalHunkBodyAdded.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("> (.*)\\n")));
    m_normalHunkBodyDivider.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("---\\n")));

    m_unifiedDiffHeader1.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("--- ([^\\t]+)(?:\\t([^\\t]+)(?:\\t?)(.*))?\\n")));
    m_unifiedDiffHeader2.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("\\+\\+\\+ ([^\\t]+)(?:\\t([^\\t]+)(?:\\t?)(.*))?\\n")));
    m_unifiedHunkHeader.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("@@ -([0-9]+)(|,([0-9]+)) \\+([0-9]+)(|,([0-9]+)) @@(?: ?)(.*)\\n")));
}

ParserBase::~ParserBase()
{
    if (m_models)
        m_models = nullptr; // do not delete this, i pass it around...
}

enum Kompare::Format ParserBase::determineFormat()
{
    // Write your own format detection routine damn it :)
    return Kompare::UnknownFormat;
}

DiffModelList* ParserBase::parse(bool* malformed)
{
    DiffModelList* result;
    switch (determineFormat())
    {
    case Kompare::Context :
        result = parseContext();
        break;
    case Kompare::Ed :
        result = parseEd();
        break;
    case Kompare::Normal :
        result = parseNormal();
        break;
    case Kompare::RCS :
        result = parseRCS();
        break;
    case Kompare::Unified :
        result = parseUnified();
        break;
    default: // Unknown and SideBySide for now
        result = nullptr;
        break;
    }

    // *malformed is set to true if some hunks or parts of hunks were
    // probably missed due to a malformed diff
    if (malformed)
        *malformed = m_malformed;

    return result;
}

bool ParserBase::parseContextDiffHeader()
{
//     qCDebug(LIBKOMPAREDIFF2) << "ParserBase::parseContextDiffHeader()";
    bool result = false;

    while (m_diffIterator != m_diffLines.end())
    {
        const auto contextDiffHeader1Match = m_contextDiffHeader1.match(*(m_diffIterator)++);
        if (!contextDiffHeader1Match.hasMatch())
        {
            continue;
        }
//         qCDebug(LIBKOMPAREDIFF2) << "Matched length Header1 = " << contextDiffHeader1Match.capturedLength();
//         qCDebug(LIBKOMPAREDIFF2) << "Matched string Header1 = " << contextDiffHeader1Match.captured( 0 );
        const auto contextDiffHeader2Match = m_contextDiffHeader2.match(*m_diffIterator);
        if (m_diffIterator != m_diffLines.end() && contextDiffHeader2Match.hasMatch())
        {
//             qCDebug(LIBKOMPAREDIFF2) << "Matched length Header2 = " << contextDiffHeader2Match.capturedLength();
//             qCDebug(LIBKOMPAREDIFF2) << "Matched string Header2 = " << contextDiffHeader2Match.captured( 0 );

            m_currentModel = new DiffModel(unescapePath(contextDiffHeader1Match.captured(1)), unescapePath(contextDiffHeader2Match.captured(1)));
            m_currentModel->setSourceTimestamp(contextDiffHeader1Match.captured(3));
            m_currentModel->setSourceRevision(contextDiffHeader1Match.captured(5));
            m_currentModel->setDestinationTimestamp(contextDiffHeader2Match.captured(3));
            m_currentModel->setDestinationRevision(contextDiffHeader2Match.captured(5));

            ++m_diffIterator;
            result = true;

            break;
        }
        else
        {
            // We're screwed, second line does not match or is not there...
            break;
        }
        // Do not inc the Iterator because the second line might be the first line of
        // the context header and the first hit was a fluke (impossible imo)
        // maybe we should return false here because the diff is broken ?
    }

    return result;
}

bool ParserBase::parseEdDiffHeader()
{
    return false;
}

bool ParserBase::parseNormalDiffHeader()
{
//     qCDebug(LIBKOMPAREDIFF2) << "ParserBase::parseNormalDiffHeader()";
    bool result = false;

    while (m_diffIterator != m_diffLines.end())
    {
        const auto normalDiffHeaderMatch = m_normalDiffHeader.match(*m_diffIterator);
        if (normalDiffHeaderMatch.hasMatch())
        {
//             qCDebug(LIBKOMPAREDIFF2) << "Matched length Header = " << normalDiffHeaderMatch.capturedLength();
//             qCDebug(LIBKOMPAREDIFF2) << "Matched string Header = " << normalDiffHeaderMatch.captured( 0 );

            m_currentModel = new DiffModel();
            m_currentModel->setSourceFile(unescapePath(normalDiffHeaderMatch.captured(1)));
            m_currentModel->setDestinationFile(unescapePath(normalDiffHeaderMatch.captured(2)));

            result = true;

            ++m_diffIterator;
            break;
        }
        else
        {
            qCDebug(LIBKOMPAREDIFF2) << "No match for: " << (*m_diffIterator);
        }
        ++m_diffIterator;
    }

    if (result == false)
    {
        // Set this to the first line again and hope it is a single file diff
        m_diffIterator = m_diffLines.begin();
        m_currentModel = new DiffModel();
        m_singleFileDiff = true;
    }

    return result;
}

bool ParserBase::parseRCSDiffHeader()
{
    return false;
}

bool ParserBase::parseUnifiedDiffHeader()
{
//     qCDebug(LIBKOMPAREDIFF2) << "ParserBase::parseUnifiedDiffHeader()";
    bool result = false;

    while (m_diffIterator != m_diffLines.end())   // do not assume we start with the diffheader1 line
    {
        const auto unifiedDiffHeader1Match = m_unifiedDiffHeader1.match(*m_diffIterator);
        if (!unifiedDiffHeader1Match.hasMatch())
        {
            ++m_diffIterator;
            continue;
        }
//         qCDebug(LIBKOMPAREDIFF2) << "Matched length Header1 = " << unifiedDiffHeader1Match.capturedLength();
//         qCDebug(LIBKOMPAREDIFF2) << "Matched string Header1 = " << unifiedDiffHeader1Match.captured( 0 );
        ++m_diffIterator;
        const auto unifiedDiffHeader2Match = m_unifiedDiffHeader2.match(*m_diffIterator);
        if (m_diffIterator != m_diffLines.end() && unifiedDiffHeader2Match.hasMatch())
        {
            m_currentModel = new DiffModel(unescapePath(unifiedDiffHeader1Match.captured(1)), unescapePath(unifiedDiffHeader2Match.captured(1)));
            m_currentModel->setSourceTimestamp(unifiedDiffHeader1Match.captured(2));
            m_currentModel->setSourceRevision(unifiedDiffHeader1Match.captured(4));
            m_currentModel->setDestinationTimestamp(unifiedDiffHeader2Match.captured(2));
            m_currentModel->setDestinationRevision(unifiedDiffHeader2Match.captured(4));

            ++m_diffIterator;
            result = true;

            break;
        }
        else
        {
            // We're screwed, second line does not match or is not there...
            break;
        }
    }

    return result;
}

bool ParserBase::parseContextHunkHeader()
{
//     qCDebug(LIBKOMPAREDIFF2) << "ParserBase::parseContextHunkHeader()";

    if (m_diffIterator == m_diffLines.end())
        return false;

    m_contextHunkHeader1Match = m_contextHunkHeader1.match(*(m_diffIterator));
    if (!m_contextHunkHeader1Match.hasMatch())
        return false; // big fat trouble, aborting...

    ++m_diffIterator;

    if (m_diffIterator == m_diffLines.end())
        return false;

    m_contextHunkHeader2Match = m_contextHunkHeader2.match(*(m_diffIterator));
    if (!m_contextHunkHeader2Match.hasMatch())
        return false; // big fat trouble, aborting...

    ++m_diffIterator;

    return true;
}

bool ParserBase::parseEdHunkHeader()
{
    return false;
}

bool ParserBase::parseNormalHunkHeader()
{
//     qCDebug(LIBKOMPAREDIFF2) << "ParserBase::parseNormalHunkHeader()";
    if (m_diffIterator != m_diffLines.end())
    {
//         qCDebug(LIBKOMPAREDIFF2) << "Header = " << *m_diffIterator;
        if (m_normalHunkHeaderAddedMatch = m_normalHunkHeaderAdded.match(*m_diffIterator); m_normalHunkHeaderAddedMatch.hasMatch())
        {
            m_normalDiffType = Difference::Insert;
        }
        else if (m_normalHunkHeaderRemovedMatch = m_normalHunkHeaderRemoved.match(*m_diffIterator); m_normalHunkHeaderRemovedMatch.hasMatch())
        {
            m_normalDiffType = Difference::Delete;
        }
        else if (m_normalHunkHeaderChangedMatch = m_normalHunkHeaderChanged.match(*m_diffIterator); m_normalHunkHeaderChangedMatch.hasMatch())
        {
            m_normalDiffType = Difference::Change;
        }
        else
            return false;

        ++m_diffIterator;
        return true;
    }

    return false;
}

bool ParserBase::parseRCSHunkHeader()
{
    return false;
}

bool ParserBase::parseUnifiedHunkHeader()
{
//     qCDebug(LIBKOMPAREDIFF2) << "ParserBase::parseUnifiedHunkHeader()";

    if (m_diffIterator != m_diffLines.end())
    {
        m_unifiedHunkHeaderMatch = m_unifiedHunkHeader.match(*m_diffIterator);
        if (m_unifiedHunkHeaderMatch.hasMatch()) {
            ++m_diffIterator;
            return true;
        }
    }
//     qCDebug(LIBKOMPAREDIFF2) << "This is not a unified hunk header : " << (*m_diffIterator);
    return false;
}

bool ParserBase::parseContextHunkBody()
{
//     qCDebug(LIBKOMPAREDIFF2) << "ParserBase::parseContextHunkBody()";

    // Storing the src part of the hunk for later use
    QStringList oldLines;
    for (; m_diffIterator != m_diffLines.end() && m_contextHunkBodyLine.match(*m_diffIterator).hasMatch(); ++m_diffIterator) {
//         qCDebug(LIBKOMPAREDIFF2) << "Added old line: " << *m_diffIterator;
        oldLines.append(*m_diffIterator);
    }

    const auto contextHunkHeader3Match = m_contextHunkHeader3.match(*m_diffIterator);
    if (!contextHunkHeader3Match.hasMatch())
        return false;

    ++m_diffIterator;

    // Storing the dest part of the hunk for later use
    QStringList newLines;
    for (; m_diffIterator != m_diffLines.end() && m_contextHunkBodyLine.match(*m_diffIterator).hasMatch(); ++m_diffIterator) {
//         qCDebug(LIBKOMPAREDIFF2) << "Added new line: " << *m_diffIterator;
        newLines.append(*m_diffIterator);
    }

    QString function = m_contextHunkHeader1Match.captured(1);
//     qCDebug(LIBKOMPAREDIFF2) << "Captured function: " << function;
    int linenoA      = m_contextHunkHeader2Match.captured(1).toInt();
//     qCDebug(LIBKOMPAREDIFF2) << "Source line number: " << linenoA;
    int linenoB      = contextHunkHeader3Match.captured(1).toInt();
//     qCDebug(LIBKOMPAREDIFF2) << "Dest   line number: " << linenoB;

    DiffHunk* hunk = new DiffHunk(linenoA, linenoB, function);

    m_currentModel->addHunk(hunk);

    QStringList::Iterator oldIt = oldLines.begin();
    QStringList::Iterator newIt = newLines.begin();

    Difference* diff;
    while (oldIt != oldLines.end() || newIt != newLines.end())
    {
        if (oldIt != oldLines.end() && m_contextHunkBodyRemoved.match(*oldIt).hasMatch())
        {
//             qCDebug(LIBKOMPAREDIFF2) << "Delete: ";
            diff = new Difference(linenoA, linenoB);
            diff->setType(Difference::Delete);
            m_currentModel->addDiff(diff);
//             qCDebug(LIBKOMPAREDIFF2) << "Difference added";
            hunk->add(diff);
            for (; oldIt != oldLines.end(); ++oldIt)
            {
                const auto contextHunkBodyRemovedMatch = m_contextHunkBodyRemoved.match(*oldIt);
                if (!contextHunkBodyRemovedMatch.hasMatch()) {
                    break;
                }
//                 qCDebug(LIBKOMPAREDIFF2) << " " << contextHunkBodyRemovedMatch.captured( 1 );
                diff->addSourceLine(contextHunkBodyRemovedMatch.captured(1));
                ++linenoA;
            }
        }
        else if (newIt != newLines.end() && m_contextHunkBodyAdded.match(*newIt).hasMatch())
        {
//             qCDebug(LIBKOMPAREDIFF2) << "Insert: ";
            diff = new Difference(linenoA, linenoB);
            diff->setType(Difference::Insert);
            m_currentModel->addDiff(diff);
//             qCDebug(LIBKOMPAREDIFF2) << "Difference added";
            hunk->add(diff);
            for (; newIt != newLines.end(); ++newIt)
            {
                const auto contextHunkBodyAddedMatch = m_contextHunkBodyAdded.match(*newIt);
                if (!contextHunkBodyAddedMatch.hasMatch()) {
                    break;
                }
//                 qCDebug(LIBKOMPAREDIFF2) << " " << contextHunkBodyAddedMatch.captured( 1 );
                diff->addDestinationLine(contextHunkBodyAddedMatch.captured(1));
                ++linenoB;
            }
        }
        else if ((oldIt == oldLines.end() || m_contextHunkBodyContext.match(*oldIt).hasMatch()) &&
                 (newIt == newLines.end() || m_contextHunkBodyContext.match(*newIt).hasMatch()))
        {
//             qCDebug(LIBKOMPAREDIFF2) << "Unchanged: ";
            diff = new Difference(linenoA, linenoB);
            // Do not add this diff with addDiff to the model... no unchanged differences allowed in there...
            diff->setType(Difference::Unchanged);
            hunk->add(diff);
            while ((oldIt == oldLines.end() || m_contextHunkBodyContext.match(*oldIt).hasMatch()) &&
                    (newIt == newLines.end() || m_contextHunkBodyContext.match(*newIt).hasMatch()) &&
                    (oldIt != oldLines.end() || newIt != newLines.end()))
            {
                QString l;
                if (oldIt != oldLines.end())
                {
                    l = m_contextHunkBodyContext.match(*oldIt).captured(1);
//                     qCDebug(LIBKOMPAREDIFF2) << "old: " << l;
                    ++oldIt;
                }
                if (newIt != newLines.end())
                {
                    l = m_contextHunkBodyContext.match(*newIt).captured(1);
//                     qCDebug(LIBKOMPAREDIFF2) << "new: " << l;
                    ++newIt;
                }
                diff->addSourceLine(l);
                diff->addDestinationLine(l);
                ++linenoA;
                ++linenoB;
            }
        }
        else if ((oldIt != oldLines.end() && m_contextHunkBodyChanged.match(*oldIt).hasMatch()) ||
                 (newIt != newLines.end() && m_contextHunkBodyChanged.match(*newIt).hasMatch()))
        {
//             qCDebug(LIBKOMPAREDIFF2) << "Changed: ";
            diff = new Difference(linenoA, linenoB);
            diff->setType(Difference::Change);
            m_currentModel->addDiff(diff);
//             qCDebug(LIBKOMPAREDIFF2) << "Difference added";
            hunk->add(diff);
            while (oldIt != oldLines.end())
            {
                const auto contextHunkBodyChangedMatch = m_contextHunkBodyChanged.match(*oldIt);
                if (!contextHunkBodyChangedMatch.hasMatch()) {
                    break;
                }
//                 qCDebug(LIBKOMPAREDIFF2) << " " << contextHunkBodyChangedMatch.captured( 1 );
                diff->addSourceLine(contextHunkBodyChangedMatch.captured(1));
                ++linenoA;
                ++oldIt;
            }
            while (newIt != newLines.end())
            {
                const auto contextHunkBodyChangedMatch = m_contextHunkBodyChanged.match(*newIt);
                if (!contextHunkBodyChangedMatch.hasMatch()) {
                    break;
                }
//                 qCDebug(LIBKOMPAREDIFF2) << " " << contextHunkBodyChangedMatch.captured( 1 );
                diff->addDestinationLine(contextHunkBodyChangedMatch.captured(1));
                ++linenoB;
                ++newIt;
            }
        }
        else
            return false;
        diff->determineInlineDifferences();
    }

    return true;
}

bool ParserBase::parseEdHunkBody()
{
    return false;
}

bool ParserBase::parseNormalHunkBody()
{
//     qCDebug(LIBKOMPAREDIFF2) << "ParserBase::parseNormalHunkBody";

    QString type;

    int linenoA = 0, linenoB = 0;

    if (m_normalDiffType == Difference::Insert)
    {
        linenoA = m_normalHunkHeaderAddedMatch.captured(1).toInt();
        linenoB = m_normalHunkHeaderAddedMatch.captured(2).toInt();
    }
    else if (m_normalDiffType == Difference::Delete)
    {
        linenoA = m_normalHunkHeaderRemovedMatch.captured(1).toInt();
        linenoB = m_normalHunkHeaderRemovedMatch.captured(3).toInt();
    }
    else if (m_normalDiffType == Difference::Change)
    {
        linenoA = m_normalHunkHeaderChangedMatch.captured(1).toInt();
        linenoB = m_normalHunkHeaderChangedMatch.captured(3).toInt();
    }

    DiffHunk* hunk = new DiffHunk(linenoA, linenoB);
    m_currentModel->addHunk(hunk);
    Difference* diff = new Difference(linenoA, linenoB);
    hunk->add(diff);
    m_currentModel->addDiff(diff);

    diff->setType(m_normalDiffType);

    if (m_normalDiffType == Difference::Change || m_normalDiffType == Difference::Delete)
        for (; m_diffIterator != m_diffLines.end(); ++m_diffIterator)
        {
            const auto normalHunkBodyRemovedMatch = m_normalHunkBodyRemoved.match(*m_diffIterator);
            if (!normalHunkBodyRemovedMatch.hasMatch()) {
                break;
            }
//             qCDebug(LIBKOMPAREDIFF2) << "Line = " << *m_diffIterator;
            diff->addSourceLine(normalHunkBodyRemovedMatch.captured(1));
        }

    if (m_normalDiffType == Difference::Change)
    {
        if (m_diffIterator != m_diffLines.end() && m_normalHunkBodyDivider.match(*m_diffIterator).hasMatch())
        {
//             qCDebug(LIBKOMPAREDIFF2) << "Line = " << *m_diffIterator;
            ++m_diffIterator;
        }
        else
            return false;
    }

    if (m_normalDiffType == Difference::Insert || m_normalDiffType == Difference::Change)
        for (; m_diffIterator != m_diffLines.end(); ++m_diffIterator)
        {
             const auto normalHunkBodyAddedMatch = m_normalHunkBodyAdded.match(*m_diffIterator);
             if (!normalHunkBodyAddedMatch.hasMatch()) {
                 break;
             }
//             qCDebug(LIBKOMPAREDIFF2) << "Line = " << *m_diffIterator;
            diff->addDestinationLine(normalHunkBodyAddedMatch.captured(1));
        }

    return true;
}

bool ParserBase::parseRCSHunkBody()
{
    return false;
}

bool ParserBase::matchesUnifiedHunkLine(const QString& line) const
{
    static const QChar context = QLatin1Char(' ');
    static const QChar added = QLatin1Char('+');
    static const QChar removed = QLatin1Char('-');

    QChar first = line[0];

    return (first == context || first == added || first == removed);
}

bool ParserBase::parseUnifiedHunkBody()
{
//     qCDebug(LIBKOMPAREDIFF2) << "ParserBase::parseUnifiedHunkBody";

    int linenoA = 0, linenoB = 0;
    bool wasNum;

    // Fetching the stuff we need from the hunkheader regexp that was parsed in parseUnifiedHunkHeader();
    linenoA = m_unifiedHunkHeaderMatch.captured(1).toInt();
    int lineCountA = 1, lineCountB = 1; // an omitted line count in the header implies a line count of 1
    if (!m_unifiedHunkHeaderMatch.captured(3).isEmpty())
    {
        lineCountA = m_unifiedHunkHeaderMatch.captured(3).toInt(&wasNum);
        if (!wasNum)
            return false;

        // If a hunk is an insertion or deletion with no context, the line number given
        // is the one before the hunk. this isn't what we want, so increment it to fix this.
        if (lineCountA == 0)
            ++linenoA;
    }
    linenoB = m_unifiedHunkHeaderMatch.captured(4).toInt();
    if (!m_unifiedHunkHeaderMatch.captured(6).isEmpty()) {
        lineCountB = m_unifiedHunkHeaderMatch.captured(6).toInt(&wasNum);
        if (!wasNum)
            return false;

        if (lineCountB == 0) // see above
            ++linenoB;
    }
    QString function = m_unifiedHunkHeaderMatch.captured(7);

    DiffHunk* hunk = new DiffHunk(linenoA, linenoB, function);
    m_currentModel->addHunk(hunk);

    const QStringList::ConstIterator m_diffLinesEnd = m_diffLines.end();

    const QString context = QStringLiteral(" ");
    const QString added   = QStringLiteral("+");
    const QString removed = QStringLiteral("-");

    while (m_diffIterator != m_diffLinesEnd && matchesUnifiedHunkLine(*m_diffIterator) && (lineCountA || lineCountB))
    {
        Difference* diff = new Difference(linenoA, linenoB);
        hunk->add(diff);

        if ((*m_diffIterator).startsWith(context))
        {   // context
            for (; m_diffIterator != m_diffLinesEnd && (*m_diffIterator).startsWith(context) && (lineCountA || lineCountB); ++m_diffIterator)
            {
                diff->addSourceLine(QString(*m_diffIterator).remove(0, 1));
                diff->addDestinationLine(QString(*m_diffIterator).remove(0, 1));
                ++linenoA;
                ++linenoB;
                --lineCountA;
                --lineCountB;
            }
        }
        else
        {   // This is a real difference, not context
            for (; m_diffIterator != m_diffLinesEnd && (*m_diffIterator).startsWith(removed) && (lineCountA || lineCountB); ++m_diffIterator)
            {
                diff->addSourceLine(QString(*m_diffIterator).remove(0, 1));
                ++linenoA;
                --lineCountA;
            }
            for (; m_diffIterator != m_diffLinesEnd && (*m_diffIterator).startsWith(added) && (lineCountA || lineCountB); ++m_diffIterator)
            {
                diff->addDestinationLine(QString(*m_diffIterator).remove(0, 1));
                ++linenoB;
                --lineCountB;
            }
            if (diff->sourceLineCount() == 0)
            {
                diff->setType(Difference::Insert);
//                 qCDebug(LIBKOMPAREDIFF2) << "Insert difference";
            }
            else if (diff->destinationLineCount() == 0)
            {
                diff->setType(Difference::Delete);
//                 qCDebug(LIBKOMPAREDIFF2) << "Delete difference";
            }
            else
            {
                diff->setType(Difference::Change);
//                 qCDebug(LIBKOMPAREDIFF2) << "Change difference";
            }
            diff->determineInlineDifferences();
            m_currentModel->addDiff(diff);
        }
    }

    return true;
}

void ParserBase::checkHeader(const QRegularExpression& header)
{
    if (m_diffIterator != m_diffLines.end()
            && !header.match(*m_diffIterator).hasMatch()
            && !m_diffIterator->startsWith(QLatin1String("Index: ")) /* SVN diff */
            && !m_diffIterator->startsWith(QLatin1String("diff ")) /* concatenated diff */
            && !m_diffIterator->startsWith(QLatin1String("-- ")) /* git format-patch */)
        m_malformed = true;
}

DiffModelList* ParserBase::parseContext()
{
    while (parseContextDiffHeader())
    {
        while (parseContextHunkHeader())
            parseContextHunkBody();
        if (m_currentModel->differenceCount() > 0)
            m_models->append(m_currentModel);
        checkHeader(m_contextDiffHeader1);
    }

    m_models->sort();

    if (m_models->count() > 0)
    {
        return m_models;
    }
    else
    {
        delete m_models;
        return nullptr;
    }
}

DiffModelList* ParserBase::parseEd()
{
    while (parseEdDiffHeader())
    {
        while (parseEdHunkHeader())
            parseEdHunkBody();
        if (m_currentModel->differenceCount() > 0)
            m_models->append(m_currentModel);
    }

    m_models->sort();

    if (m_models->count() > 0)
    {
        return m_models;
    }
    else
    {
        delete m_models;
        return nullptr;
    }
}

DiffModelList* ParserBase::parseNormal()
{
    while (parseNormalDiffHeader())
    {
        while (parseNormalHunkHeader())
            parseNormalHunkBody();
        if (m_currentModel->differenceCount() > 0)
            m_models->append(m_currentModel);
        checkHeader(m_normalDiffHeader);
    }

    if (m_singleFileDiff)
    {
        while (parseNormalHunkHeader())
            parseNormalHunkBody();
        if (m_currentModel->differenceCount() > 0)
            m_models->append(m_currentModel);
        if (m_diffIterator != m_diffLines.end())
            m_malformed = true;
    }

    m_models->sort();

    if (m_models->count() > 0)
    {
        return m_models;
    }
    else
    {
        delete m_models;
        return nullptr;
    }
}

DiffModelList* ParserBase::parseRCS()
{
    while (parseRCSDiffHeader())
    {
        while (parseRCSHunkHeader())
            parseRCSHunkBody();
        if (m_currentModel->differenceCount() > 0)
            m_models->append(m_currentModel);
    }

    m_models->sort();

    if (m_models->count() > 0)
    {
        return m_models;
    }
    else
    {
        delete m_models;
        return nullptr;
    }
}

DiffModelList* ParserBase::parseUnified()
{
    while (parseUnifiedDiffHeader())
    {
        while (parseUnifiedHunkHeader())
            parseUnifiedHunkBody();
//         qCDebug(LIBKOMPAREDIFF2) << "New model ready to be analyzed...";
//         qCDebug(LIBKOMPAREDIFF2) << " differenceCount() == " << m_currentModel->differenceCount();
        if (m_currentModel->differenceCount() > 0)
            m_models->append(m_currentModel);
        checkHeader(m_unifiedDiffHeader1);
    }

    m_models->sort();

    if (m_models->count() > 0)
    {
        return m_models;
    }
    else
    {
        delete m_models;
        return nullptr;
    }
}


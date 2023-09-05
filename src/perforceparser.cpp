/*
SPDX-FileCopyrightText: 2002-2004 Otto Bruggeman <otto.bruggeman@home.nl>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "perforceparser.h"

#include <QRegularExpression>

#include <komparediff2_logging.h>
#include "diffmodel.h"

using namespace KompareDiff2;

PerforceParser::PerforceParser(const ModelList* list, const QStringList& diff) : ParserBase(list, diff)
{
    m_contextDiffHeader1.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("==== (.*) - (.*) ====\\n")));
    m_contextDiffHeader1.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    m_normalDiffHeader.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("==== (.*) - (.*) ====\\n")));
    m_normalDiffHeader.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    m_rcsDiffHeader.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("==== (.*) - (.*) ====\\n")));
    m_rcsDiffHeader.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    m_unifiedDiffHeader1.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("==== (.*) - (.*) ====\\n")));
    m_unifiedDiffHeader1.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
}

PerforceParser::~PerforceParser()
{
}

enum Format PerforceParser::determineFormat()
{
    qCDebug(KOMPAREDIFF2_LOG) << "Determining the format of the Perforce Diff";

    QRegularExpression unifiedRE(QStringLiteral("^@@"));
    QRegularExpression contextRE(QStringLiteral("^\\*{15}"));
    QRegularExpression normalRE(QStringLiteral("^\\d+(|,\\d+)[acd]\\d+(|,\\d+)"));
    QRegularExpression rcsRE(QStringLiteral("^[acd]\\d+ \\d+"));
    // Summary is not supported since it gives no useful parsable info

    QStringList::ConstIterator it = m_diffLines.begin();

    while (it != m_diffLines.end())
    {
        if (it->indexOf(unifiedRE, 0) == 0)
        {
            qCDebug(KOMPAREDIFF2_LOG) << "Difflines are from a Unified diff...";
            return Unified;
        }
        else if (it->indexOf(contextRE, 0) == 0)
        {
            qCDebug(KOMPAREDIFF2_LOG) << "Difflines are from a Context diff...";
            return Context;
        }
        else if (it->indexOf(normalRE, 0) == 0)
        {
            qCDebug(KOMPAREDIFF2_LOG) << "Difflines are from a Normal diff...";
            return Normal;
        }
        else if (it->indexOf(rcsRE, 0) == 0)
        {
            qCDebug(KOMPAREDIFF2_LOG) << "Difflines are from a RCS diff...";
            return RCS;
        }
        ++it;
    }
    qCDebug(KOMPAREDIFF2_LOG) << "Difflines are from an unknown diff...";
    return UnknownFormat;
}

bool PerforceParser::parseContextDiffHeader()
{
//     qCDebug(KOMPAREDIFF2_LOG) << "ParserBase::parseContextDiffHeader()";
    bool result = false;

    QStringList::ConstIterator itEnd = m_diffLines.end();

    const QRegularExpression sourceFileRE(QRegularExpression::anchoredPattern(QStringLiteral("([^\\#]+)#(\\d+)")));
    const QRegularExpression destinationFileRE(QRegularExpression::anchoredPattern(QStringLiteral("([^\\#]+)#(|\\d+)")));

    while (m_diffIterator != itEnd)
    {
        const auto contextDiffHeader1Match = m_contextDiffHeader1.match(*(m_diffIterator)++);
        if (contextDiffHeader1Match.hasMatch())
        {
//             qCDebug(KOMPAREDIFF2_LOG) << "Matched length Header1 = " << contextDiffHeader1Match.capturedLength();
//             qCDebug(KOMPAREDIFF2_LOG) << "Matched string Header1 = " << contextDiffHeader1Match.captured( 0 );
//             qCDebug(KOMPAREDIFF2_LOG) << "First capture  Header1 = " << contextDiffHeader1Match.captured( 1 );
//             qCDebug(KOMPAREDIFF2_LOG) << "Second capture Header1 = " << contextDiffHeader1Match.captured( 2 );

            m_currentModel = new DiffModel();
            const auto sourceFileREMatch = sourceFileRE.match(contextDiffHeader1Match.captured(1));
            const auto destinationFileREMatch = destinationFileRE.match(contextDiffHeader1Match.captured(2));
            qCDebug(KOMPAREDIFF2_LOG) << "Matched length   = " << sourceFileREMatch.capturedLength();
            qCDebug(KOMPAREDIFF2_LOG) << "Matched length   = " << destinationFileREMatch.capturedLength();
            qCDebug(KOMPAREDIFF2_LOG) << "Captured texts   = " << sourceFileREMatch.capturedTexts();
            qCDebug(KOMPAREDIFF2_LOG) << "Captured texts   = " << destinationFileREMatch.capturedTexts();
            qCDebug(KOMPAREDIFF2_LOG) << "Source File      : " << sourceFileREMatch.captured(1);
            qCDebug(KOMPAREDIFF2_LOG) << "Destination File : " << destinationFileREMatch.captured(1);
            m_currentModel->setSourceFile(sourceFileREMatch.captured(1));
            m_currentModel->setDestinationFile(destinationFileREMatch.captured(1));

            result = true;

            break;
        }
        else
        {
            qCDebug(KOMPAREDIFF2_LOG) << "Matched length = " << contextDiffHeader1Match.capturedLength();
            qCDebug(KOMPAREDIFF2_LOG) << "Captured texts = " << contextDiffHeader1Match.capturedTexts();
        }
    }

    return result;
}

bool PerforceParser::parseNormalDiffHeader()
{
    bool result = false;

    QStringList::ConstIterator itEnd = m_diffLines.end();

    QRegularExpression sourceFileRE(QRegularExpression::anchoredPattern(QStringLiteral("([^\\#]+)#(\\d+)")));
    QRegularExpression destinationFileRE(QRegularExpression::anchoredPattern(QStringLiteral("([^\\#]+)#(|\\d+)")));

    while (m_diffIterator != itEnd)
    {
        qCDebug(KOMPAREDIFF2_LOG) << "Line = " << *m_diffIterator;
        qCDebug(KOMPAREDIFF2_LOG) << "String length  = " << (*m_diffIterator).length();
        const auto normalDiffHeaderMatch = m_normalDiffHeader.match(*(m_diffIterator)++);
        if (normalDiffHeaderMatch.hasMatch())
        {
            qCDebug(KOMPAREDIFF2_LOG) << "Matched length Header1 = " << normalDiffHeaderMatch.capturedLength();
            qCDebug(KOMPAREDIFF2_LOG) << "Matched string Header1 = " << normalDiffHeaderMatch.captured(0);
            qCDebug(KOMPAREDIFF2_LOG) << "First  capture Header1 = \"" << normalDiffHeaderMatch.captured(1) << "\"";
            qCDebug(KOMPAREDIFF2_LOG) << "Second capture Header1 = \"" << normalDiffHeaderMatch.captured(2) << "\"";

            m_currentModel = new DiffModel();
            const auto sourceFileREMatch = sourceFileRE.match(normalDiffHeaderMatch.captured(1));
            const auto destinationFileREMatch = destinationFileRE.match(normalDiffHeaderMatch.captured(2));
            qCDebug(KOMPAREDIFF2_LOG) << "Matched length   = " << sourceFileREMatch.capturedLength();
            qCDebug(KOMPAREDIFF2_LOG) << "Matched length   = " << destinationFileREMatch.capturedLength();
            qCDebug(KOMPAREDIFF2_LOG) << "Captured texts   = " << sourceFileREMatch.capturedTexts();
            qCDebug(KOMPAREDIFF2_LOG) << "Captured texts   = " << destinationFileREMatch.capturedTexts();
            qCDebug(KOMPAREDIFF2_LOG) << "Source File      : " << sourceFileREMatch.captured(1);
            qCDebug(KOMPAREDIFF2_LOG) << "Destination File : " << destinationFileREMatch.captured(1);
            m_currentModel->setSourceFile(sourceFileREMatch.captured(1));
            m_currentModel->setDestinationFile(destinationFileREMatch.captured(1));

            result = true;

            break;
        }
        else
        {
            qCDebug(KOMPAREDIFF2_LOG) << "Matched length = " << normalDiffHeaderMatch.capturedLength();
            qCDebug(KOMPAREDIFF2_LOG) << "Captured texts = " << normalDiffHeaderMatch.capturedTexts();
        }
    }

    return result;
}

bool PerforceParser::parseRCSDiffHeader()
{
    return false;
}

bool PerforceParser::parseUnifiedDiffHeader()
{
    bool result = false;

    QStringList::ConstIterator itEnd = m_diffLines.end();

    QRegularExpression sourceFileRE(QRegularExpression::anchoredPattern(QStringLiteral("([^\\#]+)#(\\d+)")));
    QRegularExpression destinationFileRE(QRegularExpression::anchoredPattern(QStringLiteral("([^\\#]+)#(|\\d+)")));

    while (m_diffIterator != itEnd)
    {
//         qCDebug(KOMPAREDIFF2_LOG) << "Line = " << *m_diffIterator;
//         qCDebug(KOMPAREDIFF2_LOG) << "String length  = " << (*m_diffIterator).length();
        const auto unifiedDiffHeader1Match = m_unifiedDiffHeader1.match(*(m_diffIterator)++);
        if (unifiedDiffHeader1Match.hasMatch())
        {
//             qCDebug(KOMPAREDIFF2_LOG) << "Matched length Header1 = " << unifiedDiffHeader1Match.capturedLength();
//             qCDebug(KOMPAREDIFF2_LOG) << "Matched string Header1 = " << unifiedDiffHeader1Match.captured( 0 );
//             qCDebug(KOMPAREDIFF2_LOG) << "First  capture Header1 = \"" << unifiedDiffHeader1Match.captured( 1 ) << "\"";
//             qCDebug(KOMPAREDIFF2_LOG) << "Second capture Header1 = \"" << unifiedDiffHeader1Match.captured( 2 ) << "\"";

            m_currentModel = new DiffModel();
            const auto sourceFileREMatch = sourceFileRE.match(unifiedDiffHeader1Match.captured(1));
            const auto destinationFileREMatch = destinationFileRE.match(unifiedDiffHeader1Match.captured(2));
//             qCDebug(KOMPAREDIFF2_LOG) << "Matched length   = " << sourceFileREMatch.capturedLength();
//             qCDebug(KOMPAREDIFF2_LOG) << "Matched length   = " << destinationFileREMatch.capturedLength();
//             qCDebug(KOMPAREDIFF2_LOG) << "Captured texts   = " << sourceFileREMatch.capturedTexts();
//             qCDebug(KOMPAREDIFF2_LOG) << "Captured texts   = " << destinationFileREMatch.capturedTexts();
//             qCDebug(KOMPAREDIFF2_LOG) << "Source File      : " << sourceFileREMatch.captured( 1 );
//             qCDebug(KOMPAREDIFF2_LOG) << "Destination File : " << destinationFileREMatch.captured( 1 );
            m_currentModel->setSourceFile(sourceFileREMatch.captured(1));
            m_currentModel->setDestinationFile(destinationFileREMatch.captured(1));

            result = true;

            break;
        }
        else
        {
//             qCDebug(KOMPAREDIFF2_LOG) << "Matched length = " << unifiedDiffHeader1Match.capturedLength();
//             qCDebug(KOMPAREDIFF2_LOG) << "Captured texts = " << unifiedDiffHeader1Match.capturedTexts();
        }
    }

    return result;
}


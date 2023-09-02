/*
SPDX-FileCopyrightText: 2002-2004 Otto Bruggeman <otto.bruggeman@home.nl>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "perforceparser.h"

#include <QRegularExpression>

#include <komparediffdebug.h>
#include "diffmodel.h"

using namespace KompareDiff2;

PerforceParser::PerforceParser(const KompareModelList* list, const QStringList& diff) : ParserBase(list, diff)
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

enum Kompare::Format PerforceParser::determineFormat()
{
    qCDebug(LIBKOMPAREDIFF2) << "Determining the format of the Perforce Diff";

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
            qCDebug(LIBKOMPAREDIFF2) << "Difflines are from a Unified diff...";
            return Kompare::Unified;
        }
        else if (it->indexOf(contextRE, 0) == 0)
        {
            qCDebug(LIBKOMPAREDIFF2) << "Difflines are from a Context diff...";
            return Kompare::Context;
        }
        else if (it->indexOf(normalRE, 0) == 0)
        {
            qCDebug(LIBKOMPAREDIFF2) << "Difflines are from a Normal diff...";
            return Kompare::Normal;
        }
        else if (it->indexOf(rcsRE, 0) == 0)
        {
            qCDebug(LIBKOMPAREDIFF2) << "Difflines are from a RCS diff...";
            return Kompare::RCS;
        }
        ++it;
    }
    qCDebug(LIBKOMPAREDIFF2) << "Difflines are from an unknown diff...";
    return Kompare::UnknownFormat;
}

bool PerforceParser::parseContextDiffHeader()
{
//     qCDebug(LIBKOMPAREDIFF2) << "ParserBase::parseContextDiffHeader()";
    bool result = false;

    QStringList::ConstIterator itEnd = m_diffLines.end();

    const QRegularExpression sourceFileRE(QRegularExpression::anchoredPattern(QStringLiteral("([^\\#]+)#(\\d+)")));
    const QRegularExpression destinationFileRE(QRegularExpression::anchoredPattern(QStringLiteral("([^\\#]+)#(|\\d+)")));

    while (m_diffIterator != itEnd)
    {
        const auto contextDiffHeader1Match = m_contextDiffHeader1.match(*(m_diffIterator)++);
        if (contextDiffHeader1Match.hasMatch())
        {
//             qCDebug(LIBKOMPAREDIFF2) << "Matched length Header1 = " << contextDiffHeader1Match.capturedLength();
//             qCDebug(LIBKOMPAREDIFF2) << "Matched string Header1 = " << contextDiffHeader1Match.captured( 0 );
//             qCDebug(LIBKOMPAREDIFF2) << "First capture  Header1 = " << contextDiffHeader1Match.captured( 1 );
//             qCDebug(LIBKOMPAREDIFF2) << "Second capture Header1 = " << contextDiffHeader1Match.captured( 2 );

            m_currentModel = new DiffModel();
            const auto sourceFileREMatch = sourceFileRE.match(contextDiffHeader1Match.captured(1));
            const auto destinationFileREMatch = destinationFileRE.match(contextDiffHeader1Match.captured(2));
            qCDebug(LIBKOMPAREDIFF2) << "Matched length   = " << sourceFileREMatch.capturedLength();
            qCDebug(LIBKOMPAREDIFF2) << "Matched length   = " << destinationFileREMatch.capturedLength();
            qCDebug(LIBKOMPAREDIFF2) << "Captured texts   = " << sourceFileREMatch.capturedTexts();
            qCDebug(LIBKOMPAREDIFF2) << "Captured texts   = " << destinationFileREMatch.capturedTexts();
            qCDebug(LIBKOMPAREDIFF2) << "Source File      : " << sourceFileREMatch.captured(1);
            qCDebug(LIBKOMPAREDIFF2) << "Destination File : " << destinationFileREMatch.captured(1);
            m_currentModel->setSourceFile(sourceFileREMatch.captured(1));
            m_currentModel->setDestinationFile(destinationFileREMatch.captured(1));

            result = true;

            break;
        }
        else
        {
            qCDebug(LIBKOMPAREDIFF2) << "Matched length = " << contextDiffHeader1Match.capturedLength();
            qCDebug(LIBKOMPAREDIFF2) << "Captured texts = " << contextDiffHeader1Match.capturedTexts();
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
        qCDebug(LIBKOMPAREDIFF2) << "Line = " << *m_diffIterator;
        qCDebug(LIBKOMPAREDIFF2) << "String length  = " << (*m_diffIterator).length();
        const auto normalDiffHeaderMatch = m_normalDiffHeader.match(*(m_diffIterator)++);
        if (normalDiffHeaderMatch.hasMatch())
        {
            qCDebug(LIBKOMPAREDIFF2) << "Matched length Header1 = " << normalDiffHeaderMatch.capturedLength();
            qCDebug(LIBKOMPAREDIFF2) << "Matched string Header1 = " << normalDiffHeaderMatch.captured(0);
            qCDebug(LIBKOMPAREDIFF2) << "First  capture Header1 = \"" << normalDiffHeaderMatch.captured(1) << "\"";
            qCDebug(LIBKOMPAREDIFF2) << "Second capture Header1 = \"" << normalDiffHeaderMatch.captured(2) << "\"";

            m_currentModel = new DiffModel();
            const auto sourceFileREMatch = sourceFileRE.match(normalDiffHeaderMatch.captured(1));
            const auto destinationFileREMatch = destinationFileRE.match(normalDiffHeaderMatch.captured(2));
            qCDebug(LIBKOMPAREDIFF2) << "Matched length   = " << sourceFileREMatch.capturedLength();
            qCDebug(LIBKOMPAREDIFF2) << "Matched length   = " << destinationFileREMatch.capturedLength();
            qCDebug(LIBKOMPAREDIFF2) << "Captured texts   = " << sourceFileREMatch.capturedTexts();
            qCDebug(LIBKOMPAREDIFF2) << "Captured texts   = " << destinationFileREMatch.capturedTexts();
            qCDebug(LIBKOMPAREDIFF2) << "Source File      : " << sourceFileREMatch.captured(1);
            qCDebug(LIBKOMPAREDIFF2) << "Destination File : " << destinationFileREMatch.captured(1);
            m_currentModel->setSourceFile(sourceFileREMatch.captured(1));
            m_currentModel->setDestinationFile(destinationFileREMatch.captured(1));

            result = true;

            break;
        }
        else
        {
            qCDebug(LIBKOMPAREDIFF2) << "Matched length = " << normalDiffHeaderMatch.capturedLength();
            qCDebug(LIBKOMPAREDIFF2) << "Captured texts = " << normalDiffHeaderMatch.capturedTexts();
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
//         qCDebug(LIBKOMPAREDIFF2) << "Line = " << *m_diffIterator;
//         qCDebug(LIBKOMPAREDIFF2) << "String length  = " << (*m_diffIterator).length();
        const auto unifiedDiffHeader1Match = m_unifiedDiffHeader1.match(*(m_diffIterator)++);
        if (unifiedDiffHeader1Match.hasMatch())
        {
//             qCDebug(LIBKOMPAREDIFF2) << "Matched length Header1 = " << unifiedDiffHeader1Match.capturedLength();
//             qCDebug(LIBKOMPAREDIFF2) << "Matched string Header1 = " << unifiedDiffHeader1Match.captured( 0 );
//             qCDebug(LIBKOMPAREDIFF2) << "First  capture Header1 = \"" << unifiedDiffHeader1Match.captured( 1 ) << "\"";
//             qCDebug(LIBKOMPAREDIFF2) << "Second capture Header1 = \"" << unifiedDiffHeader1Match.captured( 2 ) << "\"";

            m_currentModel = new DiffModel();
            const auto sourceFileREMatch = sourceFileRE.match(unifiedDiffHeader1Match.captured(1));
            const auto destinationFileREMatch = destinationFileRE.match(unifiedDiffHeader1Match.captured(2));
//             qCDebug(LIBKOMPAREDIFF2) << "Matched length   = " << sourceFileREMatch.capturedLength();
//             qCDebug(LIBKOMPAREDIFF2) << "Matched length   = " << destinationFileREMatch.capturedLength();
//             qCDebug(LIBKOMPAREDIFF2) << "Captured texts   = " << sourceFileREMatch.capturedTexts();
//             qCDebug(LIBKOMPAREDIFF2) << "Captured texts   = " << destinationFileREMatch.capturedTexts();
//             qCDebug(LIBKOMPAREDIFF2) << "Source File      : " << sourceFileREMatch.captured( 1 );
//             qCDebug(LIBKOMPAREDIFF2) << "Destination File : " << destinationFileREMatch.captured( 1 );
            m_currentModel->setSourceFile(sourceFileREMatch.captured(1));
            m_currentModel->setDestinationFile(destinationFileREMatch.captured(1));

            result = true;

            break;
        }
        else
        {
//             qCDebug(LIBKOMPAREDIFF2) << "Matched length = " << unifiedDiffHeader1Match.capturedLength();
//             qCDebug(LIBKOMPAREDIFF2) << "Captured texts = " << unifiedDiffHeader1Match.capturedTexts();
        }
    }

    return result;
}


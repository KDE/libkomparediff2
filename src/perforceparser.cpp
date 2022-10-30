/*
SPDX-FileCopyrightText: 2002-2004 Otto Bruggeman <otto.bruggeman@home.nl>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "perforceparser.h"

#include <QRegularExpression>

#include <komparediffdebug.h>
#include "diffmodel.h"

using namespace Diff2;

PerforceParser::PerforceParser(const KompareModelList* list, const QStringList& diff) : ParserBase(list, diff)
{
    m_contextDiffHeader1.setPattern(QStringLiteral("==== (.*) - (.*) ====\\n"));
    m_contextDiffHeader1.setMinimal(true);
    m_normalDiffHeader.setPattern(QStringLiteral("==== (.*) - (.*) ====\\n"));
    m_normalDiffHeader.setMinimal(true);
    m_rcsDiffHeader.setPattern(QStringLiteral("==== (.*) - (.*) ====\\n"));
    m_rcsDiffHeader.setMinimal(true);
    m_unifiedDiffHeader1.setPattern(QStringLiteral("==== (.*) - (.*) ====\\n"));
    m_unifiedDiffHeader1.setMinimal(true);
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
        if (m_contextDiffHeader1.exactMatch(*(m_diffIterator)++))
        {
//             qCDebug(LIBKOMPAREDIFF2) << "Matched length Header1 = " << m_contextDiffHeader1.matchedLength();
//             qCDebug(LIBKOMPAREDIFF2) << "Matched string Header1 = " << m_contextDiffHeader1.cap( 0 );
//             qCDebug(LIBKOMPAREDIFF2) << "First capture  Header1 = " << m_contextDiffHeader1.cap( 1 );
//             qCDebug(LIBKOMPAREDIFF2) << "Second capture Header1 = " << m_contextDiffHeader1.cap( 2 );

            m_currentModel = new DiffModel();
            const auto sourceFileREMatch = sourceFileRE.match(m_contextDiffHeader1.cap(1));
            const auto destinationFileREMatch = destinationFileRE.match(m_contextDiffHeader1.cap(2));
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
            qCDebug(LIBKOMPAREDIFF2) << "Matched length = " << m_contextDiffHeader1.matchedLength();
            qCDebug(LIBKOMPAREDIFF2) << "Captured texts = " << m_contextDiffHeader1.capturedTexts();
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
        if (m_normalDiffHeader.exactMatch(*(m_diffIterator)++))
        {
            qCDebug(LIBKOMPAREDIFF2) << "Matched length Header1 = " << m_normalDiffHeader.matchedLength();
            qCDebug(LIBKOMPAREDIFF2) << "Matched string Header1 = " << m_normalDiffHeader.cap(0);
            qCDebug(LIBKOMPAREDIFF2) << "First  capture Header1 = \"" << m_normalDiffHeader.cap(1) << "\"";
            qCDebug(LIBKOMPAREDIFF2) << "Second capture Header1 = \"" << m_normalDiffHeader.cap(2) << "\"";

            m_currentModel = new DiffModel();
            const auto sourceFileREMatch = sourceFileRE.match(m_normalDiffHeader.cap(1));
            const auto destinationFileREMatch = destinationFileRE.match(m_normalDiffHeader.cap(2));
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
            qCDebug(LIBKOMPAREDIFF2) << "Matched length = " << m_normalDiffHeader.matchedLength();
            qCDebug(LIBKOMPAREDIFF2) << "Captured texts = " << m_normalDiffHeader.capturedTexts();
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
        if (m_unifiedDiffHeader1.exactMatch(*(m_diffIterator)++))
        {
//             qCDebug(LIBKOMPAREDIFF2) << "Matched length Header1 = " << m_unifiedDiffHeader1.matchedLength();
//             qCDebug(LIBKOMPAREDIFF2) << "Matched string Header1 = " << m_unifiedDiffHeader1.cap( 0 );
//             qCDebug(LIBKOMPAREDIFF2) << "First  capture Header1 = \"" << m_unifiedDiffHeader1.cap( 1 ) << "\"";
//             qCDebug(LIBKOMPAREDIFF2) << "Second capture Header1 = \"" << m_unifiedDiffHeader1.cap( 2 ) << "\"";

            m_currentModel = new DiffModel();
            const auto sourceFileREMatch = sourceFileRE.match(m_unifiedDiffHeader1.cap(1));
            const auto destinationFileREMatch = destinationFileRE.match(m_unifiedDiffHeader1.cap(2));
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
//             qCDebug(LIBKOMPAREDIFF2) << "Matched length = " << m_unifiedDiffHeader1.matchedLength();
//             qCDebug(LIBKOMPAREDIFF2) << "Captured texts = " << m_unifiedDiffHeader1.capturedTexts();
        }
    }

    return result;
}


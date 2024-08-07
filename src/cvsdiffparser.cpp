/*
    SPDX-FileCopyrightText: 2002-2004 Otto Bruggeman <otto.bruggeman@home.nl>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "cvsdiffparser.h"

// lib
#include "modellist.h"
#include <komparediff2_logging.h>
// Qt
#include <QRegularExpression>

using namespace KompareDiff2;

CVSDiffParser::CVSDiffParser(const ModelList *list, const QStringList &diff)
    : ParserBase(list, diff)
{
    // The regexps needed for context cvs diff parsing, the rest is the same as in parserbase.cpp
    // third capture in header1 is non optional for cvs diff, it is the revision
    m_contextDiffHeader1.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("\\*\\*\\* ([^\\t]+)\\t([^\\t]+)\\t(.*)\\n")));
    m_contextDiffHeader2.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("--- ([^\\t]+)\\t([^\\t]+)(|\\t(.*))\\n")));

    m_normalDiffHeader.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("Index: (.*)\\n")));
}

CVSDiffParser::~CVSDiffParser() = default;

Format CVSDiffParser::determineFormat()
{
    //     qCDebug(KOMPAREDIFF2_LOG) << "Determining the format of the CVSDiff";

    QRegularExpression normalRE(QStringLiteral("[0-9]+[0-9,]*[acd][0-9]+[0-9,]*"));
    QRegularExpression unifiedRE(QStringLiteral("^--- [^\\t]+\\t"));
    QRegularExpression contextRE(QStringLiteral("^\\*\\*\\* [^\\t]+\\t"));
    QRegularExpression rcsRE(QStringLiteral("^[acd][0-9]+ [0-9]+"));
    QRegularExpression edRE(QStringLiteral("^[0-9]+[0-9,]*[acd]"));

    for (const QString &diffLine : std::as_const(m_diffLines)) {
        if (diffLine.indexOf(normalRE, 0) == 0) {
//             qCDebug(KOMPAREDIFF2_LOG) << "Difflines are from a Normal diff...";
            return Normal;
        }
        if (diffLine.indexOf(unifiedRE, 0) == 0) {
//             qCDebug(KOMPAREDIFF2_LOG) << "Difflines are from a Unified diff...";
            return Unified;
        }
        if (diffLine.indexOf(contextRE, 0) == 0) {
//             qCDebug(KOMPAREDIFF2_LOG) << "Difflines are from a Context diff...";
            return Context;
        }
        if (diffLine.indexOf(rcsRE, 0) == 0) {
//             qCDebug(KOMPAREDIFF2_LOG) << "Difflines are from a RCS diff...";
            return RCS;
        }
        if (diffLine.indexOf(edRE, 0) == 0) {
//             qCDebug(KOMPAREDIFF2_LOG) << "Difflines are from an ED diff...";
            return Ed;
        }
    }
    //     qCDebug(KOMPAREDIFF2_LOG) << "Difflines are from an unknown diff...";
    return UnknownFormat;
}

bool CVSDiffParser::parseNormalDiffHeader()
{
    qCDebug(KOMPAREDIFF2_LOG) << "CVSDiffParser::parseNormalDiffHeader()";
    bool result = false;

    QStringList::ConstIterator diffEnd = m_diffLines.end();

    while (m_diffIterator != diffEnd) {
        const auto normalDiffHeaderMatch = m_normalDiffHeader.match(*m_diffIterator);
        if (normalDiffHeaderMatch.hasMatch()) {
            qCDebug(KOMPAREDIFF2_LOG) << "Matched length Header = " << normalDiffHeaderMatch.capturedLength();
            qCDebug(KOMPAREDIFF2_LOG) << "Matched string Header = " << normalDiffHeaderMatch.captured(0);

            m_currentModel = new DiffModel();
            m_currentModel->setSourceFile(normalDiffHeaderMatch.captured(1));
            m_currentModel->setDestinationFile(normalDiffHeaderMatch.captured(1));

            result = true;

            ++m_diffIterator;
            break;
        } else {
            qCDebug(KOMPAREDIFF2_LOG) << "No match for: " << (*m_diffIterator);
        }
        ++m_diffIterator;
    }

    if (result == false) {
        // Set this to the first line again and hope it is a single file diff
        m_diffIterator = m_diffLines.begin();
        m_currentModel = new DiffModel();
        m_singleFileDiff = true;
    }

    return result;
}

bool CVSDiffParser::parseEdDiffHeader()
{
    return false;
}

bool CVSDiffParser::parseRCSDiffHeader()
{
    return false;
}

bool CVSDiffParser::parseEdHunkHeader()
{
    return false;
}

bool CVSDiffParser::parseRCSHunkHeader()
{
    return false;
}

bool CVSDiffParser::parseEdHunkBody()
{
    return false;
}

bool CVSDiffParser::parseRCSHunkBody()
{
    return false;
}

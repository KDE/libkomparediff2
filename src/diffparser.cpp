/*
    SPDX-FileCopyrightText: 2002-2004 Otto Bruggeman <otto.bruggeman@home.nl>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "diffparser.h"

// lib
#include <komparediff2_logging.h>
// Qt
#include <QRegularExpression>

using namespace KompareDiff2;

DiffParser::DiffParser(const ModelList *list, const QStringList &diff)
    : ParserBase(list, diff)
{
    // The regexps needed for context diff parsing, the rest is the same as in parserbase.cpp
    m_contextDiffHeader1.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("\\*\\*\\* ([^\\t]+)(\\t([^\\t]+))?\\n")));
    m_contextDiffHeader2.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("--- ([^\\t]+)(\\t([^\\t]+))?\\n")));
}

DiffParser::~DiffParser() = default;

Format DiffParser::determineFormat()
{
    qCDebug(KOMPAREDIFF2_LOG) << "Determining the format of the diff Diff" << m_diffLines;

    QRegularExpression normalRE(QStringLiteral("[0-9]+[0-9,]*[acd][0-9]+[0-9,]*"));
    QRegularExpression unifiedRE(QStringLiteral("^--- "));
    QRegularExpression contextRE(QStringLiteral("^\\*\\*\\* "));
    QRegularExpression rcsRE(QStringLiteral("^[acd][0-9]+ [0-9]+"));
    QRegularExpression edRE(QStringLiteral("^[0-9]+[0-9,]*[acd]"));

    for (const QString &diffLine : std::as_const(m_diffLines)) {
        qCDebug(KOMPAREDIFF2_LOG) << diffLine;
        if (diffLine.indexOf(normalRE, 0) == 0) {
            qCDebug(KOMPAREDIFF2_LOG) << "Difflines are from a Normal diff...";
            return Normal;
        }
        if (diffLine.indexOf(unifiedRE, 0) == 0) {
            qCDebug(KOMPAREDIFF2_LOG) << "Difflines are from a Unified diff...";
            return Unified;
        }
        if (diffLine.indexOf(contextRE, 0) == 0) {
            qCDebug(KOMPAREDIFF2_LOG) << "Difflines are from a Context diff...";
            return Context;
        }
        if (diffLine.indexOf(rcsRE, 0) == 0) {
            qCDebug(KOMPAREDIFF2_LOG) << "Difflines are from an RCS diff...";
            return RCS;
        }
        if (diffLine.indexOf(edRE, 0) == 0) {
            qCDebug(KOMPAREDIFF2_LOG) << "Difflines are from an ED diff...";
            return Ed;
        }
    }
    qCDebug(KOMPAREDIFF2_LOG) << "Difflines are from an unknown diff...";
    return UnknownFormat;
}

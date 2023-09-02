/*
SPDX-FileCopyrightText: 2002-2004 Otto Bruggeman <otto.bruggeman@home.nl>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "diffparser.h"

#include <QRegularExpression>

#include <komparediffdebug.h>

using namespace KompareDiff2;

DiffParser::DiffParser(const KompareModelList* list, const QStringList& diff) : ParserBase(list, diff)
{
    // The regexps needed for context diff parsing, the rest is the same as in parserbase.cpp
    m_contextDiffHeader1.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("\\*\\*\\* ([^\\t]+)(\\t([^\\t]+))?\\n")));
    m_contextDiffHeader2.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("--- ([^\\t]+)(\\t([^\\t]+))?\\n")));
}

DiffParser::~DiffParser()
{
}

enum Kompare::Format DiffParser::determineFormat()
{
    qCDebug(LIBKOMPAREDIFF2) << "Determining the format of the diff Diff" << m_diffLines;

    QRegularExpression normalRE(QStringLiteral("[0-9]+[0-9,]*[acd][0-9]+[0-9,]*"));
    QRegularExpression unifiedRE(QStringLiteral("^--- "));
    QRegularExpression contextRE(QStringLiteral("^\\*\\*\\* "));
    QRegularExpression rcsRE(QStringLiteral("^[acd][0-9]+ [0-9]+"));
    QRegularExpression edRE(QStringLiteral("^[0-9]+[0-9,]*[acd]"));

    QStringList::ConstIterator it = m_diffLines.begin();

    while (it != m_diffLines.end())
    {
        qCDebug(LIBKOMPAREDIFF2) << (*it);
        if (it->indexOf(normalRE, 0) == 0)
        {
            qCDebug(LIBKOMPAREDIFF2) << "Difflines are from a Normal diff...";
            return Kompare::Normal;
        }
        else if (it->indexOf(unifiedRE, 0) == 0)
        {
            qCDebug(LIBKOMPAREDIFF2) << "Difflines are from a Unified diff...";
            return Kompare::Unified;
        }
        else if (it->indexOf(contextRE, 0) == 0)
        {
            qCDebug(LIBKOMPAREDIFF2) << "Difflines are from a Context diff...";
            return Kompare::Context;
        }
        else if (it->indexOf(rcsRE, 0) == 0)
        {
            qCDebug(LIBKOMPAREDIFF2) << "Difflines are from an RCS diff...";
            return Kompare::RCS;
        }
        else if (it->indexOf(edRE, 0) == 0)
        {
            qCDebug(LIBKOMPAREDIFF2) << "Difflines are from an ED diff...";
            return Kompare::Ed;
        }
        ++it;
    }
    qCDebug(LIBKOMPAREDIFF2) << "Difflines are from an unknown diff...";
    return Kompare::UnknownFormat;
}

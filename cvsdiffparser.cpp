/*
SPDX-FileCopyrightText: 2002-2004 Otto Bruggeman <otto.bruggeman@home.nl>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "cvsdiffparser.h"

#include <QRegExp>

#include <komparediffdebug.h>
#include "komparemodellist.h"

using namespace Diff2;

CVSDiffParser::CVSDiffParser(const KompareModelList* list, const QStringList& diff) : ParserBase(list, diff)
{
    // The regexps needed for context cvs diff parsing, the rest is the same as in parserbase.cpp
    // third capture in header1 is non optional for cvs diff, it is the revision
    m_contextDiffHeader1.setPattern(QStringLiteral("\\*\\*\\* ([^\\t]+)\\t([^\\t]+)\\t(.*)\\n"));
    m_contextDiffHeader2.setPattern(QStringLiteral("--- ([^\\t]+)\\t([^\\t]+)(|\\t(.*))\\n"));

    m_normalDiffHeader.setPattern(QStringLiteral("Index: (.*)\\n"));
}

CVSDiffParser::~CVSDiffParser()
{
}

enum Kompare::Format CVSDiffParser::determineFormat()
{
//     qCDebug(LIBKOMPAREDIFF2) << "Determining the format of the CVSDiff";

    QRegExp normalRE(QStringLiteral("[0-9]+[0-9,]*[acd][0-9]+[0-9,]*"));
    QRegExp unifiedRE(QStringLiteral("^--- [^\\t]+\\t"));
    QRegExp contextRE(QStringLiteral("^\\*\\*\\* [^\\t]+\\t"));
    QRegExp rcsRE(QStringLiteral("^[acd][0-9]+ [0-9]+"));
    QRegExp edRE(QStringLiteral("^[0-9]+[0-9,]*[acd]"));

    QStringList::ConstIterator it = m_diffLines.begin();

    while (it != m_diffLines.end())
    {
        if ((*it).indexOf(normalRE, 0) == 0)
        {
//             qCDebug(LIBKOMPAREDIFF2) << "Difflines are from a Normal diff...";
            return Kompare::Normal;
        }
        else if ((*it).indexOf(unifiedRE, 0) == 0)
        {
//             qCDebug(LIBKOMPAREDIFF2) << "Difflines are from a Unified diff...";
            return Kompare::Unified;
        }
        else if ((*it).indexOf(contextRE, 0) == 0)
        {
//             qCDebug(LIBKOMPAREDIFF2) << "Difflines are from a Context diff...";
            return Kompare::Context;
        }
        else if ((*it).indexOf(rcsRE, 0) == 0)
        {
//             qCDebug(LIBKOMPAREDIFF2) << "Difflines are from a RCS diff...";
            return Kompare::RCS;
        }
        else if ((*it).indexOf(edRE, 0) == 0)
        {
//             qCDebug(LIBKOMPAREDIFF2) << "Difflines are from an ED diff...";
            return Kompare::Ed;
        }
        ++it;
    }
//     qCDebug(LIBKOMPAREDIFF2) << "Difflines are from an unknown diff...";
    return Kompare::UnknownFormat;
}

bool CVSDiffParser::parseNormalDiffHeader()
{
    qCDebug(LIBKOMPAREDIFF2) << "CVSDiffParser::parseNormalDiffHeader()";
    bool result = false;

    QStringList::ConstIterator diffEnd = m_diffLines.end();

    while (m_diffIterator != diffEnd)
    {
        if (m_normalDiffHeader.exactMatch(*m_diffIterator))
        {
            qCDebug(LIBKOMPAREDIFF2) << "Matched length Header = " << m_normalDiffHeader.matchedLength();
            qCDebug(LIBKOMPAREDIFF2) << "Matched string Header = " << m_normalDiffHeader.cap(0);

            m_currentModel = new DiffModel();
            m_currentModel->setSourceFile(m_normalDiffHeader.cap(1));
            m_currentModel->setDestinationFile(m_normalDiffHeader.cap(1));

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

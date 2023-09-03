/*
SPDX-FileCopyrightText: 2002-2004 Otto Bruggeman <otto.bruggeman@home.nl>
SPDX-FileCopyrightText: 2010 Kevin Kofler <kevin.kofler@chello.at>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "parser.h"

#include <komparediffdebug.h>
#include "cvsdiffparser.h"
#include "diffparser.h"
#include "perforceparser.h"
#include "diffmodel.h"
#include "diffmodellist.h"

using namespace KompareDiff2;

Parser::Parser(const ModelList* list) :
    m_list(list)
{
}

Parser::~Parser()
{
}

int Parser::cleanUpCrap(QStringList& diffLines)
{
    QStringList::Iterator it = diffLines.begin();

    int nol = 0;

    QLatin1String noNewLine("\\ No newline");

    for (; it != diffLines.end(); ++it)
    {
        if ((*it).startsWith(noNewLine))
        {
            it = diffLines.erase(it);
            // correcting the advance of the iterator because of the remove
            --it;
            QString temp(*it);
            temp.truncate(temp.indexOf(QLatin1Char('\n')));
            *it = temp;
            ++nol;
        }
    }

    return nol;
}

DiffModelList* Parser::parse(QStringList& diffLines, bool* malformed)
{
    /* Basically determine the generator then call the parse method */
    ParserBase* parser;

    m_generator = determineGenerator(diffLines);

    int nol = cleanUpCrap(diffLines);
    qCDebug(LIBKOMPAREDIFF2) << "Cleaned up " << nol << " line(s) of crap from the diff...";

    switch (m_generator)
    {
    case CVSDiff :
        qCDebug(LIBKOMPAREDIFF2) << "It is a CVS generated diff...";
        parser = new CVSDiffParser(m_list, diffLines);
        break;
    case Diff :
        qCDebug(LIBKOMPAREDIFF2) << "It is a diff generated diff...";
        parser = new DiffParser(m_list, diffLines);
        break;
    case Perforce :
        qCDebug(LIBKOMPAREDIFF2) << "It is a Perforce generated diff...";
        parser = new PerforceParser(m_list, diffLines);
        break;
    default:
        // Nothing to delete, just leave...
        return nullptr;
    }

    m_format = parser->format();
    DiffModelList* modelList = parser->parse(malformed);
    if (modelList)
    {
        qCDebug(LIBKOMPAREDIFF2) << "Modelcount: " << modelList->count();
        DiffModelListIterator modelIt = modelList->begin();
        DiffModelListIterator mEnd    = modelList->end();
        for (; modelIt != mEnd; ++modelIt)
        {
            qCDebug(LIBKOMPAREDIFF2) << "Hunkcount:  " << (*modelIt)->hunkCount();
            qCDebug(LIBKOMPAREDIFF2) << "Diffcount:  " << (*modelIt)->differenceCount();
        }
    }

    delete parser;

    return modelList;
}

enum Generator Parser::determineGenerator(const QStringList& diffLines)
{
    // Shit have to duplicate some code with this method and the ParserBase derived classes
    QLatin1String cvsDiff("Index: ");
    QLatin1String perforceDiff("==== ");

    QStringList::ConstIterator it = diffLines.begin();
    QStringList::ConstIterator linesEnd = diffLines.end();

    while (it != linesEnd)
    {
        if ((*it).startsWith(cvsDiff))
        {
            qCDebug(LIBKOMPAREDIFF2) << "Diff is a CVSDiff";
            return CVSDiff;
        }
        else if ((*it).startsWith(perforceDiff))
        {
            qCDebug(LIBKOMPAREDIFF2) << "Diff is a Perforce Diff";
            return Perforce;
        }
        ++it;
    }

    qCDebug(LIBKOMPAREDIFF2) << "We'll assume it is a diff Diff";

    // For now we'll assume it is a diff file diff, later we might
    // try to really determine if it is a diff file diff.
    return Diff;
}

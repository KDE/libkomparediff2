/*
    SPDX-FileCopyrightText: 2002-2004 Otto Bruggeman <otto.bruggeman@home.nl>
    SPDX-FileCopyrightText: 2010 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "parser.h"

// lib
#include "cvsdiffparser.h"
#include "diffmodel.h"
#include "diffmodellist.h"
#include "diffparser.h"
#include "perforceparser.h"
#include <komparediff2_logging.h>

using namespace KompareDiff2;

Parser::Parser(const ModelList *list)
    : m_list(list)
{
}

Parser::~Parser() = default;

int Parser::cleanUpCrap(QStringList &diffLines)
{
    QStringList::Iterator it = diffLines.begin();

    int nol = 0;

    QLatin1String noNewLine("\\ No newline");

    for (; it != diffLines.end(); ++it) {
        if ((*it).startsWith(noNewLine)) {
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

DiffModelList *Parser::parse(QStringList &diffLines, bool *malformed)
{
    /* Basically determine the generator then call the parse method */
    std::unique_ptr<ParserBase> parser;

    m_generator = determineGenerator(diffLines);

    int nol = cleanUpCrap(diffLines);
    qCDebug(KOMPAREDIFF2_LOG) << "Cleaned up " << nol << " line(s) of crap from the diff...";

    switch (m_generator) {
    case CVSDiff:
        qCDebug(KOMPAREDIFF2_LOG) << "It is a CVS generated diff...";
        parser = std::make_unique<CVSDiffParser>(m_list, diffLines);
        break;
    case Diff:
        qCDebug(KOMPAREDIFF2_LOG) << "It is a diff generated diff...";
        parser = std::make_unique<DiffParser>(m_list, diffLines);
        break;
    case Perforce:
        qCDebug(KOMPAREDIFF2_LOG) << "It is a Perforce generated diff...";
        parser = std::make_unique<PerforceParser>(m_list, diffLines);
        break;
    default:
        // Nothing to delete, just leave...
        return nullptr;
    }

    m_format = parser->format();
    DiffModelList *modelList = parser->parse(malformed);
    if (modelList) {
        qCDebug(KOMPAREDIFF2_LOG) << "Modelcount: " << modelList->count();
        for (const DiffModel *model : std::as_const(*modelList)) {
            qCDebug(KOMPAREDIFF2_LOG) << "Hunkcount:  " << model->hunkCount();
            qCDebug(KOMPAREDIFF2_LOG) << "Diffcount:  " << model->differenceCount();
        }
    }

    return modelList;
}

Generator Parser::determineGenerator(const QStringList &diffLines)
{
    // Shit have to duplicate some code with this method and the ParserBase derived classes
    QLatin1String cvsDiff("Index: ");
    QLatin1String perforceDiff("==== ");

    for (const QString &diffLine : diffLines) {
        if (diffLine.startsWith(cvsDiff)) {
            qCDebug(KOMPAREDIFF2_LOG) << "Diff is a CVSDiff";
            return CVSDiff;
        }
        if (diffLine.startsWith(perforceDiff)) {
            qCDebug(KOMPAREDIFF2_LOG) << "Diff is a Perforce Diff";
            return Perforce;
        }
    }

    qCDebug(KOMPAREDIFF2_LOG) << "We'll assume it is a diff Diff";

    // For now we'll assume it is a diff file diff, later we might
    // try to really determine if it is a diff file diff.
    return Diff;
}

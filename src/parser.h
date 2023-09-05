/*
    SPDX-FileCopyrightText: 2002-2004 Otto Bruggeman <otto.bruggeman@home.nl>
    SPDX-FileCopyrightText: 2010 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_PARSER_H
#define KOMPAREDIFF2_PARSER_H

// lib
#include "global.h"
// Qt
#include <QStringList>

namespace KompareDiff2
{

class DiffModelList;
class ModelList;

class Parser
{
public:
    explicit Parser(const ModelList *list);
    ~Parser();

public:
    DiffModelList *parse(QStringList &diffLines, bool *malformed = nullptr);

    Generator generator() const
    {
        return m_generator;
    };

    Format format() const
    {
        return m_format;
    };

private:
    /** Which program was used to generate the output */
    Generator determineGenerator(const QStringList &diffLines);

    int cleanUpCrap(QStringList &diffLines);

private:
    Generator m_generator;
    Format m_format;

    const ModelList *m_list;
};

} // End of namespace KompareDiff2

#endif

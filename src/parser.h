/*
SPDX-FileCopyrightText: 2002-2004 Otto Bruggeman <otto.bruggeman@home.nl>
SPDX-FileCopyrightText: 2010 Kevin Kofler <kevin.kofler@chello.at>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_PARSER_H
#define KOMPAREDIFF2_PARSER_H

#include "kompare.h"

namespace Diff2
{

class DiffModelList;
class KompareModelList;

class Parser
{
public:
    explicit Parser(const KompareModelList* list);
    ~Parser();

public:
    DiffModelList* parse(QStringList& diffLines, bool* malformed = nullptr);

    enum Kompare::Generator generator() const { return m_generator; };
    enum Kompare::Format    format() const    { return m_format; };

private:
    /** Which program was used to generate the output */
    enum Kompare::Generator determineGenerator(const QStringList& diffLines);

    int cleanUpCrap(QStringList& diffLines);

private:
    enum Kompare::Generator m_generator;
    enum Kompare::Format    m_format;

    const KompareModelList* m_list;
};

} // End of namespace Diff2

#endif


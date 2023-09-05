/*
SPDX-FileCopyrightText: 2002-2004 Otto Bruggeman <otto.bruggeman@home.nl>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_DIFFPARSER_H
#define KOMPAREDIFF2_DIFFPARSER_H

// lib
#include "parserbase.h"

namespace KompareDiff2
{

class DiffParser : public ParserBase
{
public:
    DiffParser(const ModelList* list, const QStringList& diff);
    ~DiffParser() override;

protected:
    enum Format determineFormat() override;
};

} // End of namespace KompareDiff2

#endif

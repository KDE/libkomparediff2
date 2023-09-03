/*
SPDX-FileCopyrightText: 2002-2004 Otto Bruggeman <otto.bruggeman@home.nl>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_DIFFPARSER_H
#define KOMPAREDIFF2_DIFFPARSER_H

#include "parserbase.h"

namespace Diff2
{

class DiffParser : public ParserBase
{
public:
    DiffParser(const KompareModelList* list, const QStringList& diff);
    ~DiffParser() override;

protected:
    enum Kompare::Format determineFormat() override;
};

} // End of namespace Diff2

#endif

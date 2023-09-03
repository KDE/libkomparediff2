/*
SPDX-FileCopyrightText: 2002-2004 Otto Bruggeman <otto.bruggeman@home.nl>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_PERFORCEPARSER_H
#define KOMPAREDIFF2_PERFORCEPARSER_H

#include "parserbase.h"

namespace Diff2
{

class PerforceParser : public ParserBase
{
public:
    PerforceParser(const KompareModelList* list, const QStringList& diff);
    ~PerforceParser() override;

protected:
    bool parseContextDiffHeader() override;
    bool parseNormalDiffHeader() override;
    bool parseRCSDiffHeader() override;
    bool parseUnifiedDiffHeader() override;

protected:
    enum Kompare::Format determineFormat() override;
};

} // End of namespace Diff2

#endif

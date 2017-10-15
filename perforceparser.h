/**************************************************************************
**                             perforceparser.h
**                             ----------------
**      begin                   : Sun Sep  8 20:58:59 2002
**      Copyright 2002-2004 Otto Bruggeman <otto.bruggeman@home.nl>
***************************************************************************/
/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   ( at your option ) any later version.
**
***************************************************************************/

#ifndef PERFORCE_PARSER_H
#define PERFORCE_PARSER_H

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

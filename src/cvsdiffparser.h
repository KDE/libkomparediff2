/*
SPDX-FileCopyrightText: 2002-2004 Otto Bruggeman <otto.bruggeman@home.nl>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_CVSDIFFPARSER_H
#define KOMPAREDIFF2_CVSDIFFPARSER_H

#include "parserbase.h"

namespace KompareDiff2
{

class KompareModelList;

class CVSDiffParser : public ParserBase
{
public:
    CVSDiffParser(const KompareModelList* list, const QStringList& diff);
    ~CVSDiffParser() override;

protected:
    enum Format determineFormat() override;

protected:
//     virtual bool parseContextDiffHeader();
    bool parseEdDiffHeader() override;
    bool parseNormalDiffHeader() override;
    bool parseRCSDiffHeader() override;
//     virtual bool parseUnifiedDiffHeader();

//     virtual bool parseContextHunkHeader();
    bool parseEdHunkHeader() override;
//     virtual bool parseNormalHunkHeader();
    bool parseRCSHunkHeader() override;
//     virtual bool parseUnifiedHunkHeader();

//     virtual bool parseContextHunkBody();
    bool parseEdHunkBody() override;
//     virtual bool parseNormalHunkBody();
    bool parseRCSHunkBody() override;
//     virtual bool parseUnifiedHunkBody();
};

} // End of namespace KompareDiff2

#endif

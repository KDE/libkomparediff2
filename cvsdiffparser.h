/**************************************************************************
**                              cvsdiffparser.h
**                              ----------------
**      begin                   : Sun Aug  4 15:05:35 2002
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

#ifndef CVSDIFF_PARSER_H
#define CVSDIFF_PARSER_H

#include <QRegExp>

#include "parserbase.h"

namespace Diff2
{

class KompareModelList;

class CVSDiffParser : public ParserBase
{
public:
	CVSDiffParser( const KompareModelList* list, const QStringList& diff );
	virtual ~CVSDiffParser();

protected:
	enum Kompare::Format determineFormat() Q_DECL_OVERRIDE;

protected:
//	virtual bool parseContextDiffHeader();
	bool parseEdDiffHeader() Q_DECL_OVERRIDE;
	bool parseNormalDiffHeader() Q_DECL_OVERRIDE;
	bool parseRCSDiffHeader() Q_DECL_OVERRIDE;
//	virtual bool parseUnifiedDiffHeader();

//	virtual bool parseContextHunkHeader();
	bool parseEdHunkHeader() Q_DECL_OVERRIDE;
//	virtual bool parseNormalHunkHeader();
	bool parseRCSHunkHeader() Q_DECL_OVERRIDE;
//	virtual bool parseUnifiedHunkHeader();

//	virtual bool parseContextHunkBody();
	bool parseEdHunkBody() Q_DECL_OVERRIDE;
//	virtual bool parseNormalHunkBody();
	bool parseRCSHunkBody() Q_DECL_OVERRIDE;
//	virtual bool parseUnifiedHunkBody();
};

} // End of namespace Diff2

#endif

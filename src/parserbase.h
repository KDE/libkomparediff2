/*
SPDX-FileCopyrightText: 2002-2004 Otto Bruggeman <otto.bruggeman@home.nl>
SPDX-FileCopyrightText: 2010 Kevin Kofler <kevin.kofler@chello.at>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_PARSERBASE_H
#define KOMPAREDIFF2_PARSERBASE_H

#include <QRegularExpression>
#include <QStringList>

#include "kompare.h"
#include "difference.h"

class QString;

namespace KompareDiff2
{

class KompareModelList;
class DiffModelList;
class DiffModel;

class ParserBase
{
public:
    ParserBase(const KompareModelList* list, const QStringList& diff);
    virtual ~ParserBase();

    static QString escapePath(QString path);
    static QString unescapePath(QString path);

public:
    enum Kompare::Format format() { return determineFormat(); };
    DiffModelList* parse(bool* malformed = nullptr);

protected:
    virtual bool parseContextDiffHeader();
    virtual bool parseEdDiffHeader();
    virtual bool parseNormalDiffHeader();
    virtual bool parseRCSDiffHeader();
    virtual bool parseUnifiedDiffHeader();

    virtual bool parseContextHunkHeader();
    virtual bool parseEdHunkHeader();
    virtual bool parseNormalHunkHeader();
    virtual bool parseRCSHunkHeader();
    virtual bool parseUnifiedHunkHeader();

    virtual bool parseContextHunkBody();
    virtual bool parseEdHunkBody();
    virtual bool parseNormalHunkBody();
    virtual bool parseRCSHunkBody();
    virtual bool parseUnifiedHunkBody();

    virtual DiffModelList* parseContext();
    virtual DiffModelList* parseEd();
    virtual DiffModelList* parseNormal();
    virtual DiffModelList* parseRCS();
    virtual DiffModelList* parseUnified();

protected: // Helper methods to speed things up
    bool matchesUnifiedHunkLine(const QString& line) const;
    void checkHeader(const QRegularExpression& header);

protected:
    /** What is format of the diff */
    virtual enum Kompare::Format determineFormat();

protected:
    // Regexps for context parsing
    QRegularExpression m_contextDiffHeader1;
    QRegularExpression m_contextDiffHeader2;

    QRegularExpression m_contextHunkHeader1;
    QRegularExpression m_contextHunkHeader2;
    QRegularExpression m_contextHunkHeader3;
    QRegularExpressionMatch m_contextHunkHeader1Match;
    QRegularExpressionMatch m_contextHunkHeader2Match;

    QRegularExpression m_contextHunkBodyRemoved;
    QRegularExpression m_contextHunkBodyAdded;
    QRegularExpression m_contextHunkBodyChanged;
    QRegularExpression m_contextHunkBodyContext;
    QRegularExpression m_contextHunkBodyLine; // Added for convenience

    // Regexps for normal parsing
    QRegularExpression m_normalDiffHeader;

    QRegularExpression m_normalHunkHeaderAdded;
    QRegularExpression m_normalHunkHeaderRemoved;
    QRegularExpression m_normalHunkHeaderChanged;
    QRegularExpressionMatch m_normalHunkHeaderAddedMatch;
    QRegularExpressionMatch m_normalHunkHeaderRemovedMatch;
    QRegularExpressionMatch m_normalHunkHeaderChangedMatch;

    QRegularExpression m_normalHunkBodyRemoved;
    QRegularExpression m_normalHunkBodyAdded;
    QRegularExpression m_normalHunkBodyDivider;

    enum Difference::Type m_normalDiffType;

    // RegExps for rcs parsing
    QRegularExpression m_rcsDiffHeader;

    // Regexps for unified parsing
    QRegularExpression m_unifiedDiffHeader1;
    QRegularExpression m_unifiedDiffHeader2;

    QRegularExpression m_unifiedHunkHeader;
    QRegularExpressionMatch m_unifiedHunkHeaderMatch;

protected:
    const QStringList&         m_diffLines;
    DiffModel*                 m_currentModel;
    DiffModelList*             m_models;
    QStringList::ConstIterator m_diffIterator;

    bool                       m_singleFileDiff;
    bool                       m_malformed;

protected:
    const KompareModelList* m_list;
};

} // End of namespace KompareDiff2

#endif

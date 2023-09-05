/*
SPDX-FileCopyrightText: 2004-2005, 2009 Otto Bruggeman <bruggie@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_DIFFMODELLIST_H
#define KOMPAREDIFF2_DIFFMODELLIST_H

// lib
#include "komparediff2_export.h"
#include "diffmodel.h"
// Qt
#include <QList>

/**
 * KompareDiff2 namespace
 */
namespace KompareDiff2
{

using DiffModelListIterator =      QList<DiffModel*>::iterator;
using DiffModelListConstIterator = QList<DiffModel*>::const_iterator;

/**
 * @class DiffModelList diffmodellist.h <KompareDiff2/DiffModelList>
 *
 * A list of DiffModel.
 */
class KOMPAREDIFF2_EXPORT DiffModelList : public QList<DiffModel*>
{
public:
    DiffModelList() {}
    DiffModelList(const DiffModelList& list) : QList<DiffModel*>(list) {}
    virtual ~DiffModelList()
    {
        qDeleteAll(begin(), end());
    }

public:
    void sort();

}; // End of class DiffModelList

} // End of Namespace KompareDiff2

#endif // KOMPAREDIFF2_DIFFMODELLIST_H

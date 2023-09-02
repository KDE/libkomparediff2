/*
SPDX-FileCopyrightText: 2004-2005, 2009 Otto Bruggeman <bruggie@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_DIFFMODELLIST_H
#define KOMPAREDIFF2_DIFFMODELLIST_H

#include <QList> // include for the base class

#include "diffmodel.h"
#include "komparediff2_export.h"

/**
 * Diff2 namespace
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
    virtual void sort();

}; // End of class DiffModelList

} // End of Namespace Diff2

#endif // KOMPAREDIFF2_DIFFMODELLIST_H

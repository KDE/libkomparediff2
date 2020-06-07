/*
SPDX-FileCopyrightText: 2004-2005, 2009 Otto Bruggeman <bruggie@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DIFFMODELLIST_H
#define DIFFMODELLIST_H

#include <QList> // include for the base class

#include "diffmodel.h"
#include "diff2_export.h"

/**
 * Diff2 namespace
 */
namespace Diff2
{

typedef QList<DiffModel*>::Iterator DiffModelListIterator;
typedef QList<DiffModel*>::ConstIterator DiffModelListConstIterator;

/**
 * A list of DiffModel.
 */
class DIFF2_EXPORT DiffModelList : public QList<DiffModel*>
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

#endif // DIFFMODELLIST_H

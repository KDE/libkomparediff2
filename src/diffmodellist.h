/*
SPDX-FileCopyrightText: 2004-2005, 2009 Otto Bruggeman <bruggie@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DIFFMODELLIST_H
#define DIFFMODELLIST_H

#include <QList> // include for the base class

#include "diffmodel.h"
#include "komparediff2_export.h"
#if KOMPAREDIFF2_ENABLE_DEPRECATED_SINCE(5, 4)
#include "diff2_export_p.h"
#endif

/**
 * Diff2 namespace
 */
namespace Diff2
{

using DiffModelListIterator =      QList<DiffModel*>::iterator;
using DiffModelListConstIterator = QList<DiffModel*>::const_iterator;

/**
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

#endif // DIFFMODELLIST_H

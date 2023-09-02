/*
SPDX-FileCopyrightText: 2004 Otto Bruggeman <bruggie@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "diffmodellist.h"

using namespace KompareDiff2;

static bool diffModelCompare(DiffModel* model1, DiffModel* model2)
{
    return *model1 < *model2;
}

void DiffModelList::sort()
{
    std::sort(begin(), end(), diffModelCompare);
}

/*
    SPDX-FileCopyrightText: 2011 Dmitry Risenberg <dmitry.risenberg@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef INTERACTIVEDIFFTEST_H
#define INTERACTIVEDIFFTEST_H

// lib
#include "difference.h"
// Qt
#include <QObject>

class InteractiveDiffTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testOneLineChange();
    void testSameLine();
    void testLineNumbers_data();
    void testLineNumbers();
    void testDifferenceContents_data();
    void testDifferenceContents();
    void testAppliedTouch();
    void testAppliedIntersect();
    void testExistingAndApplied();
    void testOneLineDeletionUnapplied();
    void testApplyUnapply();
    void testContextDiff();
    void testNormalDiff();

private:
    void CompareDifferenceStringList(const KompareDiff2::DifferenceStringList &actual, const QStringList &expected);
};

#endif //  INTERACTIVEDIFFTEST_H

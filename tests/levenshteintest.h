/*
    SPDX-FileCopyrightText: 2011 Dmitry Risenberg <dmitry.risenberg@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LEVENSHTEINTEST_H
#define LEVENSHTEINTEST_H

// Qt
#include <QObject>

class LevenshteinTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testFirstEmptyString();
    void testSecondEmptyString();
    void testDifferenceStrings();
    void testStringLists();
    void testSmth();
};

#endif //  LEVENSHTEINTEST_H

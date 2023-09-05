/*
    SPDX-FileCopyrightText: 2011 Dmitry Risenberg <dmitry.risenberg@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "levenshteintest.h"

// lib
#include "difference.h"
#include "differencestringpair.h"
#include "levenshteintable.h"
#include "stringlistpair.h"
// Qt
#include <QTest>

using namespace KompareDiff2;

namespace QTest
{

template<>
char *toString(const Marker &marker)
{
    QByteArray result = "Marker(";
    if (marker.type() == Marker::Start) {
        result += "Start, ";
    } else {
        result += "End, ";
    }
    result += QByteArray::number(marker.offset());
    result += ")";
    return qstrdup(result.data());
}

}

void LevenshteinTest::testFirstEmptyString()
{
    DifferenceString *string1 = new DifferenceString(QString("12345"));
    DifferenceString *string2 = new DifferenceString(QString());
    DifferenceStringPair *pair = new DifferenceStringPair(string1, string2);
    LevenshteinTable<DifferenceStringPair> table;
    table.createTable(pair);
    table.createListsOfMarkers();
    MarkerList markersFirstExpected;
    markersFirstExpected << new Marker(Marker::Start, 0) << new Marker(Marker::End, 5);
    for (int i = 0; i < markersFirstExpected.size(); ++i) {
        QCOMPARE(*string1->markerList()[i], *markersFirstExpected[i]);
    }
    MarkerList markersSecondExpected;
    markersSecondExpected << new Marker(Marker::Start, 0) << new Marker(Marker::End, 0);
    for (int i = 0; i < markersSecondExpected.size(); ++i) {
        QCOMPARE(*string2->markerList()[i], *markersSecondExpected[i]);
    }
}

void LevenshteinTest::testSecondEmptyString()
{
    DifferenceString *string1 = new DifferenceString(QString());
    DifferenceString *string2 = new DifferenceString(QString("12345"));
    DifferenceStringPair *pair = new DifferenceStringPair(string1, string2);
    LevenshteinTable<DifferenceStringPair> table;
    table.createTable(pair);
    table.createListsOfMarkers();
    MarkerList markersFirstExpected;
    markersFirstExpected << new Marker(Marker::Start, 0) << new Marker(Marker::End, 0);
    for (int i = 0; i < markersFirstExpected.size(); ++i) {
        QCOMPARE(*string1->markerList()[i], *markersFirstExpected[i]);
    }
    MarkerList markersSecondExpected;
    markersSecondExpected << new Marker(Marker::Start, 0) << new Marker(Marker::End, 5);
    for (int i = 0; i < markersSecondExpected.size(); ++i) {
        QCOMPARE(*string2->markerList()[i], *markersSecondExpected[i]);
    }
}

void LevenshteinTest::testDifferenceStrings()
{
    DifferenceString *string1 = new DifferenceString(QString("aaabcddefghik"));
    DifferenceString *string2 = new DifferenceString(QString("aabcefghijk"));
    DifferenceStringPair *pair = new DifferenceStringPair(string1, string2);
    LevenshteinTable<DifferenceStringPair> table;
    table.createTable(pair);
    table.createListsOfMarkers();
    MarkerList markersFirstExpected;
    markersFirstExpected << new Marker(Marker::Start, 2) << new Marker(Marker::End, 3) << new Marker(Marker::Start, 5) << new Marker(Marker::End, 7);
    for (int i = 0; i < markersFirstExpected.size(); ++i) {
        QCOMPARE(*string1->markerList()[i], *markersFirstExpected[i]);
    }
    MarkerList markersSecondExpected;
    markersSecondExpected << new Marker(Marker::Start, 9) << new Marker(Marker::End, 10);
    for (int i = 0; i < markersSecondExpected.size(); ++i) {
        QCOMPARE(*string2->markerList()[i], *markersSecondExpected[i]);
    }
}

void LevenshteinTest::testStringLists()
{
    QStringList list1;
    list1 << "delete1"
          << "line1"
          << "line2"
          << "line3"
          << "delete2"
          << "delete3"
          << "line4";
    QStringList list2;
    list2 << "line1"
          << "line2"
          << "line3"
          << "insert1"
          << "line4"
          << "insert2";
    StringListPair *pair = new StringListPair(list1, list2);
    LevenshteinTable<StringListPair> table;
    table.createTable(pair);
    table.createListsOfMarkers();

    MarkerList markersFirstExpected;
    markersFirstExpected << new Marker(Marker::Start, 0) << new Marker(Marker::End, 1) << new Marker(Marker::Start, 4) << new Marker(Marker::End, 6);
    for (int i = 0; i < markersFirstExpected.size(); ++i) {
        QCOMPARE(*pair->markerListFirst()[i], *markersFirstExpected[i]);
    }
    MarkerList markersSecondExpected;
    markersSecondExpected << new Marker(Marker::Start, 3) << new Marker(Marker::End, 4) << new Marker(Marker::Start, 5) << new Marker(Marker::End, 6);
    for (int i = 0; i < markersSecondExpected.size(); ++i) {
        QCOMPARE(*pair->markerListSecond()[i], *markersSecondExpected[i]);
    }
}

void LevenshteinTest::testSmth()
{
    QStringList list1;
    list1 << "insert1\n"
          << "newline1\n"
          << "newline2\n"
          << "insert2\n";
    ;
    QStringList list2;
    list2 << "delete1\n"
          << "efgh\n"
          << "delete2\n";
    StringListPair *pair = new StringListPair(list1, list2);
    LevenshteinTable<StringListPair> table;
    table.createTable(pair);
    table.createListsOfMarkers();

    MarkerList markersFirstExpected;
    markersFirstExpected << new Marker(Marker::Start, 0) << new Marker(Marker::End, 4);
    for (int i = 0; i < markersFirstExpected.size(); ++i) {
        QCOMPARE(*pair->markerListFirst()[i], *markersFirstExpected[i]);
    }
    MarkerList markersSecondExpected;
    markersSecondExpected << new Marker(Marker::Start, 0) << new Marker(Marker::End, 3);
    for (int i = 0; i < markersSecondExpected.size(); ++i) {
        QCOMPARE(*pair->markerListSecond()[i], *markersSecondExpected[i]);
    }
}

QTEST_GUILESS_MAIN(LevenshteinTest);

#include "moc_levenshteintest.cpp"

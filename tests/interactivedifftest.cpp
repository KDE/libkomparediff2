        QFAIL(QString("Actual has too many items, starting with '%1', line %2").arg((*actualIter)->string()).arg(actualIter - actual.constBegin()).toLatin1());
        QFAIL(QString("Actual has too few items, no match for '%1', line %2").arg(*expectedIter).arg(expectedIter - expected.constBegin()).toLatin1());
static void
contextDiff1()
{
    QStringList patch;
    patch <<
    "commit 7377fcc682e85ef9784adb2a2da2c8c6756f9018 (HEAD, KDE/4.11)\n" <<
    "Author:     Dr. Chocholoušek <bla@zin.ec>\n" <<
    "AuthorDate: Sat Jan 25 17:30:01 2014 +0100\n" <<
    "\n" <<
    "    Fake diff.\n" <<
    "\n" <<
    "diff --git a/libdiff2/diffmodel.cpp b/libdiff2/diffmodel.cpp\n" <<
    "new file mode 100644\n" <<
    "index a42e82d..a8da0c9\n" <<
    "*** a/libdiff2/diffmodel.cpp\n" <<  // note the missing timestamps
    "--- b/libdiff2/diffmodel.cpp\n" <<
    "*************** DiffModel::DiffModel() :\n" <<
    "*** 58,64 ****\n" <<
    "  	m_sourceFile( "" ),\n" <<
    "  	m_destinationFile( "" ),\n" <<
    "  	m_sourceTimestamp( "" ),\n" <<
    "! 	m_destinationTimestamp( "" ),\n" <<
    "  	m_sourceRevision( "" ),\n" <<
    "  	m_destinationRevision( "" ),\n" <<
    "  	m_appliedCount( 0 ),\n" <<
    "--- 58,64 ----\n" <<
    "  	m_sourceFile( "" ),\n" <<
    "  	m_destinationFile( "" ),\n" <<
    "  	m_sourceTimestamp( "" ),\n" <<
    "! 	m_destinationTimestamp( \"doh\" ),\n" <<
    "  	m_sourceRevision( "" ),\n" <<
    "  	m_destinationRevision( "" ),\n" <<
    "  	m_appliedCount( 0 ),\n" <<
    "*************** void DiffModel::splitSourceInPathAndFile\n" <<
    "*** 84,89 ****\n" <<
    "--- 84,91 ----\n" <<
    "  	if( ( pos = m_source.lastIndexOf( \"/\" ) ) >= 0 )\n" <<
    "  		m_sourcePath = m_source.mid( 0, pos+1 );\n" <<
    "  \n" <<
    "+ 	add_this;\n" <<
    "+ \n" <<
    "  	if( ( pos = m_source.lastIndexOf( \"/\" ) ) >= 0 )\n" <<
    "  		m_sourceFile = m_source.mid( pos+1, m_source.length() - pos );\n" <<
    "  	else\n";
    Parser parser(0);
    DiffModelList* models = parser.parse(patch);
    QCOMPARE(models->size(), 1);
    DiffModel* model = models->at(0);
    QCOMPARE(model->differenceCount(), 2);
    model->applyAllDifferences(true);
    QVERIFY(model->differenceAt(0)->applied());
    QCOMPARE(model->differenceAt(0)->sourceLineNumber(), 61);
    QCOMPARE(model->differenceAt(0)->trackingDestinationLineNumber(), 61);
    QCOMPARE(model->differenceAt(1)->sourceLineNumber(), 87);
    QCOMPARE(model->differenceAt(1)->trackingDestinationLineNumber(), 87);
}

static void
contextDiff2()
{
    QStringList patch;
    patch <<
    "*** a/libdiff2/diffmodel.cpp\n" <<
    "--- b/libdiff2/diffmodel.cpp\n" <<
    "***************\n" <<
    "*** 55,60 **** DiffModel::DiffModel() :\n" << // note the  context here
    "--- 55,61 ----\n" <<
    "  	m_destination( "" ),\n" <<
    "  	m_sourcePath( "" ),\n" <<
    "  	m_destinationPath( "" ),\n" <<
    "+ 	m_hoh ( "" );\n" <<
    "  	m_sourceFile( "" ),\n" <<
    "  	m_destinationFile( "" ),\n" <<
    "  	m_sourceTimestamp( "" ),\n";

    Parser parser(0);
    DiffModelList* models = parser.parse(patch);
    QCOMPARE(models->size(), 1);
    DiffModel* model = models->at(0);
    QCOMPARE(model->differenceCount(), 1);
    model->applyAllDifferences(true);
    QVERIFY(model->differenceAt(0)->applied());
    QCOMPARE(model->differenceAt(0)->sourceLineNumber(), 58);
    QCOMPARE(model->differenceAt(0)->trackingDestinationLineNumber(), 58);
}

void InteractiveDiffTest::testContextDiff()
{
    contextDiff1();
    contextDiff2();
}

QTEST_GUILESS_MAIN(InteractiveDiffTest);
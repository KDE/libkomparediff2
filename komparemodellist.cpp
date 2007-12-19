/***************************************************************************
                          komparemodellist.cpp  -  description
                             -------------------
    begin                : Tue Jun 26 2001
    copyright            : (C) 2001-2004 Otto Bruggeman
                           (C) 2001-2003 John Firebaugh
                           (C) 2007      Kevin Kofler
    email                : jfirebaugh@kde.org
                           otto.bruggeman@home.nl
                           kevin.kofler@chello.at
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qfile.h>
#include <qdir.h>
#include <qregexp.h>
#include <qtextcodec.h>
#include <qvaluelist.h>

#include <kaction.h>
#include <kcharsets.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmimetype.h>
#include <ktempfile.h>

#include "difference.h"
#include "diffhunk.h"
#include "diffmodel.h"
#include "diffmodellist.h"
#include "kompareprocess.h"
#include "komparemodellist.h"
#include "parser.h"

#include "kompare_part.h"

using namespace Diff2;

KompareModelList::KompareModelList( DiffSettings* diffSettings, struct Kompare::Info& info, QObject* parent, const char* name )
	: QObject( parent, name ),
	m_diffProcess( 0 ),
	m_diffSettings( diffSettings ),
	m_models( 0 ),
	m_selectedModel( 0 ),
	m_selectedDifference( 0 ),
	m_noOfModified( 0 ),
	m_modelIndex( 0 ),
	m_info( info ),
	m_textCodec( 0 )
{
	m_applyDifference    = new KAction( i18n("&Apply Difference"), "1rightarrow", Qt::Key_Space,
	                                 this, SLOT(slotActionApplyDifference()),
	                                 (( KomparePart* )parent)->actionCollection(), "difference_apply" );
	m_unApplyDifference  = new KAction( i18n("Un&apply Difference"), "1leftarrow", Qt::Key_BackSpace,
	                                 this, SLOT(slotActionUnApplyDifference()),
	                                 (( KomparePart* )parent)->actionCollection(), "difference_unapply" );
	m_applyAll           = new KAction( i18n("App&ly All"), "2rightarrow", Qt::CTRL + Qt::Key_A,
	                                 this, SLOT(slotActionApplyAllDifferences()),
	                                 (( KomparePart* )parent)->actionCollection(), "difference_applyall" );
	m_unapplyAll         = new KAction( i18n("&Unapply All"), "2leftarrow", Qt::CTRL + Qt::Key_U,
	                                 this, SLOT(slotActionUnapplyAllDifferences()),
	                                 (( KomparePart* )parent)->actionCollection(), "difference_unapplyall" );
	m_previousFile       = new KAction( i18n("P&revious File"), "2uparrow", Qt::CTRL + Qt::Key_PageUp,
	                                 this, SLOT(slotPreviousModel()),
	                                 (( KomparePart* )parent)->actionCollection(), "difference_previousfile" );
	m_nextFile           = new KAction( i18n("N&ext File"), "2downarrow", Qt::CTRL + Qt::Key_PageDown,
	                                 this, SLOT(slotNextModel()),
	                                 (( KomparePart* )parent)->actionCollection(), "difference_nextfile" );
	m_previousDifference = new KAction( i18n("&Previous Difference"), "1uparrow", Qt::CTRL + Qt::Key_Up,
	                                 this, SLOT(slotPreviousDifference()),
	                                 (( KomparePart* )parent)->actionCollection(), "difference_previous" );
	m_nextDifference     = new KAction( i18n("&Next Difference"), "1downarrow", Qt::CTRL + Qt::Key_Down,
	                                 this, SLOT(slotNextDifference()),
	                                 (( KomparePart* )parent)->actionCollection(), "difference_next" );
	m_previousDifference->setEnabled( false );
	m_nextDifference->setEnabled( false );

	m_save = KStdAction::save( this, SLOT(slotSaveDestination()), ((KomparePart*)parent)->actionCollection() );
	m_save->setEnabled( false );

	updateModelListActions();
}

KompareModelList::~KompareModelList()
{
}

bool KompareModelList::isDirectory( const QString& url ) const
{
	QFileInfo fi( url );
	if ( fi.isDir() )
		return true;
	else
		return false;
}

bool KompareModelList::isDiff( const QString& mimeType ) const
{
	if ( mimeType == "text/x-diff" )
		return true;
	else
		return false;
}

bool KompareModelList::compare( const QString& source, const QString& destination )
{
	bool result = false;

	bool sourceIsDirectory = isDirectory( source );
	bool destinationIsDirectory = isDirectory( source );

	if ( sourceIsDirectory && destinationIsDirectory )
	{
		m_info.mode = Kompare::ComparingDirs;
		result = compareDirs( source, destination );
	}
	else if ( !sourceIsDirectory && !destinationIsDirectory )
	{
		QFile sourceFile( source );
		sourceFile.open( IO_ReadOnly );
		QString sourceMimeType = ( KMimeType::findByContent( sourceFile.readAll() ) )->name();
		sourceFile.close();
		kdDebug(8101) << "Mimetype source     : " << sourceMimeType << endl;

		QFile destinationFile( destination );
		destinationFile.open( IO_ReadOnly );
		QString destinationMimeType = ( KMimeType::findByContent( destinationFile.readAll() ) )->name();
		destinationFile.close();
		kdDebug(8101) << "Mimetype destination: " << destinationMimeType << endl;

		// Not checking if it is a text file/something diff can even compare, we'll let diff handle that
		if ( !isDiff( sourceMimeType ) && isDiff( destinationMimeType ) )
		{
			kdDebug(8101) << "Blending destination into source..." << endl;
			m_info.mode = Kompare::BlendingFile;
			result = openFileAndDiff( source, destination );
		}
		else if ( isDiff( sourceMimeType ) && !isDiff( destinationMimeType ) )
		{
			kdDebug(8101) << "Blending source into destination..." << endl;
			m_info.mode = Kompare::BlendingFile;
			result = openFileAndDiff( destination, source );
		}
		else
		{
			kdDebug(8101) << "Comparing source with destination" << endl;
			m_info.mode = Kompare::ComparingFiles;
			result = compareFiles( source, destination );
		}
	}
	else if ( sourceIsDirectory && !destinationIsDirectory )
	{
		m_info.mode = Kompare::BlendingDir;
		result = openDirAndDiff( source, destination );
	}
	else
	{
		m_info.mode = Kompare::BlendingDir;
		result = openDirAndDiff( destination, source );
	}

	return result;
}

bool KompareModelList::compareFiles( const QString& source, const QString& destination )
{
	m_source = source;
	m_destination = destination;

	clear(); // Destroy the old models...

//	m_fileWatch = new KDirWatch( this, "filewatch" );
//	m_fileWatch->addFile( m_source );
//	m_fileWatch->addFile( m_destination );

//	connect( m_fileWatch, SIGNAL( dirty( const QString& ) ), this, SLOT( slotFileChanged( const QString& ) ) );
//	connect( m_fileWatch, SIGNAL( created( const QString& ) ), this, SLOT( slotFileChanged( const QString& ) ) );
//	connect( m_fileWatch, SIGNAL( deleted( const QString& ) ), this, SLOT( slotFileChanged( const QString& ) ) );

//	m_fileWatch->startScan();
	m_diffProcess = new KompareProcess( m_diffSettings, Kompare::Custom, m_source, m_destination );
	m_diffProcess->setEncoding( m_encoding );

	connect( m_diffProcess, SIGNAL(diffHasFinished( bool )),
	         this, SLOT(slotDiffProcessFinished( bool )) );

	emit status( Kompare::RunningDiff );
	m_diffProcess->start();

	return true;
}

bool KompareModelList::compareDirs( const QString& source, const QString& destination )
{
	m_source = source;
	m_destination = destination;

	clear(); // Destroy the old models...

//	m_dirWatch = new KDirWatch( this, "dirwatch" );
	// Watch files in the dirs and watch the dirs recursively
//	m_dirWatch->addDir( m_source, true, true );
//	m_dirWatch->addDir( m_destination, true, true );

//	connect( m_dirWatch, SIGNAL( dirty  ( const QString& ) ), this, SLOT( slotDirectoryChanged( const QString& ) ) );
//	connect( m_dirWatch, SIGNAL( created( const QString& ) ), this, SLOT( slotDirectoryChanged( const QString& ) ) );
//	connect( m_dirWatch, SIGNAL( deleted( const QString& ) ), this, SLOT( slotDirectoryChanged( const QString& ) ) );

//	m_dirWatch->startScan();
	m_diffProcess = new KompareProcess( m_diffSettings, Kompare::Custom, m_source, m_destination );
	m_diffProcess->setEncoding( m_encoding );

	connect( m_diffProcess, SIGNAL(diffHasFinished( bool )),
	         this, SLOT(slotDiffProcessFinished( bool )) );

	emit status( Kompare::RunningDiff );
	m_diffProcess->start();

	return true;
}

bool KompareModelList::openFileAndDiff( const QString& file, const QString& diff )
{
	clear();

	if ( parseDiffOutput( readFile( diff ) ) != 0 )
	{
		emit error( i18n( "<qt>No models or no differences, this file: <b>%1</b>, is not a valid diff file.</qt>" ).arg(  diff ) );
		return false;
	}

	// Do our thing :)
	if ( !blendOriginalIntoModelList( file ) )
	{
		kdDebug(8101) << "Oops cant blend original file into modellist : " << file << endl;
		emit( i18n( "<qt>There were problems applying the diff <b>%1</b> to the file <b>%2</b>.</qt>" ).arg( diff ).arg( file ) );
		return false;
	}

	updateModelListActions();
	show();

	return true;
}

bool KompareModelList::openDirAndDiff( const QString& dir, const QString& diff )
{
	clear();

	if ( parseDiffOutput( readFile( diff ) ) != 0 )
	{
		emit error( i18n( "<qt>No models or no differences, this file: <b>%1</b>, is not a valid diff file.</qt>" ).arg( diff ) );
		return false;
	}

	// Do our thing :)
	if ( !blendOriginalIntoModelList( dir ) )
	{
		// Trouble blending the original into the model
		kdDebug(8101) << "Oops cant blend original dir into modellist : " << dir << endl;
		emit error( i18n( "<qt>There were problems applying the diff <b>%1</b> to the folder <b>%2</b>.</qt>" ).arg( diff ).arg( dir ) );
		return false;
	}

	updateModelListActions();
	show();

	return true;
}

void KompareModelList::slotSaveDestination()
{
	if ( m_selectedModel )
	{
		saveDestination( m_selectedModel );
	}
}

bool KompareModelList::saveDestination( DiffModel* model )
{
	kdDebug() << "KompareModelList::saveDestination: " << endl;

	if( !model->isModified() )
		return true;

	KTempFile* temp = new KTempFile();

	if( temp->status() != 0 ) {
		emit error( i18n( "Could not open a temporary file." ) );
		temp->unlink();
		delete temp;
		return false;
	}

	QTextStream* stream = temp->textStream();
	QStringList list;

	DiffHunkListConstIterator hunkIt = model->hunks()->begin();
	DiffHunkListConstIterator hEnd   = model->hunks()->end();

	for( ; hunkIt != hEnd; ++hunkIt )
	{
		DiffHunk* hunk = *hunkIt;

		DifferenceListConstIterator diffIt = hunk->differences().begin();
		DifferenceListConstIterator dEnd   = hunk->differences().end();

		Difference* diff;
		for( ; diffIt != dEnd; ++diffIt )
		{
			diff = *diffIt;
			if( !diff->applied() )
			{
				DifferenceStringListConstIterator stringIt = diff->destinationLines().begin();
				DifferenceStringListConstIterator sEnd     = diff->destinationLines().end();
				for ( ; stringIt != sEnd; ++stringIt )
				{
					list.append( ( *stringIt )->string() );
				}
			}
			else
			{
				DifferenceStringListConstIterator stringIt = diff->sourceLines().begin();
				DifferenceStringListConstIterator sEnd = diff->sourceLines().end();
				for ( ; stringIt != sEnd; ++stringIt )
				{
					list.append( ( *stringIt )->string() );
				}
			}
		}
	}

	// kdDebug( 8101 ) << "Everything: " << endl << list.join( "\n" ) << endl;

	if( list.count() > 0 )
		*stream << list.join( "" );

	temp->close();
	if( temp->status() != 0 ) {
		emit error( i18n( "<qt>Could not write to the temporary file <b>%1</b>, deleting it.</qt>" ).arg( temp->name() ) );
		temp->unlink();
		delete temp;
		return false;
	}

	bool result = false;

	if ( m_info.mode == Kompare::ComparingDirs )
	{
		QString destination = model->destinationPath() + model->destinationFile();
		kdDebug(8101) << "Tempfilename   : " << temp->name() << endl;
		kdDebug(8101) << "DestinationURL : " << destination << endl;
		KIO::UDSEntry entry;
		if ( !KIO::NetAccess::stat( KURL( destination ).path(), entry, (QWidget*)parent() ) )
		{
			if ( !KIO::NetAccess::mkdir( KURL( destination ).path(), (QWidget*)parent() ) )
			{
				emit error( i18n( "<qt>Could not create destination directory <b>%1</b>.\nThe file has not been saved.</qt>" ) );
				return false;
			}
		}
		result = KIO::NetAccess::upload( temp->name(), KURL( destination ), (QWidget*)parent() );
	}
	else
	{
		kdDebug(8101) << "Tempfilename   : " << temp->name() << endl;
		kdDebug(8101) << "DestinationURL : " << m_destination << endl;
		result = KIO::NetAccess::upload( temp->name(), KURL( m_destination ), (QWidget*)parent() );
	}

	if ( !result )
	{
		emit error( i18n( "<qt>Could not upload the temporary file to the destination location <b>%1</b>. The temporary file is still available under: <b>%2</b>. You can manually copy it to the right place.</qt>" ).arg( m_destination ).arg( temp->name() ) );
	}
	else
	{
		//model->slotSetModified( false );
		temp->unlink();
		delete temp;
	}

	return true;
}

bool KompareModelList::saveAll()
{
	if ( !m_models )
		return false;

	DiffModelListIterator it  =  m_models->begin();
	DiffModelListIterator end =  m_models->end();
	for ( ; it != end; ++it )
	{
		if( !saveDestination( *it ) )
			return false;
	}
	return true;
}

void KompareModelList::setEncoding( const QString& encoding )
{
	m_encoding = encoding;
	if ( encoding.lower() == "default" )
	{
		m_textCodec = QTextCodec::codecForLocale();
	}
	else
	{
		kdDebug() << "Encoding : " << encoding << endl;
		m_textCodec = KGlobal::charsets()->codecForName( encoding.latin1() );
		kdDebug() << "TextCodec: " << m_textCodec << endl;
		if ( !m_textCodec )
			m_textCodec = QTextCodec::codecForLocale();
	}
	kdDebug() << "TextCodec: " << m_textCodec << endl;
}

void KompareModelList::slotDiffProcessFinished( bool success )
{
	if ( success )
	{
		emit status( Kompare::Parsing );
		if ( parseDiffOutput( m_diffProcess->diffOutput() ) != 0 )
		{
			emit error( i18n( "Could not parse diff output." ) );
		}
		else
		{
			if ( m_info.mode != Kompare::ShowingDiff )
			{
				kdDebug() << "Blend this crap please and dont gimme any conflicts..." << endl;
				blendOriginalIntoModelList( m_info.localSource );
			}
			updateModelListActions();
			show();
		}
		emit status( Kompare::FinishedParsing );
	}
	else if ( m_diffProcess->exitStatus() == 0 )
	{
		emit error( i18n( "The files are identical." ) );
	}
	else
	{
		emit error( m_diffProcess->stdErr() );
	}

	delete m_diffProcess;
	m_diffProcess = 0;
}

void KompareModelList::slotDirectoryChanged( const QString& /*dir*/ )
{
	// some debug output to see if watching works properly
	kdDebug(8101) << "Yippie directories are being watched !!! :)" << endl;
	if ( m_diffProcess )
	{
		emit status( Kompare::ReRunningDiff );
		m_diffProcess->start();
	}
}

void KompareModelList::slotFileChanged( const QString& /*file*/ )
{
	// some debug output to see if watching works properly
	kdDebug(8101) << "Yippie files are being watched !!! :)" << endl;
	if ( m_diffProcess )
	{
		emit status( Kompare::ReRunningDiff );
		m_diffProcess->start();
	}
}

QStringList KompareModelList::split( const QString& fileContents )
{
	QString contents = fileContents;
	QStringList list;

	int pos = 0;
	unsigned int oldpos = 0;
	// split that does not strip the split char
#ifdef QT_OS_MAC
	const char split = '\r';
#else
	const char split = '\n';
#endif
	while ( ( pos = contents.find( split, oldpos ) ) >= 0 )
	{
		list.append( contents.mid( oldpos, pos - oldpos + 1 ) );
		oldpos = pos + 1;
	}

	if ( contents.length() > oldpos )
	{
		list.append( contents.right( contents.length() - oldpos ) );
	}

	return list;
}

QString KompareModelList::readFile( const QString& fileName )
{
	QStringList list;

	QFile file( fileName );
	file.open( IO_ReadOnly );

	QTextStream stream( &file );
	kdDebug() << "Codec = " << m_textCodec << endl;

	if (  !m_textCodec )
		m_textCodec = QTextCodec::codecForLocale();

	stream.setCodec(  m_textCodec );

	QString contents = stream.read();

	file.close();

	return contents;
}

bool KompareModelList::openDiff( const QString& diffFile )
{
	kdDebug(8101) << "Stupid :) Url = " << diffFile << endl;

	if ( diffFile.isEmpty() )
		return false;

	QString diff = readFile( diffFile );

	clear(); // Clear the current models

	emit status( Kompare::Parsing );

	if ( parseDiffOutput( diff ) != 0 )
	{
		emit error( i18n( "Could not parse diff output." ) );
		return false;
	}

	updateModelListActions();
	show();

	emit status( Kompare::FinishedParsing );

	return true;
}

QString KompareModelList::recreateDiff() const
{
	QString diff;

	DiffModelListConstIterator modelIt = m_models->begin();
	DiffModelListConstIterator mEnd    = m_models->end();

	for ( ; modelIt != mEnd; ++modelIt )
	{
		diff += (*modelIt)->recreateDiff();
	}
	return diff;
}

bool KompareModelList::saveDiff( const QString& url, QString directory, DiffSettings* diffSettings )
{
	kdDebug() << "KompareModelList::saveDiff: " << endl;

	m_diffTemp = new KTempFile();
	m_diffURL = url;

	if( m_diffTemp->status() != 0 ) {
		emit error( i18n( "Could not open a temporary file." ) );
		m_diffTemp->unlink();
		delete m_diffTemp;
		m_diffTemp = 0;
		return false;
	}

	m_diffProcess = new KompareProcess( diffSettings, Kompare::Custom, m_source, m_destination, directory );
	m_diffProcess->setEncoding( m_encoding );

	connect( m_diffProcess, SIGNAL(diffHasFinished( bool )),
	         this, SLOT(slotWriteDiffOutput( bool )) );

	emit status( Kompare::RunningDiff );
	return m_diffProcess->start();
}

void KompareModelList::slotWriteDiffOutput( bool success )
{
	kdDebug(8101) << "Success = " << success << endl;

	if( success )
	{
		QTextStream* stream = m_diffTemp->textStream();

		*stream << m_diffProcess->diffOutput();

		m_diffTemp->close();

		if( m_diffTemp->status() != 0 )
		{
			emit error( i18n( "Could not write to the temporary file." ) );
		}

		KIO::NetAccess::upload( m_diffTemp->name(), KURL( m_diffURL ), (QWidget*)parent() );

		emit status( Kompare::FinishedWritingDiff );
	}

	m_diffURL.truncate( 0 );
	m_diffTemp->unlink();

	delete m_diffTemp;
	m_diffTemp = 0;

	delete m_diffProcess;
	m_diffProcess = 0;
}

void KompareModelList::slotSelectionChanged( const Diff2::DiffModel* model, const Diff2::Difference* diff )
{
// This method will signal all the other objects about a change in selection,
// it will emit setSelection( const DiffModel*, const Difference* ) to all who are connected
	kdDebug(8101) << "KompareModelList::slotSelectionChanged( " << model << ", " << diff << " )" << endl;
	kdDebug(8101) << "Sender is : " << sender()->className() << endl;
//	kdDebug(8101) << kdBacktrace() << endl;

	m_selectedModel = const_cast<DiffModel*>(model);
	m_modelIndex = m_models->findIndex( m_selectedModel );
	kdDebug( 8101 ) << "m_modelIndex = " << m_modelIndex << endl;
	m_selectedDifference = const_cast<Difference*>(diff);

	m_selectedModel->setSelectedDifference( m_selectedDifference );

	// setSelected* search for the argument in the lists and return false if not found
	// if found they return true and set the m_selected*
	if ( !setSelectedModel( m_selectedModel ) )
	{
		// Backup plan
		m_selectedModel = firstModel();
		m_selectedDifference = m_selectedModel->firstDifference();
	}
	else if ( !m_selectedModel->setSelectedDifference( m_selectedDifference ) )
	{
		// Another backup plan
		m_selectedDifference = m_selectedModel->firstDifference();
	}

	emit setSelection( model, diff );
	emit setStatusBarModelInfo( findModel( m_selectedModel ), m_selectedModel->findDifference( m_selectedDifference ), modelCount(), differenceCount(), m_selectedModel->appliedCount() );

	updateModelListActions();
}

void KompareModelList::slotSelectionChanged( const Diff2::Difference* diff )
{
// This method will emit setSelection( const Difference* ) to whomever is listening
// when for instance in kompareview the selection has changed
	kdDebug(8101) << "KompareModelList::slotSelectionChanged( " << diff << " )" << endl;
	kdDebug(8101) << "Sender is : " << sender()->className() << endl;

	m_selectedDifference = const_cast<Difference*>(diff);

	if ( !m_selectedModel->setSelectedDifference( m_selectedDifference ) )
	{
		// Backup plan
		m_selectedDifference = m_selectedModel->firstDifference();
	}

	emit setSelection( diff );
	emit setStatusBarModelInfo( findModel( m_selectedModel ), m_selectedModel->findDifference( m_selectedDifference ), modelCount(), differenceCount(), m_selectedModel->appliedCount() );

	updateModelListActions();
}

void KompareModelList::slotPreviousModel()
{
	if ( ( m_selectedModel = prevModel() ) != 0 )
	{
		m_selectedDifference = m_selectedModel->firstDifference();
	}
	else
	{
		m_selectedModel = firstModel();
		m_selectedDifference = m_selectedModel->firstDifference();
	}

	emit setSelection( m_selectedModel, m_selectedDifference );
	emit setStatusBarModelInfo( findModel( m_selectedModel ), m_selectedModel->findDifference( m_selectedDifference ), modelCount(), differenceCount(), m_selectedModel->appliedCount() );
	updateModelListActions();
}

void KompareModelList::slotNextModel()
{
	if ( ( m_selectedModel = nextModel() ) != 0 )
	{
		m_selectedDifference = m_selectedModel->firstDifference();
	}
	else
	{
		m_selectedModel = lastModel();
		m_selectedDifference = m_selectedModel->firstDifference();
	}

	emit setSelection( m_selectedModel, m_selectedDifference );
	emit setStatusBarModelInfo( findModel( m_selectedModel ), m_selectedModel->findDifference( m_selectedDifference ), modelCount(), differenceCount(), m_selectedModel->appliedCount() );
	updateModelListActions();
}

DiffModel* KompareModelList::firstModel()
{
	kdDebug( 8101 ) << "KompareModelList::firstModel()" << endl;
	m_modelIndex = 0;
	kdDebug( 8101 ) << "m_modelIndex = " << m_modelIndex << endl;

	m_selectedModel = m_models->first();

	return m_selectedModel;
}

DiffModel* KompareModelList::lastModel()
{
	kdDebug( 8101 ) << "KompareModelList::lastModel()" << endl;
	m_modelIndex = m_models->count() - 1;
	kdDebug( 8101 ) << "m_modelIndex = " << m_modelIndex << endl;

	m_selectedModel = m_models->last();

	return m_selectedModel;
}

DiffModel* KompareModelList::prevModel()
{
	kdDebug( 8101 ) << "KompareModelList::prevModel()" << endl;
	if ( --m_modelIndex < m_models->count() )
	{
		kdDebug( 8101 ) << "m_modelIndex = " << m_modelIndex << endl;
		m_selectedModel = (*m_models)[ m_modelIndex ];
	}
	else
	{
		m_selectedModel = 0;
		m_modelIndex = 0;
		kdDebug( 8101 ) << "m_modelIndex = " << m_modelIndex << endl;
	}

	return m_selectedModel;
}

DiffModel* KompareModelList::nextModel()
{
	kdDebug( 8101 ) << "KompareModelList::nextModel()" << endl;
	if ( ++m_modelIndex < m_models->count() )
	{
		kdDebug( 8101 ) << "m_modelIndex = " << m_modelIndex << endl;
		m_selectedModel = (*m_models)[ m_modelIndex ];
	}
	else
	{
		m_selectedModel = 0;
		m_modelIndex = 0;
		kdDebug( 8101 ) << "m_modelIndex = " << m_modelIndex << endl;
	}

	return m_selectedModel;
}

void KompareModelList::slotPreviousDifference()
{
	kdDebug(8101) << "slotPreviousDifference called" << endl;
	if ( ( m_selectedDifference = m_selectedModel->prevDifference() ) != 0 )
	{
		emit setSelection( m_selectedDifference );
		emit setStatusBarModelInfo( findModel( m_selectedModel ), m_selectedModel->findDifference( m_selectedDifference ), modelCount(), differenceCount(), m_selectedModel->appliedCount() );
		updateModelListActions();
		return;
	}

	kdDebug(8101) << "**** no previous difference... ok lets find the previous model..." << endl;

	if ( ( m_selectedModel = prevModel() ) != 0 )
	{
		m_selectedDifference = m_selectedModel->lastDifference();

		emit setSelection( m_selectedModel, m_selectedDifference );
		emit setStatusBarModelInfo( findModel( m_selectedModel ), m_selectedModel->findDifference( m_selectedDifference ), modelCount(), differenceCount(), m_selectedModel->appliedCount() );
		updateModelListActions();
		return;
	}


	kdDebug(8101) << "**** !!! No previous model, ok backup plan activated..." << endl;

	// Backup plan
	m_selectedModel = firstModel();
	m_selectedDifference = m_selectedModel->firstDifference();

	emit setSelection( m_selectedModel, m_selectedDifference );
	emit setStatusBarModelInfo( findModel( m_selectedModel ), m_selectedModel->findDifference( m_selectedDifference ), modelCount(), differenceCount(), m_selectedModel->appliedCount() );
	updateModelListActions();
}

void KompareModelList::slotNextDifference()
{
	kdDebug(8101) << "slotNextDifference called" << endl;
	if ( ( m_selectedDifference = m_selectedModel->nextDifference() ) != 0 )
	{
		emit setSelection( m_selectedDifference );
		emit setStatusBarModelInfo( findModel( m_selectedModel ), m_selectedModel->findDifference( m_selectedDifference ), modelCount(), differenceCount(), m_selectedModel->appliedCount() );
		updateModelListActions();
		return;
	}

	kdDebug(8101) << "**** no next difference... ok lets find the next model..." << endl;

	if ( ( m_selectedModel = nextModel() ) != 0 )
	{
		m_selectedDifference = m_selectedModel->firstDifference();

		emit setSelection( m_selectedModel, m_selectedDifference );
		emit setStatusBarModelInfo( findModel( m_selectedModel ), m_selectedModel->findDifference( m_selectedDifference ), modelCount(), differenceCount(), m_selectedModel->appliedCount() );
		updateModelListActions();
		return;
	}

	kdDebug(8101) << "**** !!! No next model, ok backup plan activated..." << endl;

	// Backup plan
	m_selectedModel = lastModel();
	m_selectedDifference = m_selectedModel->lastDifference();

	emit setSelection( m_selectedModel, m_selectedDifference );
	emit setStatusBarModelInfo( findModel( m_selectedModel ), m_selectedModel->findDifference( m_selectedDifference ), modelCount(), differenceCount(), m_selectedModel->appliedCount() );
	updateModelListActions();
}

void KompareModelList::slotApplyDifference( bool apply )
{
	m_selectedModel->applyDifference( apply );
	emit applyDifference( apply );
}

void KompareModelList::slotApplyAllDifferences( bool apply )
{
	m_selectedModel->applyAllDifferences( apply );
	emit applyAllDifferences( apply );
}

int KompareModelList::parseDiffOutput( const QString& diff )
{
	kdDebug(8101) << "KompareModelList::parseDiffOutput" << endl;

	QStringList diffLines = split( diff );

	Parser* parser = new Parser( this );
	m_models = parser->parse( diffLines );

	m_info.generator = parser->generator();
	m_info.format    = parser->format();

	delete parser;

	if ( m_models )
	{
		m_selectedModel = firstModel();
		kdDebug(8101) << "Ok there are differences..." << endl;
		m_selectedDifference = m_selectedModel->firstDifference();
		emit setStatusBarModelInfo( 0, 0, modelCount(), differenceCount(), 0);
	}
	else
	{
		// Wow trouble, no models, so no differences...
		kdDebug(8101) << "Now i'll be damned, there should be models here !!!" << endl;
		return -1;
	}

	return 0;
}

bool KompareModelList::blendOriginalIntoModelList( const QString& localURL )
{
	kdDebug() << "Hurrah we are blending..." << endl;
	QFileInfo fi( localURL );

	bool result = false;
	DiffModel* model;

	QString fileContents;

	if ( fi.isDir() )
	{ // is a dir
		kdDebug() << "Blend Dir" << endl;
//		QDir dir( localURL, QString::null, QDir::Name|QDir::DirsFirst, QDir::All );
		DiffModelListIterator modelIt = m_models->begin();
		DiffModelListIterator mEnd    = m_models->end();
		for ( ; modelIt != mEnd; ++modelIt )
		{
			model = *modelIt;
			kdDebug(8101) << "Model : " << model << endl;
			QString filename = model->sourcePath() + model->sourceFile();
			if ( !filename.startsWith( localURL ) )
				filename.prepend( localURL );
			QFileInfo fi2( filename );
			if ( fi2.exists() )
			{
				kdDebug(8101) << "Reading from: " << filename << endl;
				fileContents = readFile( filename );
				result = blendFile( model, fileContents );
			}
			else
			{
				kdDebug(8101) << "File " << filename << " does not exist !" << endl;
				kdDebug(8101) << "Assume empty file !" << endl;
				fileContents.truncate( 0 );
				result = blendFile( model, fileContents );
			}
		}
		kdDebug() << "End of Blend Dir" << endl;
	}
	else if ( fi.isFile() )
	{ // is a file
		kdDebug() << "Blend File" << endl;
		kdDebug(8101) << "Reading from: " << localURL << endl;
		fileContents = readFile( localURL );

		result = blendFile( (*m_models)[ 0 ], fileContents );
		kdDebug() << "End of Blend File" << endl;
	}

	return result;
}

bool KompareModelList::blendFile( DiffModel* model, const QString& fileContents )
{
	if ( !model )
	{
		kdDebug() << "**** model is null :(" << endl;
		return false;
	}

	model->setBlended( true );

	int srcLineNo = 1, destLineNo = 1;

	QStringList lines = split( fileContents );

	QStringList::ConstIterator linesIt = lines.begin();
	QStringList::ConstIterator lEnd    = lines.end();

	DiffHunkList* hunks = model->hunks();
	kdDebug(8101) << "Hunks in hunklist: " << hunks->count() << endl;
	DiffHunkListIterator hunkIt = hunks->begin();

	DiffHunk*   newHunk = 0;
	Difference* newDiff = 0;

	// FIXME: this approach is not very good, we should first check if the hunk applies cleanly
	// and without offset and if not use that new linenumber with offset to compare against
	// This will only work for files we just diffed with kompare but not for blending where
	// file(s) to patch has/have potentially changed

	for ( ; hunkIt != hunks->end(); ++hunkIt )
	{
		// Do we need to insert a new hunk before this one ?
		DiffHunk* hunk = *hunkIt;
		if ( srcLineNo < hunk->sourceLineNumber() )
		{
			newHunk = new DiffHunk( srcLineNo, destLineNo, "", DiffHunk::AddedByBlend );

			hunks->insert( hunkIt, newHunk );

			newDiff = new Difference( srcLineNo, destLineNo,
			              Difference::Unchanged );

			newHunk->add( newDiff );

			while ( srcLineNo < hunk->sourceLineNumber() && linesIt != lEnd )
			{
				newDiff->addSourceLine( *linesIt );
				newDiff->addDestinationLine( *linesIt );
				srcLineNo++;
				destLineNo++;
				++linesIt;
			}
		}

		// Now we add the linecount difference for the hunk that follows
		int size = hunk->sourceLineCount();

		for ( int i = 0; i < size; ++i )
		{
			++linesIt;
		}

		srcLineNo += size;
		destLineNo += (*hunkIt)->destinationLineCount();
	}

	if ( linesIt != lEnd )
	{
		newHunk = new DiffHunk( srcLineNo, destLineNo, "", DiffHunk::AddedByBlend );

		model->addHunk( newHunk );

		newDiff = new Difference( srcLineNo, destLineNo, Difference::Unchanged );

		newHunk->add( newDiff );

		while ( linesIt != lEnd )
		{
			newDiff->addSourceLine( *linesIt );
			newDiff->addDestinationLine( *linesIt );
			++linesIt;
		}
	}
#if 0
		DifferenceList hunkDiffList   = (*hunkIt)->differences();
		DifferenceListIterator diffIt = hunkDiffList.begin();
		DifferenceListIterator dEnd   = hunkDiffList.end();
		kdDebug() << "Number of differences in hunkDiffList = " << diffList.count() << endl;

		DifferenceListIterator tempIt;
		Difference* diff;

		for ( ; diffIt != dEnd; ++diffIt )
		{
			diff = *diffIt;
			kdDebug() << "*(Diff it) = " << diff << endl;
			// Check if there are lines in the original file before the difference
			// that are not yet in the diff. If so create new Unchanged diff
			if ( srcLineNo < diff->sourceLineNumber() )
			{
				newDiff = new Difference( srcLineNo, destLineNo,
				              Difference::Unchanged | Difference::AddedByBlend );
				newHunk->add( newDiff );
				while ( srcLineNo < diff->sourceLineNumber() && linesIt != lEnd )
				{
//					kdDebug(8101) << "SourceLine = " << srcLineNo << ": " << *linesIt << endl;
					newDiff->addSourceLine( *linesIt );
					newDiff->addDestinationLine( *linesIt );
					srcLineNo++;
					destLineNo++;
					++linesIt;
				}
			}
			// Now i've got to add that diff
			switch ( diff->type() )
			{
			case Difference::Unchanged:
				kdDebug(8101) << "Unchanged" << endl;
				for ( int i = 0; i < diff->sourceLineCount(); i++ )
				{
					if ( linesIt != lEnd && *linesIt != diff->sourceLineAt( i )->string() )
					{
						kdDebug(8101) << "Conflict: SourceLine = " << srcLineNo << ": " << *linesIt << endl;
						kdDebug(8101) << "Conflict: DiffLine   = " << diff->sourceLineNumber() + i << ": " << diff->sourceLineAt( i )->string() << endl;

						// Do conflict resolution (well sort of)
						diff->sourceLineAt( i )->setConflictString( *linesIt );
						diff->setConflict( true );
					}
//					kdDebug(8101) << "SourceLine = " << srcLineNo << ": " << *linesIt << endl;
//					kdDebug(8101) << "DiffLine   = " << diff->sourceLineNumber() + i << ": " << diff->sourceLineAt( i )->string() << endl;
					srcLineNo++;
					destLineNo++;
					++linesIt;
				}

				tempIt = diffIt;
				--diffIt;
				diffList.remove( tempIt );
				newHunk->add( diff );

				break;
			case Difference::Change:
				kdDebug(8101) << "Change" << endl;

				//QStringListConstIterator saveIt = linesIt;

				for ( int i = 0; i < diff->sourceLineCount(); i++ )
				{
					if ( linesIt != lEnd && *linesIt != diff->sourceLineAt( i )->string() )
					{
						kdDebug(8101) << "Conflict: SourceLine = " << srcLineNo << ": " << *linesIt << endl;
						kdDebug(8101) << "Conflict: DiffLine   = " << diff->sourceLineNumber() + i << ": " << diff->sourceLineAt( i )->string() << endl;

						// Do conflict resolution (well sort of)
						diff->sourceLineAt( i )->setConflictString( *linesIt );
						diff->setConflict( true );
					}
					srcLineNo++;
					destLineNo++;
					++linesIt;
				}

				destLineNo += diff->destinationLineCount();

				tempIt = diffIt;
				--diffIt;
				diffList.remove( tempIt );
				newHunk->add( diff );
				newModel->addDiff( diff );

				break;
			case Difference::Insert:
				kdDebug(8101) << "Insert" << endl;
				destLineNo += diff->destinationLineCount();
				tempIt = diffIt;
				--diffIt;
				diffList.remove( tempIt );
				newHunk->add( diff );
				newModel->addDiff( diff );
				break;
			case Difference::Delete:
				kdDebug(8101) << "Delete" << endl;
				kdDebug(8101) << "Number of lines in Delete: " << diff->sourceLineCount() << endl;
				for ( int i = 0; i < diff->sourceLineCount(); i++ )
				{
					if ( linesIt != lEnd && *linesIt != diff->sourceLineAt( i )->string() )
					{
						kdDebug(8101) << "Conflict: SourceLine = " << srcLineNo << ": " << *linesIt << endl;
						kdDebug(8101) << "Conflict: DiffLine   = " << diff->sourceLineNumber() + i << ": " << diff->sourceLineAt( i )->string() << endl;

						// Do conflict resolution (well sort of)
						diff->sourceLineAt( i )->setConflictString( *linesIt );
						diff->setConflict( true );
					}

//					kdDebug(8101) << "SourceLine = " << srcLineNo << ": " << *it << endl;
//					kdDebug(8101) << "DiffLine   = " << diff->sourceLineNumber() + i << ": " << diff->sourceLineAt( i )->string() << endl;
					srcLineNo++;
					++linesIt;
				}

				tempIt = diffIt;
				--diffIt;
				diffList.remove( tempIt );
				newHunk->add( diff );
				newModel->addDiff( diff );
				break;
			default:
				kdDebug(8101) << "****, some diff type we dont know about ???" << endl;
			}
		}
	}
#endif

/*
	diffList = newModel->differences();

	diff = diffList.first();
	kdDebug(8101) << "Count = " << diffList.count() << endl;
	for ( diff = diffList.first(); diff; diff = diffList.next() )
	{
		kdDebug(8101) << "sourcelinenumber = " << diff->sourceLineNumber() << endl;
	}
*/

	m_selectedModel = firstModel();

	m_selectedDifference = m_selectedModel->firstDifference();

	return true;
}

void KompareModelList::show()
{
	kdDebug() << "KompareModelList::Show Number of models = " << m_models->count() << endl;
	emit modelsChanged( m_models );
	emit setSelection( m_selectedModel, m_selectedDifference );
}

void KompareModelList::clear()
{
	if ( m_models )
		m_models->clear();

	emit modelsChanged( m_models );
}

void KompareModelList::swap()
{
	QString source = m_source;
	QString destination = m_destination;
	if ( m_info.mode == Kompare::ComparingFiles )
		compareFiles( destination, source );
	else if ( m_info.mode == Kompare::ComparingDirs )
		compareDirs( destination, source );
}

bool KompareModelList::isModified() const
{
	if ( m_noOfModified > 0 )
		return true;
	return false;
}

int KompareModelList::modelCount() const
{
	return m_models ? m_models->count() : 0;
}

int KompareModelList::differenceCount() const
{
	return m_selectedModel ? m_selectedModel->differenceCount() : -1;
}

int KompareModelList::appliedCount() const
{
	return m_selectedModel ? m_selectedModel->appliedCount() : -1;
}

void KompareModelList::slotSetModified( bool modified )
{
	kdDebug(8101) << "KompareModelList::slotSetModified( " << modified << " );" << endl;
	kdDebug(8101) << "Before: m_noOfModified = " << m_noOfModified << endl;

	// If selectedModel emits its signal setModified it does not set the model
	// internal m_modified bool yet, it only does that after the emit.
	if ( modified && !m_selectedModel->isModified() )
		m_noOfModified++;
	else if ( !modified && m_selectedModel->isModified() )
		m_noOfModified--;

	kdDebug(8101) << "After : m_noOfModified = " << m_noOfModified << endl;

	if ( m_noOfModified < 0 )
	{
		kdDebug(8101) << "Wow something is ****ed up..." << endl;
	}
	else if ( m_noOfModified == 0 )
	{
		emit setModified( false );
	}
	else // > 0 :-)
	{
		emit setModified( true );
	}
}

bool KompareModelList::setSelectedModel( DiffModel* model )
{
	kdDebug(8101) << "KompareModelList::setSelectedModel( " << model << " )" << endl;

	if ( model != m_selectedModel )
	{
		if ( m_models->findIndex( model ) == -1 )
			return false;
		kdDebug(8101) << "m_selectedModel (was) = " << m_selectedModel << endl;
		m_modelIndex = m_models->findIndex( model );
		kdDebug(8101) << "m_selectedModel (is)  = " << m_selectedModel << endl;
		m_selectedModel = model;
	}

	updateModelListActions();

	return true;
}

void KompareModelList::updateModelListActions()
{
	if ( m_models && m_selectedModel && m_selectedDifference )
	{
		if ( ( ( KomparePart* )parent() )->isReadWrite() )
		{
			if ( m_selectedModel->appliedCount() != m_selectedModel->differenceCount() )
				m_applyAll->setEnabled( true );
			else
				m_applyAll->setEnabled( false );

			if ( m_selectedModel->appliedCount() != 0 )
				m_unapplyAll->setEnabled( true );
			else
				m_unapplyAll->setEnabled( false );

			m_applyDifference->setEnabled( true );
			m_unApplyDifference->setEnabled( true );
			m_save->setEnabled( m_selectedModel->isModified() );
		}
		else
		{
			m_applyDifference->setEnabled  ( false );
			m_unApplyDifference->setEnabled( false );
			m_applyAll->setEnabled         ( false );
			m_unapplyAll->setEnabled       ( false );
			m_save->setEnabled             ( false );
		}

		m_previousFile->setEnabled      ( hasPrevModel() );
		m_nextFile->setEnabled          ( hasNextModel() );
		m_previousDifference->setEnabled( hasPrevDiff() );
		m_nextDifference->setEnabled    ( hasNextDiff() );
	}
	else
	{
		m_applyDifference->setEnabled   ( false );
		m_unApplyDifference->setEnabled ( false );
		m_applyAll->setEnabled          ( false );
		m_unapplyAll->setEnabled        ( false );

		m_previousFile->setEnabled      ( false );
		m_nextFile->setEnabled          ( false );
		m_previousDifference->setEnabled( false );
		m_nextDifference->setEnabled    ( false );
		m_save->setEnabled              ( false );
	}
}

bool KompareModelList::hasPrevModel() const
{
	kdDebug(8101) << "KompareModelList::hasPrevModel()" << endl;

	if (  m_modelIndex > 0 )
	{
//		kdDebug(8101) << "has prev model" << endl;
		return true;
	}

//	kdDebug(8101) << "doesn't have a prev model, this is the first one..." << endl;

	return false;
}

bool KompareModelList::hasNextModel() const
{
	kdDebug(8101) << "KompareModelList::hasNextModel()" << endl;

	if ( (  unsigned int )m_modelIndex < (  m_models->count() - 1 ) )
	{
//		kdDebug(8101) << "has next model" << endl;
		return true;
	}

//	kdDebug(8101) << "doesn't have a next model, this is the last one..." << endl;
	return false;
}

bool KompareModelList::hasPrevDiff() const
{
//	kdDebug(8101) << "KompareModelList::hasPrevDiff()" << endl;
	int index = m_selectedModel->diffIndex();

	if ( index > 0 )
	{
//		kdDebug(8101) << "has prev difference in same model" << endl;
		return true;
	}

	if ( hasPrevModel() )
	{
//		kdDebug(8101) << "has prev difference but in prev model" << endl;
		return true;
	}

//	kdDebug(8101) << "doesn't have a prev difference, not even in the previous model because there is no previous model" << endl;

	return false;
}

bool KompareModelList::hasNextDiff() const
{
//	kdDebug(8101) << "KompareModelList::hasNextDiff()" << endl;
	int index = m_selectedModel->diffIndex();

	if ( index < ( m_selectedModel->differenceCount() - 1 ) )
	{
//		kdDebug(8101) << "has next difference in same model" << endl;
		return true;
	}

	if ( hasNextModel() )
	{
//		kdDebug(8101) << "has next difference but in next model" << endl;
		return true;
	}

//	kdDebug(8101) << "doesn't have a next difference, not even in next model because there is no next model" << endl;

	return false;
}

void KompareModelList::slotActionApplyDifference()
{
	if ( !m_selectedDifference->applied() )
		slotApplyDifference( true );
	slotNextDifference();
	updateModelListActions();
}

void KompareModelList::slotActionUnApplyDifference()
{
	if ( m_selectedDifference->applied() )
		slotApplyDifference( false );
	slotPreviousDifference();
	updateModelListActions();
}

void KompareModelList::slotActionApplyAllDifferences()
{
	slotApplyAllDifferences( true );
	updateModelListActions();
}

void KompareModelList::slotActionUnapplyAllDifferences()
{
	slotApplyAllDifferences( false );
	updateModelListActions();
}

#include "komparemodellist.moc"

/* vim: set ts=4 sw=4 noet: */

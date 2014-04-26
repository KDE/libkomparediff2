/***************************************************************************
                          komparemodellist.cpp
                          --------------------
    begin                : Tue Jun 26 2001
    Copyright 2001-2005,2009 Otto Bruggeman <otto.bruggeman@home.nl>
    Copyright 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    Copyright 2007-2010 Kevin Kofler   <kevin.kofler@chello.at>
    Copyright 2012      Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "komparemodellist.h"

#include <QAction>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QRegExp>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtCore/QLinkedList>
#include <QtDebug>

#include <kactioncollection.h>
#include <kcharsets.h>
#include <kdirwatch.h>
#include <kio/netaccess.h>
#include <klocalizedstring.h>
#include <kmimetype.h>
#include <ktemporaryfile.h>
#include <kstandardaction.h>

#include "difference.h"
#include "diffhunk.h"
#include "diffmodel.h"
#include "diffmodellist.h"
#include "kompareprocess.h"
#include "parser.h"
#include <kglobal.h>

static const KCatalogLoader loader("libkomparediff2");

using namespace Diff2;

KompareModelList::KompareModelList( DiffSettings* diffSettings, QWidget* widgetForKIO, QObject* parent, const char* name, bool isReadWrite)
	: QObject( parent ),
	m_diffProcess( 0 ),
	m_diffSettings( diffSettings ),
	m_models( 0 ),
	m_selectedModel( 0 ),
	m_selectedDifference( 0 ),
	m_modelIndex( 0 ),
	m_info( 0 ),
	m_textCodec( 0 ),
	m_widgetForKIO( widgetForKIO ),
	m_isReadWrite( isReadWrite )
{
	qDebug() << "Show me the arguments: " << diffSettings << ", " << widgetForKIO << ", " << parent << ", " << name << endl;
	m_actionCollection = new KActionCollection(this);
	m_applyDifference = m_actionCollection->addAction( "difference_apply", this, SLOT(slotActionApplyDifference()) );
	m_applyDifference->setIcon( QIcon::fromTheme("arrow-right") );
	m_applyDifference->setText( i18n("&Apply Difference") );
	m_applyDifference->setShortcut( QKeySequence(Qt::Key_Space) );
	m_unApplyDifference = m_actionCollection->addAction( "difference_unapply", this, SLOT(slotActionUnApplyDifference()) );
	m_unApplyDifference->setIcon( QIcon::fromTheme("arrow-left") );
	m_unApplyDifference->setText( i18n("Un&apply Difference") );
	m_unApplyDifference->setShortcut( QKeySequence(Qt::Key_Backspace) );
	m_applyAll = m_actionCollection->addAction( "difference_applyall", this, SLOT(slotActionApplyAllDifferences()) );
	m_applyAll->setIcon( QIcon::fromTheme("arrow-right-double") );
	m_applyAll->setText( i18n("App&ly All") );
	m_applyAll->setShortcut( QKeySequence(Qt::CTRL + Qt::Key_A) );
	m_unapplyAll = m_actionCollection->addAction( "difference_unapplyall", this, SLOT(slotActionUnapplyAllDifferences()) );
	m_unapplyAll->setIcon( QIcon::fromTheme("arrow-left-double") );
	m_unapplyAll->setText( i18n("&Unapply All") );
	m_unapplyAll->setShortcut( QKeySequence(Qt::CTRL + Qt::Key_U) );
	m_previousFile = m_actionCollection->addAction( "difference_previousfile", this, SLOT(slotPreviousModel()) );
	m_previousFile->setIcon( QIcon::fromTheme("arrow-up-double") );
	m_previousFile->setText( i18n("P&revious File") );
	m_previousFile->setShortcut( QKeySequence(Qt::CTRL + Qt::Key_PageUp) );
	m_nextFile = m_actionCollection->addAction( "difference_nextfile", this, SLOT(slotNextModel()) );
	m_nextFile->setIcon( QIcon::fromTheme("arrow-down-double") );
	m_nextFile->setText( i18n("N&ext File") );
	m_nextFile->setShortcut( QKeySequence(Qt::CTRL + Qt::Key_PageDown) );
	m_previousDifference = m_actionCollection->addAction( "difference_previous", this, SLOT(slotPreviousDifference()) );
	m_previousDifference->setIcon( QIcon::fromTheme("arrow-up") );
	m_previousDifference->setText( i18n("&Previous Difference") );
	m_previousDifference->setShortcut( QKeySequence(Qt::CTRL + Qt::Key_Up) );
	m_nextDifference = m_actionCollection->addAction( "difference_next", this, SLOT(slotNextDifference()) );
	m_nextDifference->setIcon( QIcon::fromTheme("arrow-down") );
	m_nextDifference->setText( i18n("&Next Difference") );
	m_nextDifference->setShortcut( QKeySequence(Qt::CTRL + Qt::Key_Down) );
	m_previousDifference->setEnabled( false );
	m_nextDifference->setEnabled( false );

	m_save = KStandardAction::save( this, SLOT(slotSaveDestination()), m_actionCollection );
	m_save->setEnabled( false );

	updateModelListActions();
}

KompareModelList::~KompareModelList()
{
	m_selectedModel = 0;
	m_selectedDifference = 0;
	m_info = 0;
	delete m_models;
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
	if ( mimeType == "text/x-patch" )
		return true;
	else
		return false;
}

bool KompareModelList::compare()
{
	bool result = false;

	bool sourceIsDirectory = isDirectory( m_info->localSource );
	bool destinationIsDirectory = isDirectory( m_info->localDestination );

	if ( sourceIsDirectory && destinationIsDirectory )
	{
		m_info->mode = Kompare::ComparingDirs;
		result = compare(m_info->mode);
	}
	else if ( !sourceIsDirectory && !destinationIsDirectory )
	{
		QFile sourceFile( m_info->localSource );
		sourceFile.open( QIODevice::ReadOnly );
		QString sourceMimeType = ( KMimeType::findByContent( sourceFile.readAll() ) )->name();
		sourceFile.close();
		qDebug() << "Mimetype source     : " << sourceMimeType << endl;

		QFile destinationFile( m_info->localDestination );
		destinationFile.open( QIODevice::ReadOnly );
		QString destinationMimeType = ( KMimeType::findByContent( destinationFile.readAll() ) )->name();
		destinationFile.close();
		qDebug() << "Mimetype destination: " << destinationMimeType << endl;

		// Not checking if it is a text file/something diff can even compare, we'll let diff handle that
		if ( !isDiff( sourceMimeType ) && isDiff( destinationMimeType ) )
		{
			qDebug() << "Blending destination into source..." << endl;
			m_info->mode = Kompare::BlendingFile;
			result = openFileAndDiff();
		}
		else if ( isDiff( sourceMimeType ) && !isDiff( destinationMimeType ) )
		{
			qDebug() << "Blending source into destination..." << endl;
			m_info->mode = Kompare::BlendingFile;
			// Swap source and destination before calling this
			m_info->swapSourceWithDestination();
			// Do we need to notify anyone we swapped source and destination?
			// No we do not need to notify anyone about swapping source with destination
			result = openFileAndDiff();
		}
		else
		{
			qDebug() << "Comparing source with destination" << endl;
			m_info->mode = Kompare::ComparingFiles;
			result = compare(m_info->mode);
		}
	}
	else if ( sourceIsDirectory && !destinationIsDirectory )
	{
		m_info->mode = Kompare::BlendingDir;
		result = openDirAndDiff();
	}
	else
	{
		m_info->mode = Kompare::BlendingDir;
		// Swap source and destination first in m_info
		m_info->swapSourceWithDestination();
		// Do we need to notify anyone we swapped source and destination?
		// No we do not need to notify anyone about swapping source with destination
		result = openDirAndDiff();
	}

	return result;
}

bool KompareModelList::compare(Kompare::Mode mode)
{
	clear(); // Destroy the old models...

	m_diffProcess = new KompareProcess( m_diffSettings, Kompare::Custom, m_info->localSource, m_info->localDestination, QString(), mode );
	m_diffProcess->setEncoding( m_encoding );

	connect( m_diffProcess, SIGNAL(diffHasFinished( bool )),
	         this, SLOT(slotDiffProcessFinished( bool )) );

	emit status( Kompare::RunningDiff );
	m_diffProcess->start();

	return true;
}

QString lstripSeparators( const QString & from, uint count )
{
	int position = 0;
	for ( uint i = 0; i < count; ++i )
	{
		position = from.indexOf('/', position);
		if ( position == -1 )
		{
			break;
		}
	}
	if ( position == -1 )
	{
		return "";
	}
	else
	{
		return from.mid(position);
	}
}

void KompareModelList::setDepthAndApplied()
{
	// Splice to avoid calling ~DiffModelList
	QList<Diff2::DiffModel*> splicedModelList(*m_models);
	foreach(DiffModel* model, splicedModelList)
	{
		 model->setSourceFile( lstripSeparators(model->source(), m_info->depth) );
		 model->setDestinationFile( lstripSeparators(model->destination(), m_info->depth) );
		 model->applyAllDifferences(m_info->applied);
	}
}

bool KompareModelList::openFileAndDiff()
{
	clear();

	if ( parseDiffOutput( readFile( m_info->localDestination ) ) != 0 )
	{
		emit error( i18n( "<qt>No models or no differences, this file: <b>%1</b>, is not a valid diff file.</qt>", m_info->destination.url() ) );
		return false;
	}

	setDepthAndApplied();

	if ( !blendOriginalIntoModelList( m_info->localSource ) )
	{
		qDebug() << "Oops cant blend original file into modellist : " << m_info->localSource << endl;
		emit( i18n( "<qt>There were problems applying the diff <b>%1</b> to the file <b>%2</b>.</qt>", m_info->destination.url(), m_info->source.url() ) );
		return false;
	}

	updateModelListActions();
	show();

	return true;
}

bool KompareModelList::openDirAndDiff()
{
	clear();

	if ( parseDiffOutput( readFile( m_info->localDestination ) ) != 0 )
	{
		emit error( i18n( "<qt>No models or no differences, this file: <b>%1</b>, is not a valid diff file.</qt>", m_info->destination.url() ) );
		return false;
	}

	setDepthAndApplied();

	// Do our thing :)
	if ( !blendOriginalIntoModelList( m_info->localSource ) )
	{
		// Trouble blending the original into the model
		qDebug() << "Oops cant blend original dir into modellist : " << m_info->localSource << endl;
		emit error( i18n( "<qt>There were problems applying the diff <b>%1</b> to the folder <b>%2</b>.</qt>", m_info->destination.url(), m_info->source.url() ) );
		return false;
	}

	updateModelListActions();
	show();

	return true;
}

void KompareModelList::slotSaveDestination()
{
	// Unnecessary safety check! We can now guarantee that saving is only possible when there is a model and there are unsaved changes
	if ( m_selectedModel )
	{
		saveDestination( m_selectedModel );
		m_save->setEnabled( false );
		emit updateActions();
	}
}

bool KompareModelList::saveDestination( DiffModel* model )
{
	qDebug() << "KompareModelList::saveDestination: " << endl;

	// Unecessary safety check, we can guarantee there are unsaved changes!!!
	if( !model->hasUnsavedChanges() )
		return true;

	KTemporaryFile temp;
	
	if( !temp.open() ) {
		emit error( i18n( "Could not open a temporary file." ) );
		temp.remove();
		return false;
	}

	QTextStream stream( &temp );
	QStringList list;

	DiffHunkListConstIterator hunkIt = model->hunks()->constBegin();
	DiffHunkListConstIterator hEnd   = model->hunks()->constEnd();

	for( ; hunkIt != hEnd; ++hunkIt )
	{
		DiffHunk* hunk = *hunkIt;

		DifferenceListConstIterator diffIt = hunk->differences().constBegin();
		DifferenceListConstIterator dEnd   = hunk->differences().constEnd();

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

	// qDebug() << "Everything: " << endl << list.join( "\n" ) << endl;

	if( list.count() > 0 )
		stream << list.join( "" );
	if( temp.error() != QFile::NoError ) {
		emit error( i18n( "<qt>Could not write to the temporary file <b>%1</b>, deleting it.</qt>", temp.fileName() ) );
		temp.remove();
		return false;
	}

	temp.close();
	if( temp.error() != QFile::NoError ) {
		emit error( i18n( "<qt>Could not write to the temporary file <b>%1</b>, deleting it.</qt>", temp.fileName() ) );
		temp.remove();
		return false;
	}

	bool result = false;

	// Make sure the destination directory exists, it is possible when using -N to not have the destination dir/file available
	if ( m_info->mode == Kompare::ComparingDirs )
	{
		// Dont use destination which was used for creating the diff directly, use the original URL!!!
		// FIXME!!! Wrong destination this way! Need to add the sub directory to the url!!!!
		qDebug() << "Tempfilename (save) : " << temp.fileName() << endl;
		qDebug() << "Model->path+file    : " << model->destinationPath() << model->destinationFile() << endl;
		qDebug() << "info->localdest     : " << m_info->localDestination << endl;
		QString tmp = model->destinationPath() + model->destinationFile();
		if ( tmp.startsWith( m_info->localDestination ) ) // It should, if not serious trouble...
			tmp.remove( 0, m_info->localDestination.size() );
		qDebug() << "DestinationURL      : " << m_info->destination << endl;
		qDebug() << "tmp                 : " << tmp << endl;
		KIO::UDSEntry entry;
		QUrl fullDestinationPath = m_info->destination;
		fullDestinationPath.setPath( fullDestinationPath.path() + "/" + tmp );
		qDebug() << "fullDestinationPath : " << fullDestinationPath << endl;
		if ( !KIO::NetAccess::stat( fullDestinationPath.path(), entry, m_widgetForKIO ) )
		{
			if ( !KIO::NetAccess::mkdir( fullDestinationPath.path(), m_widgetForKIO ) )
			{
				emit error( i18n( "<qt>Could not create destination directory <b>%1</b>.\nThe file has not been saved.</qt>", fullDestinationPath.path() ) );
				return false;
			}
		}
		result = KIO::NetAccess::upload( temp.fileName(), fullDestinationPath, m_widgetForKIO );
	}
	else
	{
		qDebug() << "Tempfilename   : " << temp.fileName() << endl;
		qDebug() << "DestinationURL : " << m_info->destination << endl;
		result = KIO::NetAccess::upload( temp.fileName(), m_info->destination, m_widgetForKIO );
		qDebug() << "true or false?" << result << endl;
	}

	if ( !result )
	{
		// FIXME: Wrong first argument given in case of comparing directories!
		emit error( i18n( "<qt>Could not upload the temporary file to the destination location <b>%1</b>. The temporary file is still available under: <b>%2</b>. You can manually copy it to the right place.</qt>", m_info->destination.url(), temp.fileName() ) );
                //Don't remove file when we delete temp and don't leak it.
                temp.setAutoRemove(false);
	}
	else
	{
		temp.remove();
	}

	// If saving was fine set all differences to saved so we can start again with a clean slate
	if ( result )
	{
		DifferenceListConstIterator diffIt = model->differences()->constBegin();
		DifferenceListConstIterator endIt  = model->differences()->constEnd();

		for (; diffIt != endIt; ++diffIt )
		{
			(*diffIt)->setUnsaved( false );
		}
	}

	return true;
}

bool KompareModelList::saveAll()
{
	if ( modelCount() == 0 )
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
	if ( !encoding.compare( "default", Qt::CaseInsensitive ) )
	{
		m_textCodec = QTextCodec::codecForLocale();
	}
	else
	{
		qDebug() << "Encoding : " << encoding << endl;
		m_textCodec = KCharsets::charsets()->codecForName( encoding.toLatin1() );
		qDebug() << "TextCodec: " << m_textCodec << endl;
		if ( !m_textCodec )
			m_textCodec = QTextCodec::codecForLocale();
	}
	qDebug() << "TextCodec: " << m_textCodec << endl;
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
			if ( m_info->mode != Kompare::ShowingDiff )
			{
				qDebug() << "Blend this crap please and do not give me any conflicts..." << endl;
				blendOriginalIntoModelList( m_info->localSource );
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

	m_diffProcess->deleteLater();
	m_diffProcess = 0;
}

void KompareModelList::slotDirectoryChanged( const QString& /*dir*/ )
{
	// some debug output to see if watching works properly
	qDebug() << "Yippie directories are being watched !!! :)" << endl;
	if ( m_diffProcess )
	{
		emit status( Kompare::ReRunningDiff );
		m_diffProcess->start();
	}
}

void KompareModelList::slotFileChanged( const QString& /*file*/ )
{
	// some debug output to see if watching works properly
	qDebug() << "Yippie files are being watched !!! :)" << endl;
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
	int oldpos = 0;
	// split that does not strip the split char
#ifdef QT_OS_MAC
	const char split = '\r';
#else
	const char split = '\n';
#endif
	while ( ( pos = contents.indexOf( split, oldpos ) ) >= 0 )
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
	file.open( QIODevice::ReadOnly );

	QTextStream stream( &file );
	qDebug() << "Codec = " << m_textCodec << endl;

	if (  !m_textCodec )
		m_textCodec = QTextCodec::codecForLocale();

	stream.setCodec(  m_textCodec );

	QString contents = stream.readAll();

	file.close();

	return contents;
}

bool KompareModelList::openDiff( const QString& diffFile )
{
	qDebug() << "Stupid :) Url = " << diffFile << endl;

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

bool KompareModelList::parseAndOpenDiff(const QString& diff)
{
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

	DiffModelListConstIterator modelIt = m_models->constBegin();
	DiffModelListConstIterator mEnd    = m_models->constEnd();

	for ( ; modelIt != mEnd; ++modelIt )
	{
		diff += (*modelIt)->recreateDiff();
	}
	return diff;
}

bool KompareModelList::saveDiff( const QString& url, QString directory, DiffSettings* diffSettings )
{
	qDebug() << "KompareModelList::saveDiff: " << endl;

	m_diffTemp = new KTemporaryFile();
	m_diffURL = url;

	if( !m_diffTemp->open() ) {
		emit error( i18n( "Could not open a temporary file." ) );
		m_diffTemp->remove();
		delete m_diffTemp;
		m_diffTemp = 0;
		return false;
	}

	m_diffProcess = new KompareProcess( diffSettings, Kompare::Custom, m_info->localSource, m_info->localDestination, directory );
	m_diffProcess->setEncoding( m_encoding );

	connect( m_diffProcess, SIGNAL(diffHasFinished( bool )),
	         this, SLOT(slotWriteDiffOutput( bool )) );

	emit status( Kompare::RunningDiff );
	m_diffProcess->start();
	return true;
}

void KompareModelList::slotWriteDiffOutput( bool success )
{
	qDebug() << "Success = " << success << endl;

	if( success )
	{
		QTextStream stream( m_diffTemp );

		stream << m_diffProcess->diffOutput();

		m_diffTemp->close();

		if( false /*|| m_diffTemp->status() != 0 */)
		{
			emit error( i18n( "Could not write to the temporary file." ) );
		}

		KIO::NetAccess::upload( m_diffTemp->fileName(), QUrl::fromLocalFile( m_diffURL ), m_widgetForKIO );

		emit status( Kompare::FinishedWritingDiff );
	}

	m_diffURL.truncate( 0 );
	m_diffTemp->remove();

	delete m_diffTemp;
	m_diffTemp = 0;

	delete m_diffProcess;
	m_diffProcess = 0;
}

void KompareModelList::slotSelectionChanged( const Diff2::DiffModel* model, const Diff2::Difference* diff )
{
// This method will signal all the other objects about a change in selection,
// it will emit setSelection( const DiffModel*, const Difference* ) to all who are connected
	qDebug() << "KompareModelList::slotSelectionChanged( " << model << ", " << diff << " )" << endl;
	qDebug() << "Sender is : " << sender()->metaObject()->className() << endl;
//	qDebug() << kBacktrace() << endl;

	m_selectedModel = const_cast<DiffModel*>(model);
	m_modelIndex = m_models->indexOf( m_selectedModel );
	qDebug() << "m_modelIndex = " << m_modelIndex << endl;
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
	qDebug() << "KompareModelList::slotSelectionChanged( " << diff << " )" << endl;
	qDebug() << "Sender is : " << sender()->metaObject()->className() << endl;

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
	qDebug() << "KompareModelList::firstModel()" << endl;
	m_modelIndex = 0;
	qDebug() << "m_modelIndex = " << m_modelIndex << endl;

	m_selectedModel = m_models->first();

	return m_selectedModel;
}

DiffModel* KompareModelList::lastModel()
{
	qDebug() << "KompareModelList::lastModel()" << endl;
	m_modelIndex = m_models->count() - 1;
	qDebug() << "m_modelIndex = " << m_modelIndex << endl;

	m_selectedModel = m_models->last();

	return m_selectedModel;
}

DiffModel* KompareModelList::prevModel()
{
	qDebug() << "KompareModelList::prevModel()" << endl;
	if ( m_modelIndex > 0 && --m_modelIndex < m_models->count() )
	{
		qDebug() << "m_modelIndex = " << m_modelIndex << endl;
		m_selectedModel = (*m_models)[ m_modelIndex ];
	}
	else
	{
		m_selectedModel = 0;
		m_modelIndex = 0;
		qDebug() << "m_modelIndex = " << m_modelIndex << endl;
	}

	return m_selectedModel;
}

DiffModel* KompareModelList::nextModel()
{
	qDebug() << "KompareModelList::nextModel()" << endl;
	if ( ++m_modelIndex < m_models->count() )
	{
		qDebug() << "m_modelIndex = " << m_modelIndex << endl;
		m_selectedModel = (*m_models)[ m_modelIndex ];
	}
	else
	{
		m_selectedModel = 0;
		m_modelIndex = 0;
		qDebug() << "m_modelIndex = " << m_modelIndex << endl;
	}

	return m_selectedModel;
}

KActionCollection* KompareModelList::actionCollection() const
{
    return m_actionCollection;
}

void KompareModelList::slotPreviousDifference()
{
	qDebug() << "slotPreviousDifference called" << endl;
	if ( ( m_selectedDifference = m_selectedModel->prevDifference() ) != 0 )
	{
		emit setSelection( m_selectedDifference );
		emit setStatusBarModelInfo( findModel( m_selectedModel ), m_selectedModel->findDifference( m_selectedDifference ), modelCount(), differenceCount(), m_selectedModel->appliedCount() );
		updateModelListActions();
		return;
	}

	qDebug() << "**** no previous difference... ok lets find the previous model..." << endl;

	if ( ( m_selectedModel = prevModel() ) != 0 )
	{
		m_selectedDifference = m_selectedModel->lastDifference();

		emit setSelection( m_selectedModel, m_selectedDifference );
		emit setStatusBarModelInfo( findModel( m_selectedModel ), m_selectedModel->findDifference( m_selectedDifference ), modelCount(), differenceCount(), m_selectedModel->appliedCount() );
		updateModelListActions();
		return;
	}


	qDebug() << "**** !!! No previous model, ok backup plan activated..." << endl;

	// Backup plan
	m_selectedModel = firstModel();
	m_selectedDifference = m_selectedModel->firstDifference();

	emit setSelection( m_selectedModel, m_selectedDifference );
	emit setStatusBarModelInfo( findModel( m_selectedModel ), m_selectedModel->findDifference( m_selectedDifference ), modelCount(), differenceCount(), m_selectedModel->appliedCount() );
	updateModelListActions();
}

void KompareModelList::slotNextDifference()
{
	qDebug() << "slotNextDifference called" << endl;
	if ( ( m_selectedDifference = m_selectedModel->nextDifference() ) != 0 )
	{
		emit setSelection( m_selectedDifference );
		emit setStatusBarModelInfo( findModel( m_selectedModel ), m_selectedModel->findDifference( m_selectedDifference ), modelCount(), differenceCount(), m_selectedModel->appliedCount() );
		updateModelListActions();
		return;
	}

	qDebug() << "**** no next difference... ok lets find the next model..." << endl;

	if ( ( m_selectedModel = nextModel() ) != 0 )
	{
		m_selectedDifference = m_selectedModel->firstDifference();

		emit setSelection( m_selectedModel, m_selectedDifference );
		emit setStatusBarModelInfo( findModel( m_selectedModel ), m_selectedModel->findDifference( m_selectedDifference ), modelCount(), differenceCount(), m_selectedModel->appliedCount() );
		updateModelListActions();
		return;
	}

	qDebug() << "**** !!! No next model, ok backup plan activated..." << endl;

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
	qDebug() << "KompareModelList::parseDiffOutput" << endl;
	emit diffString( diff );

	QStringList diffLines = split( diff );

	Parser* parser = new Parser( this );
	bool malformed = false;
	m_models = parser->parse( diffLines, &malformed );

	m_info->generator = parser->generator();
	m_info->format    = parser->format();

	delete parser;

	if ( m_models )
	{
		if ( malformed )
		{
			qDebug() << "Malformed diff" << endl;
			emit error( i18n( "The diff is malformed. Some lines could not be parsed and will not be displayed in the diff view." ) );
			// proceed anyway with the lines which have been parsed
		}
		m_selectedModel = firstModel();
		qDebug() << "Ok there are differences..." << endl;
		m_selectedDifference = m_selectedModel->firstDifference();
		emit setStatusBarModelInfo( 0, 0, modelCount(), differenceCount(), 0);
	}
	else
	{
		// Wow trouble, no models, so no differences...
		qDebug() << "Now i'll be damned, there should be models here !!!" << endl;
		return -1;
	}

	return 0;
}

bool KompareModelList::blendOriginalIntoModelList( const QString& localURL )
{
	qDebug() << "Hurrah we are blending..." << endl;
	QFileInfo fi( localURL );

	bool result = false;
	DiffModel* model;

	QString fileContents;

	if ( fi.isDir() )
	{ // is a dir
		qDebug() << "Blend Dir" << endl;
//		QDir dir( localURL, QString(), QDir::Name|QDir::DirsFirst, QDir::TypeMask );
		DiffModelListIterator modelIt = m_models->begin();
		DiffModelListIterator mEnd    = m_models->end();
		for ( ; modelIt != mEnd; ++modelIt )
		{
			model = *modelIt;
			qDebug() << "Model : " << model << endl;
			QString filename = model->source();
			if ( !filename.startsWith( localURL ) )
				filename = QDir(localURL).filePath(filename);
			QFileInfo fi2( filename );
			if ( fi2.exists() )
			{
				qDebug() << "Reading from: " << filename << endl;
				fileContents = readFile( filename );
				result = blendFile( model, fileContents );
			}
			else
			{
				qDebug() << "File " << filename << " does not exist !" << endl;
				qDebug() << "Assume empty file !" << endl;
				fileContents.truncate( 0 );
				result = blendFile( model, fileContents );
			}
		}
		qDebug() << "End of Blend Dir" << endl;
	}
	else if ( fi.isFile() )
	{ // is a file
		qDebug() << "Blend File" << endl;
		qDebug() << "Reading from: " << localURL << endl;
		fileContents = readFile( localURL );

		result = blendFile( (*m_models)[ 0 ], fileContents );
		qDebug() << "End of Blend File" << endl;
	}

	return result;
}

bool KompareModelList::blendFile( DiffModel* model, const QString& fileContents )
{
	if ( !model )
	{
		qDebug() << "**** model is null :(" << endl;
		return false;
	}

	model->setBlended( true );

	int srcLineNo = 1, destLineNo = 1;

	QStringList list = split( fileContents );
	QLinkedList<QString> lines;
	foreach (const QString &str, list) {
		lines.append(str);
	}

	QLinkedList<QString>::ConstIterator linesIt = lines.begin();
	QLinkedList<QString>::ConstIterator lEnd    = lines.end();

	DiffHunkList* hunks = model->hunks();
	qDebug() << "Hunks in hunklist: " << hunks->count() << endl;
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

			hunkIt = ++hunks->insert( hunkIt, newHunk );

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
		destLineNo += hunk->destinationLineCount();
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
		qDebug() << "Number of differences in hunkDiffList = " << diffList.count() << endl;

		DifferenceListIterator tempIt;
		Difference* diff;

		for ( ; diffIt != dEnd; ++diffIt )
		{
			diff = *diffIt;
			qDebug() << "*(Diff it) = " << diff << endl;
			// Check if there are lines in the original file before the difference
			// that are not yet in the diff. If so create new Unchanged diff
			if ( srcLineNo < diff->sourceLineNumber() )
			{
				newDiff = new Difference( srcLineNo, destLineNo,
				              Difference::Unchanged | Difference::AddedByBlend );
				newHunk->add( newDiff );
				while ( srcLineNo < diff->sourceLineNumber() && linesIt != lEnd )
				{
//					qDebug() << "SourceLine = " << srcLineNo << ": " << *linesIt << endl;
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
				qDebug() << "Unchanged" << endl;
				for ( int i = 0; i < diff->sourceLineCount(); i++ )
				{
					if ( linesIt != lEnd && *linesIt != diff->sourceLineAt( i )->string() )
					{
						qDebug() << "Conflict: SourceLine = " << srcLineNo << ": " << *linesIt << endl;
						qDebug() << "Conflict: DiffLine   = " << diff->sourceLineNumber() + i << ": " << diff->sourceLineAt( i )->string() << endl;

						// Do conflict resolution (well sort of)
						diff->sourceLineAt( i )->setConflictString( *linesIt );
						diff->setConflict( true );
					}
//					qDebug() << "SourceLine = " << srcLineNo << ": " << *linesIt << endl;
//					qDebug() << "DiffLine   = " << diff->sourceLineNumber() + i << ": " << diff->sourceLineAt( i )->string() << endl;
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
				qDebug() << "Change" << endl;

				//QStringListConstIterator saveIt = linesIt;

				for ( int i = 0; i < diff->sourceLineCount(); i++ )
				{
					if ( linesIt != lEnd && *linesIt != diff->sourceLineAt( i )->string() )
					{
						qDebug() << "Conflict: SourceLine = " << srcLineNo << ": " << *linesIt << endl;
						qDebug() << "Conflict: DiffLine   = " << diff->sourceLineNumber() + i << ": " << diff->sourceLineAt( i )->string() << endl;

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
				qDebug() << "Insert" << endl;
				destLineNo += diff->destinationLineCount();
				tempIt = diffIt;
				--diffIt;
				diffList.remove( tempIt );
				newHunk->add( diff );
				newModel->addDiff( diff );
				break;
			case Difference::Delete:
				qDebug() << "Delete" << endl;
				qDebug() << "Number of lines in Delete: " << diff->sourceLineCount() << endl;
				for ( int i = 0; i < diff->sourceLineCount(); i++ )
				{
					if ( linesIt != lEnd && *linesIt != diff->sourceLineAt( i )->string() )
					{
						qDebug() << "Conflict: SourceLine = " << srcLineNo << ": " << *linesIt << endl;
						qDebug() << "Conflict: DiffLine   = " << diff->sourceLineNumber() + i << ": " << diff->sourceLineAt( i )->string() << endl;

						// Do conflict resolution (well sort of)
						diff->sourceLineAt( i )->setConflictString( *linesIt );
						diff->setConflict( true );
					}

//					qDebug() << "SourceLine = " << srcLineNo << ": " << *it << endl;
//					qDebug() << "DiffLine   = " << diff->sourceLineNumber() + i << ": " << diff->sourceLineAt( i )->string() << endl;
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
				qDebug() << "****, some diff type we do not know about ???" << endl;
			}
		}
	}
#endif

/*
	diffList = newModel->differences();

	diff = diffList.first();
	qDebug() << "Count = " << diffList.count() << endl;
	for ( diff = diffList.first(); diff; diff = diffList.next() )
	{
		qDebug() << "sourcelinenumber = " << diff->sourceLineNumber() << endl;
	}
*/

	m_selectedModel = firstModel();

	m_selectedDifference = m_selectedModel->firstDifference();

	return true;
}

void KompareModelList::show()
{
	qDebug() << "KompareModelList::Show Number of models = " << m_models->count() << endl;
	emit modelsChanged( m_models );
	emit setSelection( m_selectedModel, m_selectedDifference );
}

void KompareModelList::clear()
{
	if ( m_models )
		m_models->clear();

	emit modelsChanged( m_models );
}

void KompareModelList::refresh()
{
	// FIXME: I can imagine blending also wants to be refreshed so make a switch case here
	compare(m_info->mode);
}

void KompareModelList::swap()
{
	//FIXME Not sure if any mode could be swapped
	if ( m_info->mode == Kompare::ComparingFiles )
		compare(m_info->mode);
	else if ( m_info->mode == Kompare::ComparingDirs )
		compare(m_info->mode);
}

bool KompareModelList::hasUnsavedChanges() const
{
	if ( modelCount() == 0 )
		return false;

	DiffModelListConstIterator modelIt = m_models->constBegin();
	DiffModelListConstIterator endIt   = m_models->constEnd();

	for ( ; modelIt != endIt; ++modelIt )
	{
		if ( (*modelIt)->hasUnsavedChanges() )
			return true;
	}
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

void KompareModelList::slotKompareInfo( struct Kompare::Info* info )
{
	m_info = info;
}

bool KompareModelList::setSelectedModel( DiffModel* model )
{
	qDebug() << "KompareModelList::setSelectedModel( " << model << " )" << endl;

	if ( model != m_selectedModel )
	{
		if ( !m_models->contains( model ) )
			return false;
		qDebug() << "m_selectedModel (was) = " << m_selectedModel << endl;
		m_modelIndex = m_models->indexOf( model );
		qDebug() << "m_selectedModel (is)  = " << m_selectedModel << endl;
		m_selectedModel = model;
	}

	updateModelListActions();

	return true;
}

void KompareModelList::updateModelListActions()
{
	if ( m_models && m_selectedModel && m_selectedDifference )
	{
		if ( m_isReadWrite )
		{
			if ( m_selectedModel->appliedCount() != m_selectedModel->differenceCount() )
				m_applyAll->setEnabled( true );
			else
				m_applyAll->setEnabled( false );

			if ( m_selectedModel->appliedCount() != 0 )
				m_unapplyAll->setEnabled( true );
			else
				m_unapplyAll->setEnabled( false );

			m_applyDifference->setEnabled( m_selectedDifference->applied() == false );
			m_unApplyDifference->setEnabled( m_selectedDifference->applied() == true );
			m_save->setEnabled( m_selectedModel->hasUnsavedChanges() );
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
	qDebug() << "KompareModelList::hasPrevModel()" << endl;

	if (  m_modelIndex > 0 )
	{
//		qDebug() << "has prev model" << endl;
		return true;
	}

//	qDebug() << "doesn't have a prev model, this is the first one..." << endl;

	return false;
}

bool KompareModelList::hasNextModel() const
{
	qDebug() << "KompareModelList::hasNextModel()" << endl;

	if ( m_modelIndex < (  m_models->count() - 1 ) )
	{
//		qDebug() << "has next model" << endl;
		return true;
	}

//	qDebug() << "doesn't have a next model, this is the last one..." << endl;
	return false;
}

bool KompareModelList::hasPrevDiff() const
{
//	qDebug() << "KompareModelList::hasPrevDiff()" << endl;
	int index = m_selectedModel->diffIndex();

	if ( index > 0 )
	{
//		qDebug() << "has prev difference in same model" << endl;
		return true;
	}

	if ( hasPrevModel() )
	{
//		qDebug() << "has prev difference but in prev model" << endl;
		return true;
	}

//	qDebug() << "doesn't have a prev difference, not even in the previous model because there is no previous model" << endl;

	return false;
}

bool KompareModelList::hasNextDiff() const
{
//	qDebug() << "KompareModelList::hasNextDiff()" << endl;
	int index = m_selectedModel->diffIndex();

	if ( index < ( m_selectedModel->differenceCount() - 1 ) )
	{
//		qDebug() << "has next difference in same model" << endl;
		return true;
	}

	if ( hasNextModel() )
	{
//		qDebug() << "has next difference but in next model" << endl;
		return true;
	}

//	qDebug() << "doesn't have a next difference, not even in next model because there is no next model" << endl;

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

/*
SPDX-FileCopyrightText: 2001-2005,2009 Otto Bruggeman <otto.bruggeman@home.nl>
SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
SPDX-FileCopyrightText: 2007-2010 Kevin Kofler <kevin.kofler@chello.at>
SPDX-FileCopyrightText: 2012 Jean -Nicolas Artaud <jeannicolasartaud@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "komparemodellist.h"

#include <QAction>
#include <QFile>
#include <QDir>
#include <QTextCodec>
#include <QTextStream>
#include <QList>
#include <QTemporaryFile>
#include <QMimeType>
#include <QMimeDatabase>

#include <KActionCollection>
#include <KCharsets>
#include <KDirWatch>
#include <KIO/UDSEntry>
#include <KIO/StatJob>
#include <KIO/MkdirJob>
#include <KIO/FileCopyJob>
#include <KLocalizedString>
#include <KStandardAction>

#include <komparediffdebug.h>
#include "diffhunk.h"
#include "kompareprocess.h"
#include "parser.h"

using namespace Diff2;

KompareModelList::KompareModelList(DiffSettings* diffSettings, QWidget* widgetForKIO, QObject* parent, const char* name, bool supportReadWrite)
    : QObject(parent),
      m_diffProcess(nullptr),
      m_diffSettings(diffSettings),
      m_models(nullptr),
      m_selectedModel(nullptr),
      m_selectedDifference(nullptr),
      m_modelIndex(0),
      m_info(nullptr),
      m_textCodec(nullptr),
      m_widgetForKIO(widgetForKIO),
      m_isReadWrite(supportReadWrite)
{
    qCDebug(LIBKOMPAREDIFF2) << "Show me the arguments: " << diffSettings << ", " << widgetForKIO << ", " << parent << ", " << name;
    m_actionCollection = new KActionCollection(this);
    if (supportReadWrite) {
        m_applyDifference = m_actionCollection->addAction(QStringLiteral("difference_apply"), this, &KompareModelList::slotActionApplyDifference);
        m_applyDifference->setIcon(QIcon::fromTheme(QStringLiteral("arrow-right")));
        m_applyDifference->setText(i18nc("@action", "&Apply Difference"));
        m_actionCollection->setDefaultShortcut(m_applyDifference, QKeySequence(Qt::Key_Space));
        m_unApplyDifference = m_actionCollection->addAction(QStringLiteral("difference_unapply"), this, &KompareModelList::slotActionUnApplyDifference);
        m_unApplyDifference->setIcon(QIcon::fromTheme(QStringLiteral("arrow-left")));
        m_unApplyDifference->setText(i18nc("@action", "Un&apply Difference"));
        m_actionCollection->setDefaultShortcut(m_unApplyDifference, QKeySequence(Qt::Key_Backspace));
        m_applyAll = m_actionCollection->addAction(QStringLiteral("difference_applyall"), this, &KompareModelList::slotActionApplyAllDifferences);
        m_applyAll->setIcon(QIcon::fromTheme(QStringLiteral("arrow-right-double")));
        m_applyAll->setText(i18nc("@action", "App&ly All"));
        m_actionCollection->setDefaultShortcut(m_applyAll, QKeySequence(Qt::CTRL | Qt::Key_A));
        m_unapplyAll = m_actionCollection->addAction(QStringLiteral("difference_unapplyall"), this, &KompareModelList::slotActionUnapplyAllDifferences);
        m_unapplyAll->setIcon(QIcon::fromTheme(QStringLiteral("arrow-left-double")));
        m_unapplyAll->setText(i18nc("@action", "&Unapply All"));
        m_actionCollection->setDefaultShortcut(m_unapplyAll, QKeySequence(Qt::CTRL | Qt::Key_U));
    } else {
        m_applyDifference = nullptr;
        m_unApplyDifference = nullptr;
        m_applyAll = nullptr;
        m_unapplyAll = nullptr;
    }
    m_previousFile = m_actionCollection->addAction(QStringLiteral("difference_previousfile"), this, &KompareModelList::slotPreviousModel);
    m_previousFile->setIcon(QIcon::fromTheme(QStringLiteral("arrow-up-double")));
    m_previousFile->setText(i18nc("@action", "P&revious File"));
    m_actionCollection->setDefaultShortcut(m_previousFile, QKeySequence(Qt::CTRL | Qt::Key_PageUp));
    m_nextFile = m_actionCollection->addAction(QStringLiteral("difference_nextfile"), this, &KompareModelList::slotNextModel);
    m_nextFile->setIcon(QIcon::fromTheme(QStringLiteral("arrow-down-double")));
    m_nextFile->setText(i18nc("@action", "N&ext File"));
    m_actionCollection->setDefaultShortcut(m_nextFile, QKeySequence(Qt::CTRL | Qt::Key_PageDown));
    m_previousDifference = m_actionCollection->addAction(QStringLiteral("difference_previous"), this, &KompareModelList::slotPreviousDifference);
    m_previousDifference->setIcon(QIcon::fromTheme(QStringLiteral("arrow-up")));
    m_previousDifference->setText(i18nc("@action", "&Previous Difference"));
    m_actionCollection->setDefaultShortcut(m_previousDifference, QKeySequence(Qt::CTRL | Qt::Key_Up));
    m_nextDifference = m_actionCollection->addAction(QStringLiteral("difference_next"), this, &KompareModelList::slotNextDifference);
    m_nextDifference->setIcon(QIcon::fromTheme(QStringLiteral("arrow-down")));
    m_nextDifference->setText(i18nc("@action", "&Next Difference"));
    m_actionCollection->setDefaultShortcut(m_nextDifference, QKeySequence(Qt::CTRL | Qt::Key_Down));
    m_previousDifference->setEnabled(false);
    m_nextDifference->setEnabled(false);

    if (supportReadWrite) {
        m_save = KStandardAction::save(this, &KompareModelList::slotSaveDestination, m_actionCollection);
        m_save->setEnabled(false);
    } else {
        m_save = nullptr;
    }

    updateModelListActions();
}

KompareModelList::~KompareModelList()
{
    m_selectedModel = nullptr;
    m_selectedDifference = nullptr;
    m_info = nullptr;
    delete m_models;
}

bool KompareModelList::isDirectory(const QString& url) const
{
    QFileInfo fi(url);
    if (fi.isDir())
        return true;
    else
        return false;
}

bool KompareModelList::isDiff(const QString& mimeType) const
{
    if (mimeType == QLatin1String("text/x-patch"))
        return true;
    else
        return false;
}

bool KompareModelList::compare()
{
    bool result = false;

    bool sourceIsDirectory = isDirectory(m_info->localSource);
    bool destinationIsDirectory = isDirectory(m_info->localDestination);

    if (sourceIsDirectory && destinationIsDirectory)
    {
        m_info->mode = Kompare::ComparingDirs;
        result = compare(m_info->mode);
    }
    else if (!sourceIsDirectory && !destinationIsDirectory)
    {
        QFile sourceFile(m_info->localSource);
        sourceFile.open(QIODevice::ReadOnly);
        QMimeDatabase db;
        QString sourceMimeType = (db.mimeTypeForData(sourceFile.readAll())).name();
        sourceFile.close();
        qCDebug(LIBKOMPAREDIFF2) << "Mimetype source     : " << sourceMimeType;

        QFile destinationFile(m_info->localDestination);
        destinationFile.open(QIODevice::ReadOnly);
        QString destinationMimeType = (db.mimeTypeForData(destinationFile.readAll())).name();
        destinationFile.close();
        qCDebug(LIBKOMPAREDIFF2) << "Mimetype destination: " << destinationMimeType;

        // Not checking if it is a text file/something diff can even compare, we'll let diff handle that
        if (!isDiff(sourceMimeType) && isDiff(destinationMimeType))
        {
            qCDebug(LIBKOMPAREDIFF2) << "Blending destination into source...";
            m_info->mode = Kompare::BlendingFile;
            result = openFileAndDiff();
        }
        else if (isDiff(sourceMimeType) && !isDiff(destinationMimeType))
        {
            qCDebug(LIBKOMPAREDIFF2) << "Blending source into destination...";
            m_info->mode = Kompare::BlendingFile;
            // Swap source and destination before calling this
            m_info->swapSourceWithDestination();
            // Do we need to notify anyone we swapped source and destination?
            // No we do not need to notify anyone about swapping source with destination
            result = openFileAndDiff();
        }
        else
        {
            qCDebug(LIBKOMPAREDIFF2) << "Comparing source with destination";
            m_info->mode = Kompare::ComparingFiles;
            result = compare(m_info->mode);
        }
    }
    else if (sourceIsDirectory && !destinationIsDirectory)
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

    m_diffProcess = new KompareProcess(m_diffSettings, Kompare::Custom, m_info->localSource, m_info->localDestination, QString(), mode);
    m_diffProcess->setEncoding(m_encoding);

    connect(m_diffProcess, &KompareProcess::diffHasFinished,
            this, &KompareModelList::slotDiffProcessFinished);

    Q_EMIT status(Kompare::RunningDiff);
    m_diffProcess->start();

    return true;
}

static QString lstripSeparators(const QString& from, uint count)
{
    int position = 0;
    for (uint i = 0; i < count; ++i)
    {
        position = from.indexOf(QLatin1Char('/'), position);
        if (position == -1)
        {
            break;
        }
    }
    if (position == -1)
    {
        return QString();
    }

    return from.mid(position + 1);
}

void KompareModelList::setDepthAndApplied()
{
    // Splice to avoid calling ~DiffModelList
    const QList<Diff2::DiffModel*> splicedModelList(*m_models);
    for (DiffModel* model : splicedModelList) {
        model->setSourceFile(lstripSeparators(model->source(), m_info->depth));
        model->setDestinationFile(lstripSeparators(model->destination(), m_info->depth));
        model->applyAllDifferences(m_info->applied);
    }
}

bool KompareModelList::openFileAndDiff()
{
    clear();

    if (parseDiffOutput(readFile(m_info->localDestination)) != 0)
    {
        Q_EMIT error(i18n("<qt>No models or no differences, this file: <b>%1</b>, is not a valid diff file.</qt>", m_info->destination.url()));
        return false;
    }

    setDepthAndApplied();

    if (!blendOriginalIntoModelList(m_info->localSource))
    {
        qCDebug(LIBKOMPAREDIFF2) << "Oops cant blend original file into modellist : " << m_info->localSource;
        Q_EMIT error(i18n("<qt>There were problems applying the diff <b>%1</b> to the file <b>%2</b>.</qt>", m_info->destination.url(), m_info->source.url()));
        return false;
    }

    updateModelListActions();
    show();

    return true;
}

bool KompareModelList::openDirAndDiff()
{
    clear();

    if (parseDiffOutput(readFile(m_info->localDestination)) != 0)
    {
        Q_EMIT error(i18n("<qt>No models or no differences, this file: <b>%1</b>, is not a valid diff file.</qt>", m_info->destination.url()));
        return false;
    }

    setDepthAndApplied();

    // Do our thing :)
    if (!blendOriginalIntoModelList(m_info->localSource))
    {
        // Trouble blending the original into the model
        qCDebug(LIBKOMPAREDIFF2) << "Oops cant blend original dir into modellist : " << m_info->localSource;
        Q_EMIT error(i18n("<qt>There were problems applying the diff <b>%1</b> to the folder <b>%2</b>.</qt>", m_info->destination.url(), m_info->source.url()));
        return false;
    }

    updateModelListActions();
    show();

    return true;
}

void KompareModelList::slotSaveDestination()
{
    // Unnecessary safety check! We can now guarantee that saving is only possible when there is a model and there are unsaved changes
    if (m_selectedModel)
    {
        saveDestination(m_selectedModel);
        if (m_save) m_save->setEnabled(false);
        Q_EMIT updateActions();
    }
}

bool KompareModelList::saveDestination(DiffModel* model)
{
    qCDebug(LIBKOMPAREDIFF2) << "KompareModelList::saveDestination: ";

    // Unnecessary safety check, we can guarantee there are unsaved changes!!!
    if (!model->hasUnsavedChanges())
        return true;

    QTemporaryFile temp;

    if (!temp.open()) {
        Q_EMIT error(i18n("Could not open a temporary file."));
        temp.remove();
        return false;
    }

    QTextStream stream(&temp);
    QStringList list;

    DiffHunkListConstIterator hunkIt = model->hunks()->constBegin();
    DiffHunkListConstIterator hEnd   = model->hunks()->constEnd();

    for (; hunkIt != hEnd; ++hunkIt)
    {
        DiffHunk* hunk = *hunkIt;

        DifferenceListConstIterator diffIt = hunk->differences().constBegin();
        DifferenceListConstIterator dEnd   = hunk->differences().constEnd();

        Difference* diff;
        for (; diffIt != dEnd; ++diffIt)
        {
            diff = *diffIt;
            if (!diff->applied())
            {
                DifferenceStringListConstIterator stringIt = diff->destinationLines().begin();
                DifferenceStringListConstIterator sEnd     = diff->destinationLines().end();
                for (; stringIt != sEnd; ++stringIt)
                {
                    list.append((*stringIt)->string());
                }
            }
            else
            {
                DifferenceStringListConstIterator stringIt = diff->sourceLines().begin();
                DifferenceStringListConstIterator sEnd = diff->sourceLines().end();
                for (; stringIt != sEnd; ++stringIt)
                {
                    list.append((*stringIt)->string());
                }
            }
        }
    }

    // qCDebug(LIBKOMPAREDIFF2) << "Everything: " << endl << list.join( "\n" );

    if (list.count() > 0)
        stream << list.join(QString());
    if (temp.error() != QFile::NoError) {
        Q_EMIT error(i18n("<qt>Could not write to the temporary file <b>%1</b>, deleting it.</qt>", temp.fileName()));
        temp.remove();
        return false;
    }

    temp.close();
    if (temp.error() != QFile::NoError) {
        Q_EMIT error(i18n("<qt>Could not write to the temporary file <b>%1</b>, deleting it.</qt>", temp.fileName()));
        temp.remove();
        return false;
    }

    bool result = false;

    // Make sure the destination directory exists, it is possible when using -N to not have the destination dir/file available
    if (m_info->mode == Kompare::ComparingDirs)
    {
        // Don't use destination which was used for creating the diff directly, use the original URL!!!
        // FIXME!!! Wrong destination this way! Need to add the sub directory to the url!!!!
        qCDebug(LIBKOMPAREDIFF2) << "Tempfilename (save) : " << temp.fileName();
        qCDebug(LIBKOMPAREDIFF2) << "Model->path+file    : " << model->destinationPath() << model->destinationFile();
        qCDebug(LIBKOMPAREDIFF2) << "info->localdest     : " << m_info->localDestination;
        QString tmp = model->destinationPath();
        if (tmp.startsWith(m_info->localDestination))     // It should, if not serious trouble...
            tmp.remove(0, m_info->localDestination.size());
        qCDebug(LIBKOMPAREDIFF2) << "DestinationURL      : " << m_info->destination;
        qCDebug(LIBKOMPAREDIFF2) << "tmp                 : " << tmp;
        KIO::UDSEntry entry;
        QUrl fullDestinationPath = m_info->destination;
        fullDestinationPath.setPath(fullDestinationPath.path() + tmp);
        qCDebug(LIBKOMPAREDIFF2) << "fullDestinationPath : " << fullDestinationPath;
        KIO::StatJob* statJob = KIO::stat(fullDestinationPath);
        if (!statJob->exec())
        {
            entry = statJob->statResult();
            KIO::MkdirJob* mkdirJob = KIO::mkdir(fullDestinationPath);
            if (!mkdirJob->exec())
            {
                Q_EMIT error(i18n("<qt>Could not create destination directory <b>%1</b>.\nThe file has not been saved.</qt>", fullDestinationPath.path()));
                return false;
            }
        }
        fullDestinationPath.setPath(fullDestinationPath.path() + model->destinationFile());
        KIO::FileCopyJob* copyJob = KIO::file_copy(QUrl::fromLocalFile(temp.fileName()), fullDestinationPath, -1, KIO::Overwrite);
        result = copyJob->exec();
    }
    else
    {
        qCDebug(LIBKOMPAREDIFF2) << "Tempfilename   : " << temp.fileName();
        qCDebug(LIBKOMPAREDIFF2) << "DestinationURL : " << m_info->destination;

        // Get permissions of existing file and copy temporary file with the same permissions
        int permissions = -1;
        KIO::StatJob* statJob = KIO::stat(m_info->destination);
        result = statJob->exec();
        if (result)
            permissions = statJob->statResult().numberValue(KIO::UDSEntry::UDS_ACCESS);

        KIO::FileCopyJob* copyJob = KIO::file_copy(QUrl::fromLocalFile(temp.fileName()), m_info->destination , permissions, KIO::Overwrite);
        result = copyJob->exec();
        qCDebug(LIBKOMPAREDIFF2) << "true or false?" << result;
    }

    if (!result)
    {
        // FIXME: Wrong first argument given in case of comparing directories!
        Q_EMIT error(i18n("<qt>Could not upload the temporary file to the destination location <b>%1</b>. The temporary file is still available under: <b>%2</b>. You can manually copy it to the right place.</qt>", m_info->destination.url(), temp.fileName()));
        //Don't remove file when we delete temp and don't leak it.
        temp.setAutoRemove(false);
    }
    else
    {
        temp.remove();
    }

    // If saving was fine set all differences to saved so we can start again with a clean slate
    if (result)
    {
        DifferenceListConstIterator diffIt = model->differences()->constBegin();
        DifferenceListConstIterator endIt  = model->differences()->constEnd();

        for (; diffIt != endIt; ++diffIt)
        {
            (*diffIt)->setUnsaved(false);
        }
    }

    return true;
}

bool KompareModelList::saveAll()
{
    if (modelCount() == 0)
        return false;

    DiffModelListIterator it  =  m_models->begin();
    DiffModelListIterator end =  m_models->end();
    for (; it != end; ++it)
    {
        if (!saveDestination(*it))
            return false;
    }

    return true;
}

void KompareModelList::setEncoding(const QString& encoding)
{
    m_encoding = encoding;
    if (!encoding.compare(QLatin1String("default"), Qt::CaseInsensitive))
    {
        m_textCodec = QTextCodec::codecForLocale();
    }
    else
    {
        qCDebug(LIBKOMPAREDIFF2) << "Encoding : " << encoding;
        m_textCodec = KCharsets::charsets()->codecForName(encoding);
        qCDebug(LIBKOMPAREDIFF2) << "TextCodec: " << m_textCodec;
        if (!m_textCodec)
            m_textCodec = QTextCodec::codecForLocale();
    }
    qCDebug(LIBKOMPAREDIFF2) << "TextCodec: " << m_textCodec;
}

void KompareModelList::setReadWrite(bool isReadWrite)
{
    if (m_isReadWrite == isReadWrite)
        return;

    m_isReadWrite = isReadWrite;
    updateModelListActions();
}

bool KompareModelList::isReadWrite() const
{
    return m_isReadWrite;
}

void KompareModelList::slotDiffProcessFinished(bool success)
{
    if (success)
    {
        Q_EMIT status(Kompare::Parsing);
        if (parseDiffOutput(m_diffProcess->diffOutput()) != 0)
        {
            Q_EMIT error(i18n("Could not parse diff output."));
        }
        else
        {
            if (m_info->mode != Kompare::ShowingDiff)
            {
                qCDebug(LIBKOMPAREDIFF2) << "Blend this crap please and do not give me any conflicts...";
                blendOriginalIntoModelList(m_info->localSource);
            }
            updateModelListActions();
            show();
        }
        Q_EMIT status(Kompare::FinishedParsing);
    }
    else if (m_diffProcess->exitStatus() == 0)
    {
        Q_EMIT error(i18n("The files are identical."));
    }
    else
    {
        Q_EMIT error(m_diffProcess->stdErr());
    }

    m_diffProcess->deleteLater();
    m_diffProcess = nullptr;
}

void KompareModelList::slotDirectoryChanged(const QString& /*dir*/)
{
    // some debug output to see if watching works properly
    qCDebug(LIBKOMPAREDIFF2) << "Yippie directories are being watched !!! :)";
    if (m_diffProcess)
    {
        Q_EMIT status(Kompare::ReRunningDiff);
        m_diffProcess->start();
    }
}

void KompareModelList::slotFileChanged(const QString& /*file*/)
{
    // some debug output to see if watching works properly
    qCDebug(LIBKOMPAREDIFF2) << "Yippie files are being watched !!! :)";
    if (m_diffProcess)
    {
        Q_EMIT status(Kompare::ReRunningDiff);
        m_diffProcess->start();
    }
}

QStringList KompareModelList::split(const QString& fileContents)
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
    while ((pos = contents.indexOf(QLatin1Char(split), oldpos)) >= 0)
    {
        list.append(contents.mid(oldpos, pos - oldpos + 1));
        oldpos = pos + 1;
    }

    if (contents.length() > oldpos)
    {
        list.append(contents.right(contents.length() - oldpos));
    }

    return list;
}

QString KompareModelList::readFile(const QString& fileName)
{
    QStringList list;

    QFile file(fileName);
    file.open(QIODevice::ReadOnly);

    QTextStream stream(&file);
    qCDebug(LIBKOMPAREDIFF2) << "Codec = " << m_textCodec;

    if (!m_textCodec)
        m_textCodec = QTextCodec::codecForLocale();

    stream.setCodec(m_textCodec);

    QString contents = stream.readAll();

    file.close();

    return contents;
}

bool KompareModelList::openDiff(const QString& diffFile)
{
    qCDebug(LIBKOMPAREDIFF2) << "Stupid :) Url = " << diffFile;

    if (diffFile.isEmpty())
        return false;

    QString diff = readFile(diffFile);

    clear(); // Clear the current models

    Q_EMIT status(Kompare::Parsing);

    if (parseDiffOutput(diff) != 0)
    {
        Q_EMIT error(i18n("Could not parse diff output."));
        return false;
    }

    updateModelListActions();
    show();

    Q_EMIT status(Kompare::FinishedParsing);

    return true;
}

bool KompareModelList::parseAndOpenDiff(const QString& diff)
{
    clear(); // Clear the current models

    Q_EMIT status(Kompare::Parsing);

    if (parseDiffOutput(diff) != 0)
    {
        Q_EMIT error(i18n("Could not parse diff output."));
        return false;
    }

    updateModelListActions();
    show();

    Q_EMIT status(Kompare::FinishedParsing);
    return true;
}

QString KompareModelList::recreateDiff() const
{
    QString diff;

    DiffModelListConstIterator modelIt = m_models->constBegin();
    DiffModelListConstIterator mEnd    = m_models->constEnd();

    for (; modelIt != mEnd; ++modelIt)
    {
        diff += (*modelIt)->recreateDiff();
    }
    return diff;
}

bool KompareModelList::saveDiff(const QString& url, QString directory, DiffSettings* diffSettings)
{
    qCDebug(LIBKOMPAREDIFF2) << "KompareModelList::saveDiff: ";

    m_diffTemp = new QTemporaryFile();
    m_diffURL = QUrl(url); // ### TODO the "url" argument should be a QUrl

    if (!m_diffTemp->open()) {
        Q_EMIT error(i18n("Could not open a temporary file."));
        m_diffTemp->remove();
        delete m_diffTemp;
        m_diffTemp = nullptr;
        return false;
    }

    m_diffProcess = new KompareProcess(diffSettings, Kompare::Custom, m_info->localSource, m_info->localDestination, directory);
    m_diffProcess->setEncoding(m_encoding);

    connect(m_diffProcess, &KompareProcess::diffHasFinished,
            this, &KompareModelList::slotWriteDiffOutput);

    Q_EMIT status(Kompare::RunningDiff);
    m_diffProcess->start();
    return true;
}

void KompareModelList::slotWriteDiffOutput(bool success)
{
    qCDebug(LIBKOMPAREDIFF2) << "Success = " << success;

    if (success)
    {
        QTextStream stream(m_diffTemp);

        stream << m_diffProcess->diffOutput();

        m_diffTemp->close();

        if (false /*|| m_diffTemp->status() != 0 */)
        {
            Q_EMIT error(i18n("Could not write to the temporary file."));
        }

        KIO::FileCopyJob* copyJob = KIO::file_copy(QUrl::fromLocalFile(m_diffTemp->fileName()), m_diffURL);
        copyJob->exec();

        Q_EMIT status(Kompare::FinishedWritingDiff);
    }

    m_diffURL = QUrl();
    m_diffTemp->remove();

    delete m_diffTemp;
    m_diffTemp = nullptr;

    delete m_diffProcess;
    m_diffProcess = nullptr;
}

void KompareModelList::slotSelectionChanged(const Diff2::DiffModel* model, const Diff2::Difference* diff)
{
// This method will signal all the other objects about a change in selection,
// it will Q_EMIT setSelection( const DiffModel*, const Difference* ) to all who are connected
    qCDebug(LIBKOMPAREDIFF2) << "KompareModelList::slotSelectionChanged( " << model << ", " << diff << " )";
    qCDebug(LIBKOMPAREDIFF2) << "Sender is : " << sender()->metaObject()->className();
//     qCDebug(LIBKOMPAREDIFF2) << kBacktrace();

    m_selectedModel = const_cast<DiffModel*>(model);
    m_modelIndex = m_models->indexOf(m_selectedModel);
    qCDebug(LIBKOMPAREDIFF2) << "m_modelIndex = " << m_modelIndex;
    m_selectedDifference = const_cast<Difference*>(diff);

    m_selectedModel->setSelectedDifference(m_selectedDifference);

    // setSelected* search for the argument in the lists and return false if not found
    // if found they return true and set the m_selected*
    if (!setSelectedModel(m_selectedModel))
    {
        // Backup plan
        m_selectedModel = firstModel();
        m_selectedDifference = m_selectedModel->firstDifference();
    }
    else if (!m_selectedModel->setSelectedDifference(m_selectedDifference))
    {
        // Another backup plan
        m_selectedDifference = m_selectedModel->firstDifference();
    }

    Q_EMIT setSelection(model, diff);
    Q_EMIT setStatusBarModelInfo(findModel(m_selectedModel), m_selectedModel->findDifference(m_selectedDifference), modelCount(), differenceCount(), m_selectedModel->appliedCount());

    updateModelListActions();
}

void KompareModelList::slotSelectionChanged(const Diff2::Difference* diff)
{
// This method will Q_EMIT setSelection( const Difference* ) to whomever is listening
// when for instance in kompareview the selection has changed
    qCDebug(LIBKOMPAREDIFF2) << "KompareModelList::slotSelectionChanged( " << diff << " )";
    qCDebug(LIBKOMPAREDIFF2) << "Sender is : " << sender()->metaObject()->className();

    m_selectedDifference = const_cast<Difference*>(diff);

    if (!m_selectedModel->setSelectedDifference(m_selectedDifference))
    {
        // Backup plan
        m_selectedDifference = m_selectedModel->firstDifference();
    }

    Q_EMIT setSelection(diff);
    Q_EMIT setStatusBarModelInfo(findModel(m_selectedModel), m_selectedModel->findDifference(m_selectedDifference), modelCount(), differenceCount(), m_selectedModel->appliedCount());

    updateModelListActions();
}

void KompareModelList::slotPreviousModel()
{
    if ((m_selectedModel = prevModel()) != nullptr)
    {
        m_selectedDifference = m_selectedModel->firstDifference();
    }
    else
    {
        m_selectedModel = firstModel();
        m_selectedDifference = m_selectedModel->firstDifference();
    }

    Q_EMIT setSelection(m_selectedModel, m_selectedDifference);
    Q_EMIT setStatusBarModelInfo(findModel(m_selectedModel), m_selectedModel->findDifference(m_selectedDifference), modelCount(), differenceCount(), m_selectedModel->appliedCount());
    updateModelListActions();
}

void KompareModelList::slotNextModel()
{
    if ((m_selectedModel = nextModel()) != nullptr)
    {
        m_selectedDifference = m_selectedModel->firstDifference();
    }
    else
    {
        m_selectedModel = lastModel();
        m_selectedDifference = m_selectedModel->firstDifference();
    }

    Q_EMIT setSelection(m_selectedModel, m_selectedDifference);
    Q_EMIT setStatusBarModelInfo(findModel(m_selectedModel), m_selectedModel->findDifference(m_selectedDifference), modelCount(), differenceCount(), m_selectedModel->appliedCount());
    updateModelListActions();
}

DiffModel* KompareModelList::firstModel()
{
    qCDebug(LIBKOMPAREDIFF2) << "KompareModelList::firstModel()";
    m_modelIndex = 0;
    qCDebug(LIBKOMPAREDIFF2) << "m_modelIndex = " << m_modelIndex;

    m_selectedModel = m_models->first();

    return m_selectedModel;
}

DiffModel* KompareModelList::lastModel()
{
    qCDebug(LIBKOMPAREDIFF2) << "KompareModelList::lastModel()";
    m_modelIndex = m_models->count() - 1;
    qCDebug(LIBKOMPAREDIFF2) << "m_modelIndex = " << m_modelIndex;

    m_selectedModel = m_models->last();

    return m_selectedModel;
}

DiffModel* KompareModelList::prevModel()
{
    qCDebug(LIBKOMPAREDIFF2) << "KompareModelList::prevModel()";
    if (m_modelIndex > 0 && --m_modelIndex < m_models->count())
    {
        qCDebug(LIBKOMPAREDIFF2) << "m_modelIndex = " << m_modelIndex;
        m_selectedModel = (*m_models)[ m_modelIndex ];
    }
    else
    {
        m_selectedModel = nullptr;
        m_modelIndex = 0;
        qCDebug(LIBKOMPAREDIFF2) << "m_modelIndex = " << m_modelIndex;
    }

    return m_selectedModel;
}

DiffModel* KompareModelList::nextModel()
{
    qCDebug(LIBKOMPAREDIFF2) << "KompareModelList::nextModel()";
    if (++m_modelIndex < m_models->count())
    {
        qCDebug(LIBKOMPAREDIFF2) << "m_modelIndex = " << m_modelIndex;
        m_selectedModel = (*m_models)[ m_modelIndex ];
    }
    else
    {
        m_selectedModel = nullptr;
        m_modelIndex = 0;
        qCDebug(LIBKOMPAREDIFF2) << "m_modelIndex = " << m_modelIndex;
    }

    return m_selectedModel;
}

KActionCollection* KompareModelList::actionCollection() const
{
    return m_actionCollection;
}

void KompareModelList::slotPreviousDifference()
{
    qCDebug(LIBKOMPAREDIFF2) << "slotPreviousDifference called";
    if ((m_selectedDifference = m_selectedModel->prevDifference()) != nullptr)
    {
        Q_EMIT setSelection(m_selectedDifference);
        Q_EMIT setStatusBarModelInfo(findModel(m_selectedModel), m_selectedModel->findDifference(m_selectedDifference), modelCount(), differenceCount(), m_selectedModel->appliedCount());
        updateModelListActions();
        return;
    }

    qCDebug(LIBKOMPAREDIFF2) << "**** no previous difference... ok lets find the previous model...";

    if ((m_selectedModel = prevModel()) != nullptr)
    {
        m_selectedDifference = m_selectedModel->lastDifference();

        Q_EMIT setSelection(m_selectedModel, m_selectedDifference);
        Q_EMIT setStatusBarModelInfo(findModel(m_selectedModel), m_selectedModel->findDifference(m_selectedDifference), modelCount(), differenceCount(), m_selectedModel->appliedCount());
        updateModelListActions();
        return;
    }


    qCDebug(LIBKOMPAREDIFF2) << "**** !!! No previous model, ok backup plan activated...";

    // Backup plan
    m_selectedModel = firstModel();
    m_selectedDifference = m_selectedModel->firstDifference();

    Q_EMIT setSelection(m_selectedModel, m_selectedDifference);
    Q_EMIT setStatusBarModelInfo(findModel(m_selectedModel), m_selectedModel->findDifference(m_selectedDifference), modelCount(), differenceCount(), m_selectedModel->appliedCount());
    updateModelListActions();
}

void KompareModelList::slotNextDifference()
{
    qCDebug(LIBKOMPAREDIFF2) << "slotNextDifference called";
    if ((m_selectedDifference = m_selectedModel->nextDifference()) != nullptr)
    {
        Q_EMIT setSelection(m_selectedDifference);
        Q_EMIT setStatusBarModelInfo(findModel(m_selectedModel), m_selectedModel->findDifference(m_selectedDifference), modelCount(), differenceCount(), m_selectedModel->appliedCount());
        updateModelListActions();
        return;
    }

    qCDebug(LIBKOMPAREDIFF2) << "**** no next difference... ok lets find the next model...";

    if ((m_selectedModel = nextModel()) != nullptr)
    {
        m_selectedDifference = m_selectedModel->firstDifference();

        Q_EMIT setSelection(m_selectedModel, m_selectedDifference);
        Q_EMIT setStatusBarModelInfo(findModel(m_selectedModel), m_selectedModel->findDifference(m_selectedDifference), modelCount(), differenceCount(), m_selectedModel->appliedCount());
        updateModelListActions();
        return;
    }

    qCDebug(LIBKOMPAREDIFF2) << "**** !!! No next model, ok backup plan activated...";

    // Backup plan
    m_selectedModel = lastModel();
    m_selectedDifference = m_selectedModel->lastDifference();

    Q_EMIT setSelection(m_selectedModel, m_selectedDifference);
    Q_EMIT setStatusBarModelInfo(findModel(m_selectedModel), m_selectedModel->findDifference(m_selectedDifference), modelCount(), differenceCount(), m_selectedModel->appliedCount());
    updateModelListActions();
}

void KompareModelList::slotApplyDifference(bool apply)
{
    m_selectedModel->applyDifference(apply);
    Q_EMIT applyDifference(apply);
}

void KompareModelList::slotApplyAllDifferences(bool apply)
{
    m_selectedModel->applyAllDifferences(apply);
    Q_EMIT applyAllDifferences(apply);
}

int KompareModelList::parseDiffOutput(const QString& diff)
{
    qCDebug(LIBKOMPAREDIFF2) << "KompareModelList::parseDiffOutput";
    Q_EMIT diffString(diff);

    QStringList diffLines = split(diff);

    Parser* parser = new Parser(this);
    bool malformed = false;
    m_models = parser->parse(diffLines, &malformed);

    m_info->generator = parser->generator();
    m_info->format    = parser->format();

    delete parser;

    if (m_models)
    {
        if (malformed)
        {
            qCDebug(LIBKOMPAREDIFF2) << "Malformed diff";
            Q_EMIT error(i18n("The diff is malformed. Some lines could not be parsed and will not be displayed in the diff view."));
            // proceed anyway with the lines which have been parsed
        }
        m_selectedModel = firstModel();
        qCDebug(LIBKOMPAREDIFF2) << "Ok there are differences...";
        m_selectedDifference = m_selectedModel->firstDifference();
        Q_EMIT setStatusBarModelInfo(0, 0, modelCount(), differenceCount(), 0);
    }
    else
    {
        // Wow trouble, no models, so no differences...
        qCDebug(LIBKOMPAREDIFF2) << "Now i'll be damned, there should be models here !!!";
        return -1;
    }

    return 0;
}

bool KompareModelList::blendOriginalIntoModelList(const QString& localURL)
{
    qCDebug(LIBKOMPAREDIFF2) << "Hurrah we are blending...";
    QFileInfo fi(localURL);

    bool result = false;
    DiffModel* model;

    QString fileContents;

    if (fi.isDir())
    {   // is a dir
        qCDebug(LIBKOMPAREDIFF2) << "Blend Dir";
//      QDir dir( localURL, QString(), QDir::Name|QDir::DirsFirst, QDir::TypeMask );
        DiffModelListIterator modelIt = m_models->begin();
        DiffModelListIterator mEnd    = m_models->end();
        for (; modelIt != mEnd; ++modelIt)
        {
            model = *modelIt;
            qCDebug(LIBKOMPAREDIFF2) << "Model : " << model;
            QString filename = model->source();
            if (!filename.startsWith(localURL))
                filename = QDir(localURL).filePath(filename);
            QFileInfo fi2(filename);
            if (fi2.exists())
            {
                qCDebug(LIBKOMPAREDIFF2) << "Reading from: " << filename;
                fileContents = readFile(filename);
                result = blendFile(model, fileContents);
            }
            else
            {
                qCDebug(LIBKOMPAREDIFF2) << "File " << filename << " does not exist !";
                qCDebug(LIBKOMPAREDIFF2) << "Assume empty file !";
                fileContents.truncate(0);
                result = blendFile(model, fileContents);
            }
        }
        qCDebug(LIBKOMPAREDIFF2) << "End of Blend Dir";
    }
    else if (fi.isFile())
    {   // is a file
        qCDebug(LIBKOMPAREDIFF2) << "Blend File";
        qCDebug(LIBKOMPAREDIFF2) << "Reading from: " << localURL;
        fileContents = readFile(localURL);

        result = blendFile((*m_models)[ 0 ], fileContents);
        qCDebug(LIBKOMPAREDIFF2) << "End of Blend File";
    }

    return result;
}

bool KompareModelList::blendFile(DiffModel* model, const QString& fileContents)
{
    if (!model)
    {
        qCDebug(LIBKOMPAREDIFF2) << "**** model is null :(";
        return false;
    }

    model->setBlended(true);

    int srcLineNo = 1, destLineNo = 1;

    const QStringList lines = split(fileContents);
    auto linesIt = lines.constBegin(), lEnd = lines.constEnd();

    DiffHunkList* hunks = model->hunks();
    qCDebug(LIBKOMPAREDIFF2) << "Hunks in hunklist: " << hunks->count();
    DiffHunkListIterator hunkIt = hunks->begin();

    DiffHunk*   newHunk = nullptr;
    Difference* newDiff = nullptr;

    // FIXME: this approach is not very good, we should first check if the hunk applies cleanly
    // and without offset and if not use that new linenumber with offset to compare against
    // This will only work for files we just diffed with kompare but not for blending where
    // file(s) to patch has/have potentially changed

    for (; hunkIt != hunks->end(); ++hunkIt)
    {
        // Do we need to insert a new hunk before this one ?
        DiffHunk* hunk = *hunkIt;
        if (srcLineNo < hunk->sourceLineNumber())
        {
            newHunk = new DiffHunk(srcLineNo, destLineNo, QString(), DiffHunk::AddedByBlend);

            hunkIt = ++hunks->insert(hunkIt, newHunk);

            newDiff = new Difference(srcLineNo, destLineNo,
                                     Difference::Unchanged);

            newHunk->add(newDiff);

            while (srcLineNo < hunk->sourceLineNumber() && linesIt != lEnd)
            {
                newDiff->addSourceLine(*linesIt);
                newDiff->addDestinationLine(*linesIt);
                ++srcLineNo;
                ++destLineNo;
                ++linesIt;
            }
        }

        // Now we add the linecount difference for the hunk that follows
        int size = hunk->sourceLineCount();

        linesIt += size;
        if (linesIt > lEnd)
        {
            linesIt = lEnd;
        }

        srcLineNo += size;
        destLineNo += hunk->destinationLineCount();
    }

    if (linesIt != lEnd)
    {
        newHunk = new DiffHunk(srcLineNo, destLineNo, QString(), DiffHunk::AddedByBlend);

        model->addHunk(newHunk);

        newDiff = new Difference(srcLineNo, destLineNo, Difference::Unchanged);

        newHunk->add(newDiff);

        while (linesIt != lEnd)
        {
            newDiff->addSourceLine(*linesIt);
            newDiff->addDestinationLine(*linesIt);
            ++linesIt;
        }
    }
#if 0
    DifferenceList hunkDiffList   = (*hunkIt)->differences();
    DifferenceListIterator diffIt = hunkDiffList.begin();
    DifferenceListIterator dEnd   = hunkDiffList.end();
    qCDebug(LIBKOMPAREDIFF2) << "Number of differences in hunkDiffList = " << diffList.count();

    DifferenceListIterator tempIt;
    Difference* diff;

    for (; diffIt != dEnd; ++diffIt)
    {
        diff = *diffIt;
        qCDebug(LIBKOMPAREDIFF2) << "*(Diff it) = " << diff;
        // Check if there are lines in the original file before the difference
        // that are not yet in the diff. If so create new Unchanged diff
        if (srcLineNo < diff->sourceLineNumber())
        {
            newDiff = new Difference(srcLineNo, destLineNo,
                                     Difference::Unchanged | Difference::AddedByBlend);
            newHunk->add(newDiff);
            while (srcLineNo < diff->sourceLineNumber() && linesIt != lEnd)
            {
//                  qCDebug(LIBKOMPAREDIFF2) << "SourceLine = " << srcLineNo << ": " << *linesIt;
                newDiff->addSourceLine(*linesIt);
                newDiff->addDestinationLine(*linesIt);
                ++srcLineNo;
                ++destLineNo;
                ++linesIt;
            }
        }
        // Now i've got to add that diff
        switch (diff->type())
        {
        case Difference::Unchanged:
            qCDebug(LIBKOMPAREDIFF2) << "Unchanged";
            for (int i = 0; i < diff->sourceLineCount(); ++i)
            {
                if (linesIt != lEnd && *linesIt != diff->sourceLineAt(i)->string())
                {
                    qCDebug(LIBKOMPAREDIFF2) << "Conflict: SourceLine = " << srcLineNo << ": " << *linesIt;
                    qCDebug(LIBKOMPAREDIFF2) << "Conflict: DiffLine   = " << diff->sourceLineNumber() + i << ": " << diff->sourceLineAt(i)->string();

                    // Do conflict resolution (well sort of)
                    diff->sourceLineAt(i)->setConflictString(*linesIt);
                    diff->setConflict(true);
                }
//                  qCDebug(LIBKOMPAREDIFF2) << "SourceLine = " << srcLineNo << ": " << *linesIt;
//                  qCDebug(LIBKOMPAREDIFF2) << "DiffLine   = " << diff->sourceLineNumber() + i << ": " << diff->sourceLineAt( i )->string();
                ++srcLineNo;
                ++destLineNo;
                ++linesIt;
            }

            tempIt = diffIt;
            --diffIt;
            diffList.remove(tempIt);
            newHunk->add(diff);

            break;
        case Difference::Change:
            qCDebug(LIBKOMPAREDIFF2) << "Change";

            //QStringListConstIterator saveIt = linesIt;

            for (int i = 0; i < diff->sourceLineCount(); ++i)
            {
                if (linesIt != lEnd && *linesIt != diff->sourceLineAt(i)->string())
                {
                    qCDebug(LIBKOMPAREDIFF2) << "Conflict: SourceLine = " << srcLineNo << ": " << *linesIt;
                    qCDebug(LIBKOMPAREDIFF2) << "Conflict: DiffLine   = " << diff->sourceLineNumber() + i << ": " << diff->sourceLineAt(i)->string();

                    // Do conflict resolution (well sort of)
                    diff->sourceLineAt(i)->setConflictString(*linesIt);
                    diff->setConflict(true);
                }
                ++srcLineNo;
                ++destLineNo;
                ++linesIt;
            }

            destLineNo += diff->destinationLineCount();

            tempIt = diffIt;
            --diffIt;
            diffList.remove(tempIt);
            newHunk->add(diff);
            newModel->addDiff(diff);

            break;
        case Difference::Insert:
            qCDebug(LIBKOMPAREDIFF2) << "Insert";
            destLineNo += diff->destinationLineCount();
            tempIt = diffIt;
            --diffIt;
            diffList.remove(tempIt);
            newHunk->add(diff);
            newModel->addDiff(diff);
            break;
        case Difference::Delete:
            qCDebug(LIBKOMPAREDIFF2) << "Delete";
            qCDebug(LIBKOMPAREDIFF2) << "Number of lines in Delete: " << diff->sourceLineCount();
            for (int i = 0; i < diff->sourceLineCount(); ++i)
            {
                if (linesIt != lEnd && *linesIt != diff->sourceLineAt(i)->string())
                {
                    qCDebug(LIBKOMPAREDIFF2) << "Conflict: SourceLine = " << srcLineNo << ": " << *linesIt;
                    qCDebug(LIBKOMPAREDIFF2) << "Conflict: DiffLine   = " << diff->sourceLineNumber() + i << ": " << diff->sourceLineAt(i)->string();

                    // Do conflict resolution (well sort of)
                    diff->sourceLineAt(i)->setConflictString(*linesIt);
                    diff->setConflict(true);
                }

//                  qCDebug(LIBKOMPAREDIFF2) << "SourceLine = " << srcLineNo << ": " << *it;
//                  qCDebug(LIBKOMPAREDIFF2) << "DiffLine   = " << diff->sourceLineNumber() + i << ": " << diff->sourceLineAt( i )->string();
                ++srcLineNo;
                ++linesIt;
            }

            tempIt = diffIt;
            --diffIt;
            diffList.remove(tempIt);
            newHunk->add(diff);
            newModel->addDiff(diff);
            break;
        default:
            qCDebug(LIBKOMPAREDIFF2) << "****, some diff type we do not know about ???";
        }
    }
}
#endif

/*
    diffList = newModel->differences();

    diff = diffList.first();
    qCDebug(LIBKOMPAREDIFF2) << "Count = " << diffList.count();
    for ( diff = diffList.first(); diff; diff = diffList.next() )
    {
        qCDebug(LIBKOMPAREDIFF2) << "sourcelinenumber = " << diff->sourceLineNumber();
    }
*/

m_selectedModel = firstModel();

m_selectedDifference = m_selectedModel->firstDifference();

return true;
}

void KompareModelList::show()
{
    qCDebug(LIBKOMPAREDIFF2) << "KompareModelList::Show Number of models = " << m_models->count();
    Q_EMIT modelsChanged(m_models);
    Q_EMIT setSelection(m_selectedModel, m_selectedDifference);
}

void KompareModelList::clear()
{
    if (m_models)
        m_models->clear();

    Q_EMIT modelsChanged(m_models);
}

void KompareModelList::refresh()
{
    // FIXME: I can imagine blending also wants to be refreshed so make a switch case here
    compare(m_info->mode);
}

void KompareModelList::swap()
{
    //FIXME Not sure if any mode could be swapped
    if (m_info->mode == Kompare::ComparingFiles)
        compare(m_info->mode);
    else if (m_info->mode == Kompare::ComparingDirs)
        compare(m_info->mode);
}

bool KompareModelList::hasUnsavedChanges() const
{
    if (modelCount() == 0)
        return false;

    DiffModelListConstIterator modelIt = m_models->constBegin();
    DiffModelListConstIterator endIt   = m_models->constEnd();

    for (; modelIt != endIt; ++modelIt)
    {
        if ((*modelIt)->hasUnsavedChanges())
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

void KompareModelList::slotKompareInfo(struct Kompare::Info* info)
{
    m_info = info;
}

bool KompareModelList::setSelectedModel(DiffModel* model)
{
    qCDebug(LIBKOMPAREDIFF2) << "KompareModelList::setSelectedModel( " << model << " )";

    if (model != m_selectedModel)
    {
        if (!m_models->contains(model))
            return false;
        qCDebug(LIBKOMPAREDIFF2) << "m_selectedModel (was) = " << m_selectedModel;
        m_modelIndex = m_models->indexOf(model);
        qCDebug(LIBKOMPAREDIFF2) << "m_selectedModel (is)  = " << m_selectedModel;
        m_selectedModel = model;
    }

    updateModelListActions();

    return true;
}

void KompareModelList::updateModelListActions()
{
    if (m_models && m_selectedModel && m_selectedDifference)
    {
        if (m_isReadWrite && m_save)
        {
            if (m_selectedModel->appliedCount() != m_selectedModel->differenceCount())
                m_applyAll->setEnabled(true);
            else
                m_applyAll->setEnabled(false);

            if (m_selectedModel->appliedCount() != 0)
                m_unapplyAll->setEnabled(true);
            else
                m_unapplyAll->setEnabled(false);

            m_applyDifference->setEnabled(m_selectedDifference->applied() == false);
            m_unApplyDifference->setEnabled(m_selectedDifference->applied() == true);
            m_save->setEnabled(m_selectedModel->hasUnsavedChanges());
        }
        else if (m_save)
        {
            m_applyDifference->setEnabled(false);
            m_unApplyDifference->setEnabled(false);
            m_applyAll->setEnabled(false);
            m_unapplyAll->setEnabled(false);
            m_save->setEnabled(false);
        }

        m_previousFile->setEnabled(hasPrevModel());
        m_nextFile->setEnabled(hasNextModel());
        m_previousDifference->setEnabled(hasPrevDiff());
        m_nextDifference->setEnabled(hasNextDiff());
    }
    else
    {
        if (m_save) {
            m_applyDifference->setEnabled(false);
            m_unApplyDifference->setEnabled(false);
            m_applyAll->setEnabled(false);
            m_unapplyAll->setEnabled(false);
            m_save->setEnabled(false);
        }

        m_previousFile->setEnabled(false);
        m_nextFile->setEnabled(false);
        m_previousDifference->setEnabled(false);
        m_nextDifference->setEnabled(false);
    }
}

bool KompareModelList::hasPrevModel() const
{
    qCDebug(LIBKOMPAREDIFF2) << "KompareModelList::hasPrevModel()";

    if (m_modelIndex > 0)
    {
//         qCDebug(LIBKOMPAREDIFF2) << "has prev model";
        return true;
    }

//     qCDebug(LIBKOMPAREDIFF2) << "doesn't have a prev model, this is the first one...";

    return false;
}

bool KompareModelList::hasNextModel() const
{
    qCDebug(LIBKOMPAREDIFF2) << "KompareModelList::hasNextModel()";

    if (m_modelIndex < (m_models->count() - 1))
    {
//         qCDebug(LIBKOMPAREDIFF2) << "has next model";
        return true;
    }

//     qCDebug(LIBKOMPAREDIFF2) << "doesn't have a next model, this is the last one...";
    return false;
}

bool KompareModelList::hasPrevDiff() const
{
//     qCDebug(LIBKOMPAREDIFF2) << "KompareModelList::hasPrevDiff()";
    int index = m_selectedModel->diffIndex();

    if (index > 0)
    {
//         qCDebug(LIBKOMPAREDIFF2) << "has prev difference in same model";
        return true;
    }

    if (hasPrevModel())
    {
//         qCDebug(LIBKOMPAREDIFF2) << "has prev difference but in prev model";
        return true;
    }

//     qCDebug(LIBKOMPAREDIFF2) << "doesn't have a prev difference, not even in the previous model because there is no previous model";

    return false;
}

bool KompareModelList::hasNextDiff() const
{
//     qCDebug(LIBKOMPAREDIFF2) << "KompareModelList::hasNextDiff()";
    int index = m_selectedModel->diffIndex();

    if (index < (m_selectedModel->differenceCount() - 1))
    {
//         qCDebug(LIBKOMPAREDIFF2) << "has next difference in same model";
        return true;
    }

    if (hasNextModel())
    {
//         qCDebug(LIBKOMPAREDIFF2) << "has next difference but in next model";
        return true;
    }

//     qCDebug(LIBKOMPAREDIFF2) << "doesn't have a next difference, not even in next model because there is no next model";

    return false;
}

void KompareModelList::slotActionApplyDifference()
{
    if (!m_selectedDifference->applied())
        slotApplyDifference(true);
    slotNextDifference();
    updateModelListActions();
}

void KompareModelList::slotActionUnApplyDifference()
{
    if (m_selectedDifference->applied())
        slotApplyDifference(false);
    slotPreviousDifference();
    updateModelListActions();
}

void KompareModelList::slotActionApplyAllDifferences()
{
    slotApplyAllDifferences(true);
    updateModelListActions();
}

void KompareModelList::slotActionUnapplyAllDifferences()
{
    slotApplyAllDifferences(false);
    updateModelListActions();
}


/* vim: set ts=4 sw=4 noet: */

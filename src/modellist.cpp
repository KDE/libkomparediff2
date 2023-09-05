/*
    SPDX-FileCopyrightText: 2001-2005,2009 Otto Bruggeman <otto.bruggeman@home.nl>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2007-2010 Kevin Kofler <kevin.kofler@chello.at>
    SPDX-FileCopyrightText: 2012 Jean -Nicolas Artaud <jeannicolasartaud@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "modellist.h"
#include "modellist_p.h"

// lib
#include "diffhunk.h"
#include "parser.h"
#include <komparediff2_logging.h>
// KF
#include <KActionCollection>
#include <KDirWatch>
#include <KIO/FileCopyJob>
#include <KIO/MkdirJob>
#include <KIO/StatJob>
#include <KIO/UDSEntry>
#include <KLocalizedString>
#include <KStandardAction>
// Qt
#include <QDir>
#include <QFile>
#include <QList>
#include <QMimeDatabase>
#include <QMimeType>
#include <QTextCodec>
#include <QTextStream>
// Std
#include <algorithm>

using namespace KompareDiff2;

ModelList::ModelList(DiffSettings *diffSettings, QObject *parent, bool supportReadWrite)
    : QObject(parent)
    , d_ptr(new ModelListPrivate(diffSettings, supportReadWrite))
{
    Q_D(ModelList);

    qCDebug(KOMPAREDIFF2_LOG) << "Show me the arguments: " << diffSettings << ", " << parent;
    d->actionCollection = new KActionCollection(this);
    if (supportReadWrite) {
        d->applyDifference = d->actionCollection->addAction(QStringLiteral("difference_apply"), this, &ModelList::slotActionApplyDifference);
        d->applyDifference->setIcon(QIcon::fromTheme(QStringLiteral("arrow-right")));
        d->applyDifference->setText(i18nc("@action", "&Apply Difference"));
        d->actionCollection->setDefaultShortcut(d->applyDifference, QKeySequence(Qt::Key_Space));
        d->unApplyDifference = d->actionCollection->addAction(QStringLiteral("difference_unapply"), this, &ModelList::slotActionUnApplyDifference);
        d->unApplyDifference->setIcon(QIcon::fromTheme(QStringLiteral("arrow-left")));
        d->unApplyDifference->setText(i18nc("@action", "Un&apply Difference"));
        d->actionCollection->setDefaultShortcut(d->unApplyDifference, QKeySequence(Qt::Key_Backspace));
        d->applyAll = d->actionCollection->addAction(QStringLiteral("difference_applyall"), this, &ModelList::slotActionApplyAllDifferences);
        d->applyAll->setIcon(QIcon::fromTheme(QStringLiteral("arrow-right-double")));
        d->applyAll->setText(i18nc("@action", "App&ly All"));
        d->actionCollection->setDefaultShortcut(d->applyAll, QKeySequence(Qt::CTRL | Qt::Key_A));
        d->unapplyAll = d->actionCollection->addAction(QStringLiteral("difference_unapplyall"), this, &ModelList::slotActionUnapplyAllDifferences);
        d->unapplyAll->setIcon(QIcon::fromTheme(QStringLiteral("arrow-left-double")));
        d->unapplyAll->setText(i18nc("@action", "&Unapply All"));
        d->actionCollection->setDefaultShortcut(d->unapplyAll, QKeySequence(Qt::CTRL | Qt::Key_U));
    } else {
        d->applyDifference = nullptr;
        d->unApplyDifference = nullptr;
        d->applyAll = nullptr;
        d->unapplyAll = nullptr;
    }
    d->previousFile = d->actionCollection->addAction(QStringLiteral("difference_previousfile"), this, &ModelList::slotPreviousModel);
    d->previousFile->setIcon(QIcon::fromTheme(QStringLiteral("arrow-up-double")));
    d->previousFile->setText(i18nc("@action", "P&revious File"));
    d->actionCollection->setDefaultShortcut(d->previousFile, QKeySequence(Qt::CTRL | Qt::Key_PageUp));
    d->nextFile = d->actionCollection->addAction(QStringLiteral("difference_nextfile"), this, &ModelList::slotNextModel);
    d->nextFile->setIcon(QIcon::fromTheme(QStringLiteral("arrow-down-double")));
    d->nextFile->setText(i18nc("@action", "N&ext File"));
    d->actionCollection->setDefaultShortcut(d->nextFile, QKeySequence(Qt::CTRL | Qt::Key_PageDown));
    d->previousDifference = d->actionCollection->addAction(QStringLiteral("difference_previous"), this, &ModelList::slotPreviousDifference);
    d->previousDifference->setIcon(QIcon::fromTheme(QStringLiteral("arrow-up")));
    d->previousDifference->setText(i18nc("@action", "&Previous Difference"));
    d->actionCollection->setDefaultShortcut(d->previousDifference, QKeySequence(Qt::CTRL | Qt::Key_Up));
    d->nextDifference = d->actionCollection->addAction(QStringLiteral("difference_next"), this, &ModelList::slotNextDifference);
    d->nextDifference->setIcon(QIcon::fromTheme(QStringLiteral("arrow-down")));
    d->nextDifference->setText(i18nc("@action", "&Next Difference"));
    d->actionCollection->setDefaultShortcut(d->nextDifference, QKeySequence(Qt::CTRL | Qt::Key_Down));
    d->previousDifference->setEnabled(false);
    d->nextDifference->setEnabled(false);

    if (supportReadWrite) {
        d->save = KStandardAction::save(this, &ModelList::slotSaveDestination, d->actionCollection);
        d->save->setEnabled(false);
    } else {
        d->save = nullptr;
    }

    d->updateModelListActions();
}

ModelList::~ModelList() = default;

bool ModelList::compare()
{
    Q_D(ModelList);

    bool result = false;

    bool sourceIsDirectory = ModelListPrivate::isDirectory(d->info->localSource);
    bool destinationIsDirectory = ModelListPrivate::isDirectory(d->info->localDestination);

    if (sourceIsDirectory && destinationIsDirectory) {
        d->info->mode = ComparingDirs;
        result = compare(d->info->mode);
    } else if (!sourceIsDirectory && !destinationIsDirectory) {
        QFile sourceFile(d->info->localSource);
        sourceFile.open(QIODevice::ReadOnly);
        QMimeDatabase db;
        QString sourceMimeType = (db.mimeTypeForData(sourceFile.readAll())).name();
        sourceFile.close();
        qCDebug(KOMPAREDIFF2_LOG) << "Mimetype source     : " << sourceMimeType;

        QFile destinationFile(d->info->localDestination);
        destinationFile.open(QIODevice::ReadOnly);
        QString destinationMimeType = (db.mimeTypeForData(destinationFile.readAll())).name();
        destinationFile.close();
        qCDebug(KOMPAREDIFF2_LOG) << "Mimetype destination: " << destinationMimeType;

        // Not checking if it is a text file/something diff can even compare, we'll let diff handle that
        if (!ModelListPrivate::isDiff(sourceMimeType) && ModelListPrivate::isDiff(destinationMimeType)) {
            qCDebug(KOMPAREDIFF2_LOG) << "Blending destination into source...";
            d->info->mode = BlendingFile;
            result = openFileAndDiff();
        } else if (ModelListPrivate::isDiff(sourceMimeType) && !ModelListPrivate::isDiff(destinationMimeType)) {
            qCDebug(KOMPAREDIFF2_LOG) << "Blending source into destination...";
            d->info->mode = BlendingFile;
            // Swap source and destination before calling this
            d->info->swapSourceWithDestination();
            // Do we need to notify anyone we swapped source and destination?
            // No we do not need to notify anyone about swapping source with destination
            result = openFileAndDiff();
        } else {
            qCDebug(KOMPAREDIFF2_LOG) << "Comparing source with destination";
            d->info->mode = ComparingFiles;
            result = compare(d->info->mode);
        }
    } else if (sourceIsDirectory && !destinationIsDirectory) {
        d->info->mode = BlendingDir;
        result = openDirAndDiff();
    } else {
        d->info->mode = BlendingDir;
        // Swap source and destination first in d->info
        d->info->swapSourceWithDestination();
        // Do we need to notify anyone we swapped source and destination?
        // No we do not need to notify anyone about swapping source with destination
        result = openDirAndDiff();
    }

    return result;
}

bool ModelList::compare(Mode mode)
{
    Q_D(ModelList);

    clear(); // Destroy the old models...

    d->diffProcess = std::make_unique<KompareProcess>(d->diffSettings, Custom, d->info->localSource, d->info->localDestination, QString(), mode);
    d->diffProcess->setEncoding(d->encoding);

    connect(d->diffProcess.get(), &KompareProcess::diffHasFinished, this, &ModelList::slotDiffProcessFinished);

    Q_EMIT status(RunningDiff);
    d->diffProcess->start();

    return true;
}

bool ModelList::openFileAndDiff()
{
    Q_D(ModelList);

    clear();

    if (parseDiffOutput(d->readFile(d->info->localDestination)) != 0) {
        Q_EMIT error(i18n("<qt>No models or no differences, this file: <b>%1</b>, is not a valid diff file.</qt>", d->info->destination.url()));
        return false;
    }

    d->setDepthAndApplied();

    if (!blendOriginalIntoModelList(d->info->localSource)) {
        qCDebug(KOMPAREDIFF2_LOG) << "Oops cant blend original file into modellist : " << d->info->localSource;
        Q_EMIT error(
            i18n("<qt>There were problems applying the diff <b>%1</b> to the file <b>%2</b>.</qt>", d->info->destination.url(), d->info->source.url()));
        return false;
    }

    d->updateModelListActions();
    show();

    return true;
}

bool ModelList::openDirAndDiff()
{
    Q_D(ModelList);

    clear();

    if (parseDiffOutput(d->readFile(d->info->localDestination)) != 0) {
        Q_EMIT error(i18n("<qt>No models or no differences, this file: <b>%1</b>, is not a valid diff file.</qt>", d->info->destination.url()));
        return false;
    }

    d->setDepthAndApplied();

    // Do our thing :)
    if (!blendOriginalIntoModelList(d->info->localSource)) {
        // Trouble blending the original into the model
        qCDebug(KOMPAREDIFF2_LOG) << "Oops cant blend original dir into modellist : " << d->info->localSource;
        Q_EMIT error(
            i18n("<qt>There were problems applying the diff <b>%1</b> to the folder <b>%2</b>.</qt>", d->info->destination.url(), d->info->source.url()));
        return false;
    }

    d->updateModelListActions();
    show();

    return true;
}

void ModelList::slotSaveDestination()
{
    Q_D(ModelList);

    // Unnecessary safety check! We can now guarantee that saving is only possible when there is a model and there are unsaved changes
    if (d->selectedModel) {
        saveDestination(d->selectedModel);
        if (d->save)
            d->save->setEnabled(false);
        Q_EMIT updateActions();
    }
}

bool ModelList::saveDestination(DiffModel *model)
{
    Q_D(ModelList);

    qCDebug(KOMPAREDIFF2_LOG) << "ModelList::saveDestination: ";

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

    const DiffHunkList *hunks = model->hunks();
    DiffHunkListConstIterator hunkIt = hunks->constBegin();
    DiffHunkListConstIterator hEnd = hunks->constEnd();

    for (; hunkIt != hEnd; ++hunkIt) {
        DiffHunk *hunk = *hunkIt;

        const DifferenceList differences = hunk->differences();
        DifferenceListConstIterator diffIt = differences.constBegin();
        DifferenceListConstIterator dEnd = differences.constEnd();

        Difference *diff;
        for (; diffIt != dEnd; ++diffIt) {
            diff = *diffIt;
            if (!diff->applied()) {
                const DifferenceStringList destinationLines = diff->destinationLines();
                DifferenceStringListConstIterator stringIt = destinationLines.begin();
                DifferenceStringListConstIterator sEnd = destinationLines.end();
                for (; stringIt != sEnd; ++stringIt) {
                    list.append((*stringIt)->string());
                }
            } else {
                const DifferenceStringList sourceLines = diff->sourceLines();
                DifferenceStringListConstIterator stringIt = sourceLines.begin();
                DifferenceStringListConstIterator sEnd = sourceLines.end();
                for (; stringIt != sEnd; ++stringIt) {
                    list.append((*stringIt)->string());
                }
            }
        }
    }

    // qCDebug(KOMPAREDIFF2_LOG) << "Everything: " << endl << list.join( "\n" );

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
    if (d->info->mode == ComparingDirs) {
        // Don't use destination which was used for creating the diff directly, use the original URL!!!
        // FIXME!!! Wrong destination this way! Need to add the sub directory to the url!!!!
        qCDebug(KOMPAREDIFF2_LOG) << "Tempfilename (save) : " << temp.fileName();
        qCDebug(KOMPAREDIFF2_LOG) << "Model->path+file    : " << model->destinationPath() << model->destinationFile();
        qCDebug(KOMPAREDIFF2_LOG) << "info->localdest     : " << d->info->localDestination;
        QString tmp = model->destinationPath();
        if (tmp.startsWith(d->info->localDestination)) // It should, if not serious trouble...
            tmp.remove(0, d->info->localDestination.size());
        qCDebug(KOMPAREDIFF2_LOG) << "DestinationURL      : " << d->info->destination;
        qCDebug(KOMPAREDIFF2_LOG) << "tmp                 : " << tmp;
        KIO::UDSEntry entry;
        QUrl fullDestinationPath = d->info->destination;
        fullDestinationPath.setPath(fullDestinationPath.path() + tmp);
        qCDebug(KOMPAREDIFF2_LOG) << "fullDestinationPath : " << fullDestinationPath;
        KIO::StatJob *statJob = KIO::stat(fullDestinationPath);
        if (!statJob->exec()) {
            entry = statJob->statResult();
            KIO::MkdirJob *mkdirJob = KIO::mkdir(fullDestinationPath);
            if (!mkdirJob->exec()) {
                Q_EMIT error(i18n("<qt>Could not create destination directory <b>%1</b>.\nThe file has not been saved.</qt>", fullDestinationPath.path()));
                return false;
            }
        }
        fullDestinationPath.setPath(fullDestinationPath.path() + model->destinationFile());
        KIO::FileCopyJob *copyJob = KIO::file_copy(QUrl::fromLocalFile(temp.fileName()), fullDestinationPath, -1, KIO::Overwrite);
        result = copyJob->exec();
    } else {
        qCDebug(KOMPAREDIFF2_LOG) << "Tempfilename   : " << temp.fileName();
        qCDebug(KOMPAREDIFF2_LOG) << "DestinationURL : " << d->info->destination;

        // Get permissions of existing file and copy temporary file with the same permissions
        int permissions = -1;
        KIO::StatJob *statJob = KIO::stat(d->info->destination);
        result = statJob->exec();
        if (result)
            permissions = statJob->statResult().numberValue(KIO::UDSEntry::UDS_ACCESS);

        KIO::FileCopyJob *copyJob = KIO::file_copy(QUrl::fromLocalFile(temp.fileName()), d->info->destination, permissions, KIO::Overwrite);
        result = copyJob->exec();
        qCDebug(KOMPAREDIFF2_LOG) << "true or false?" << result;
    }

    if (!result) {
        // FIXME: Wrong first argument given in case of comparing directories!
        Q_EMIT error(
            i18n("<qt>Could not upload the temporary file to the destination location <b>%1</b>. The temporary file is still available under: <b>%2</b>. You "
                 "can manually copy it to the right place.</qt>",
                 d->info->destination.url(),
                 temp.fileName()));
        // Don't remove file when we delete temp and don't leak it.
        temp.setAutoRemove(false);
    } else {
        temp.remove();
    }

    // If saving was fine set all differences to saved so we can start again with a clean slate
    if (result) {
        const DifferenceList *differences = model->differences();
        DifferenceListConstIterator diffIt = differences->constBegin();
        DifferenceListConstIterator endIt = differences->constEnd();

        for (; diffIt != endIt; ++diffIt) {
            (*diffIt)->setUnsaved(false);
        }
    }

    return true;
}

bool ModelList::saveAll()
{
    Q_D(ModelList);

    if (modelCount() == 0)
        return false;

    DiffModelListIterator it = d->models->begin();
    DiffModelListIterator end = d->models->end();
    for (; it != end; ++it) {
        if (!saveDestination(*it))
            return false;
    }

    return true;
}

void ModelList::setEncoding(const QString &encoding)
{
    Q_D(ModelList);

    d->encoding = encoding;
    if (!encoding.compare(QLatin1String("default"), Qt::CaseInsensitive)) {
        d->textCodec = QTextCodec::codecForLocale();
    } else {
        qCDebug(KOMPAREDIFF2_LOG) << "Encoding : " << encoding;
        d->textCodec = QTextCodec::codecForName(encoding.toUtf8());
        qCDebug(KOMPAREDIFF2_LOG) << "TextCodec: " << d->textCodec;
        if (!d->textCodec)
            d->textCodec = QTextCodec::codecForLocale();
    }
    qCDebug(KOMPAREDIFF2_LOG) << "TextCodec: " << d->textCodec;
}

void ModelList::setReadWrite(bool isReadWrite)
{
    Q_D(ModelList);

    if (d->isReadWrite == isReadWrite)
        return;

    d->isReadWrite = isReadWrite;
    d->updateModelListActions();
}

bool ModelList::isReadWrite() const
{
    Q_D(const ModelList);

    return d->isReadWrite;
}

void ModelList::slotDiffProcessFinished(bool success)
{
    Q_D(ModelList);

    if (success) {
        Q_EMIT status(Parsing);
        if (parseDiffOutput(d->diffProcess->diffOutput()) != 0) {
            Q_EMIT error(i18n("Could not parse diff output."));
        } else {
            if (d->info->mode != ShowingDiff) {
                qCDebug(KOMPAREDIFF2_LOG) << "Blend this crap please and do not give me any conflicts...";
                blendOriginalIntoModelList(d->info->localSource);
            }
            d->updateModelListActions();
            show();
        }
        Q_EMIT status(FinishedParsing);
    } else if (d->diffProcess->exitStatus() == 0) {
        Q_EMIT error(i18n("The files are identical."));
    } else {
        Q_EMIT error(d->diffProcess->stdErr());
    }

    // delay deletion, see bug 182792
    d->diffProcess.release()->deleteLater();
}

bool ModelList::openDiff(const QString &diffFile)
{
    Q_D(ModelList);

    qCDebug(KOMPAREDIFF2_LOG) << "Stupid :) Url = " << diffFile;

    if (diffFile.isEmpty())
        return false;

    QString diff = d->readFile(diffFile);

    clear(); // Clear the current models

    Q_EMIT status(Parsing);

    if (parseDiffOutput(diff) != 0) {
        Q_EMIT error(i18n("Could not parse diff output."));
        return false;
    }

    d->updateModelListActions();
    show();

    Q_EMIT status(FinishedParsing);

    return true;
}

bool ModelList::parseAndOpenDiff(const QString &diff)
{
    Q_D(ModelList);

    clear(); // Clear the current models

    Q_EMIT status(Parsing);

    if (parseDiffOutput(diff) != 0) {
        Q_EMIT error(i18n("Could not parse diff output."));
        return false;
    }

    d->updateModelListActions();
    show();

    Q_EMIT status(FinishedParsing);
    return true;
}

QString ModelList::recreateDiff() const
{
    Q_D(const ModelList);

    QString diff;

    DiffModelListConstIterator modelIt = d->models->constBegin();
    DiffModelListConstIterator mEnd = d->models->constEnd();

    for (; modelIt != mEnd; ++modelIt) {
        diff += (*modelIt)->recreateDiff();
    }
    return diff;
}

bool ModelList::saveDiff(const QString &url, const QString &directory, DiffSettings *diffSettings)
{
    Q_D(ModelList);

    qCDebug(KOMPAREDIFF2_LOG) << "ModelList::saveDiff: ";

    d->diffTemp = std::make_unique<QTemporaryFile>();
    d->diffURL = QUrl(url); // ### TODO the "url" argument should be a QUrl

    if (!d->diffTemp->open()) {
        Q_EMIT error(i18n("Could not open a temporary file."));
        d->diffTemp->remove();
        d->diffTemp.reset();
        return false;
    }

    d->diffProcess = std::make_unique<KompareProcess>(diffSettings, Custom, d->info->localSource, d->info->localDestination, directory);
    d->diffProcess->setEncoding(d->encoding);

    connect(d->diffProcess.get(), &KompareProcess::diffHasFinished, this, &ModelList::slotWriteDiffOutput);

    Q_EMIT status(RunningDiff);
    d->diffProcess->start();
    return true;
}

void ModelList::slotWriteDiffOutput(bool success)
{
    Q_D(ModelList);

    qCDebug(KOMPAREDIFF2_LOG) << "Success = " << success;

    if (success) {
        QTextStream stream(d->diffTemp.get());

        stream << d->diffProcess->diffOutput();

        d->diffTemp->close();

        if (false /*|| d->diffTemp->status() != 0 */) {
            Q_EMIT error(i18n("Could not write to the temporary file."));
        }

        KIO::FileCopyJob *copyJob = KIO::file_copy(QUrl::fromLocalFile(d->diffTemp->fileName()), d->diffURL);
        copyJob->exec();

        Q_EMIT status(FinishedWritingDiff);
    }

    d->diffURL = QUrl();
    d->diffTemp->remove();
    d->diffTemp.reset();

    d->diffProcess.reset();
}

void ModelList::slotSelectionChanged(const KompareDiff2::DiffModel *model, const KompareDiff2::Difference *diff)
{
    Q_D(ModelList);

    // This method will signal all the other objects about a change in selection,
    // it will emit setSelection( const DiffModel*, const Difference* ) to all who are connected
    qCDebug(KOMPAREDIFF2_LOG) << "ModelList::slotSelectionChanged( " << model << ", " << diff << " )";
    qCDebug(KOMPAREDIFF2_LOG) << "Sender is : " << sender()->metaObject()->className();
//     qCDebug(KOMPAREDIFF2_LOG) << kBacktrace();

    d->selectedModel = const_cast<DiffModel *>(model);
    d->modelIndex = d->models->indexOf(d->selectedModel);
    qCDebug(KOMPAREDIFF2_LOG) << "d->modelIndex = " << d->modelIndex;
    d->selectedDifference = const_cast<Difference *>(diff);

    d->selectedModel->setSelectedDifference(d->selectedDifference);

    // setSelected* search for the argument in the lists and return false if not found
    // if found they return true and set the d->selected*
    if (!d->setSelectedModel(d->selectedModel)) {
        // Backup plan
        d->selectedModel = d->firstModel();
        d->selectedDifference = d->selectedModel->firstDifference();
    } else if (!d->selectedModel->setSelectedDifference(d->selectedDifference)) {
        // Another backup plan
        d->selectedDifference = d->selectedModel->firstDifference();
    }

    Q_EMIT setSelection(model, diff);
    Q_EMIT setStatusBarModelInfo(findModel(d->selectedModel),
                                 d->selectedModel->findDifference(d->selectedDifference),
                                 modelCount(),
                                 differenceCount(),
                                 d->selectedModel->appliedCount());

    d->updateModelListActions();
}

void ModelList::slotSelectionChanged(const KompareDiff2::Difference *diff)
{
    Q_D(ModelList);

    // This method will emit setSelection( const Difference* ) to whomever is listening
    // when for instance in kompareview the selection has changed
    qCDebug(KOMPAREDIFF2_LOG) << "ModelList::slotSelectionChanged( " << diff << " )";
    qCDebug(KOMPAREDIFF2_LOG) << "Sender is : " << sender()->metaObject()->className();

    d->selectedDifference = const_cast<Difference *>(diff);

    if (!d->selectedModel->setSelectedDifference(d->selectedDifference)) {
        // Backup plan
        d->selectedDifference = d->selectedModel->firstDifference();
    }

    Q_EMIT setSelection(diff);
    Q_EMIT setStatusBarModelInfo(findModel(d->selectedModel),
                                 d->selectedModel->findDifference(d->selectedDifference),
                                 modelCount(),
                                 differenceCount(),
                                 d->selectedModel->appliedCount());

    d->updateModelListActions();
}

void ModelList::slotPreviousModel()
{
    Q_D(ModelList);

    if ((d->selectedModel = d->prevModel()) != nullptr) {
        d->selectedDifference = d->selectedModel->firstDifference();
    } else {
        d->selectedModel = d->firstModel();
        d->selectedDifference = d->selectedModel->firstDifference();
    }

    Q_EMIT setSelection(d->selectedModel, d->selectedDifference);
    Q_EMIT setStatusBarModelInfo(findModel(d->selectedModel),
                                 d->selectedModel->findDifference(d->selectedDifference),
                                 modelCount(),
                                 differenceCount(),
                                 d->selectedModel->appliedCount());
    d->updateModelListActions();
}

void ModelList::slotNextModel()
{
    Q_D(ModelList);

    if ((d->selectedModel = d->nextModel()) != nullptr) {
        d->selectedDifference = d->selectedModel->firstDifference();
    } else {
        d->selectedModel = d->lastModel();
        d->selectedDifference = d->selectedModel->firstDifference();
    }

    Q_EMIT setSelection(d->selectedModel, d->selectedDifference);
    Q_EMIT setStatusBarModelInfo(findModel(d->selectedModel),
                                 d->selectedModel->findDifference(d->selectedDifference),
                                 modelCount(),
                                 differenceCount(),
                                 d->selectedModel->appliedCount());
    d->updateModelListActions();
}

Mode ModelList::mode() const
{
    Q_D(const ModelList);

    return d->info->mode;
}

const DiffModelList *ModelList::models() const
{
    Q_D(const ModelList);

    return d->models.get();
}

KActionCollection *ModelList::actionCollection() const
{
    Q_D(const ModelList);

    return d->actionCollection;
}

void ModelList::slotPreviousDifference()
{
    Q_D(ModelList);

    qCDebug(KOMPAREDIFF2_LOG) << "slotPreviousDifference called";
    if ((d->selectedDifference = d->selectedModel->prevDifference()) != nullptr) {
        Q_EMIT setSelection(d->selectedDifference);
        Q_EMIT setStatusBarModelInfo(findModel(d->selectedModel),
                                     d->selectedModel->findDifference(d->selectedDifference),
                                     modelCount(),
                                     differenceCount(),
                                     d->selectedModel->appliedCount());
        d->updateModelListActions();
        return;
    }

    qCDebug(KOMPAREDIFF2_LOG) << "**** no previous difference... ok lets find the previous model...";

    if ((d->selectedModel = d->prevModel()) != nullptr) {
        d->selectedDifference = d->selectedModel->lastDifference();

        Q_EMIT setSelection(d->selectedModel, d->selectedDifference);
        Q_EMIT setStatusBarModelInfo(findModel(d->selectedModel),
                                     d->selectedModel->findDifference(d->selectedDifference),
                                     modelCount(),
                                     differenceCount(),
                                     d->selectedModel->appliedCount());
        d->updateModelListActions();
        return;
    }

    qCDebug(KOMPAREDIFF2_LOG) << "**** !!! No previous model, ok backup plan activated...";

    // Backup plan
    d->selectedModel = d->firstModel();
    d->selectedDifference = d->selectedModel->firstDifference();

    Q_EMIT setSelection(d->selectedModel, d->selectedDifference);
    Q_EMIT setStatusBarModelInfo(findModel(d->selectedModel),
                                 d->selectedModel->findDifference(d->selectedDifference),
                                 modelCount(),
                                 differenceCount(),
                                 d->selectedModel->appliedCount());
    d->updateModelListActions();
}

void ModelList::slotNextDifference()
{
    Q_D(ModelList);

    qCDebug(KOMPAREDIFF2_LOG) << "slotNextDifference called";
    if ((d->selectedDifference = d->selectedModel->nextDifference()) != nullptr) {
        Q_EMIT setSelection(d->selectedDifference);
        Q_EMIT setStatusBarModelInfo(findModel(d->selectedModel),
                                     d->selectedModel->findDifference(d->selectedDifference),
                                     modelCount(),
                                     differenceCount(),
                                     d->selectedModel->appliedCount());
        d->updateModelListActions();
        return;
    }

    qCDebug(KOMPAREDIFF2_LOG) << "**** no next difference... ok lets find the next model...";

    if ((d->selectedModel = d->nextModel()) != nullptr) {
        d->selectedDifference = d->selectedModel->firstDifference();

        Q_EMIT setSelection(d->selectedModel, d->selectedDifference);
        Q_EMIT setStatusBarModelInfo(findModel(d->selectedModel),
                                     d->selectedModel->findDifference(d->selectedDifference),
                                     modelCount(),
                                     differenceCount(),
                                     d->selectedModel->appliedCount());
        d->updateModelListActions();
        return;
    }

    qCDebug(KOMPAREDIFF2_LOG) << "**** !!! No next model, ok backup plan activated...";

    // Backup plan
    d->selectedModel = d->lastModel();
    d->selectedDifference = d->selectedModel->lastDifference();

    Q_EMIT setSelection(d->selectedModel, d->selectedDifference);
    Q_EMIT setStatusBarModelInfo(findModel(d->selectedModel),
                                 d->selectedModel->findDifference(d->selectedDifference),
                                 modelCount(),
                                 differenceCount(),
                                 d->selectedModel->appliedCount());
    d->updateModelListActions();
}

void ModelList::slotApplyDifference(bool apply)
{
    Q_D(ModelList);

    d->selectedModel->applyDifference(apply);
    Q_EMIT applyDifference(apply);
}

void ModelList::slotApplyAllDifferences(bool apply)
{
    Q_D(ModelList);

    d->selectedModel->applyAllDifferences(apply);
    Q_EMIT applyAllDifferences(apply);
}

int ModelList::parseDiffOutput(const QString &diff)
{
    Q_D(ModelList);

    qCDebug(KOMPAREDIFF2_LOG) << "ModelList::parseDiffOutput";
    Q_EMIT diffString(diff);

    QStringList diffLines = ModelListPrivate::split(diff);

    std::unique_ptr<Parser> parser = std::make_unique<Parser>(this);
    bool malformed = false;
    d->models.reset(parser->parse(diffLines, &malformed));

    d->info->generator = parser->generator();
    d->info->format = parser->format();

    parser.reset();

    if (d->models) {
        if (malformed) {
            qCDebug(KOMPAREDIFF2_LOG) << "Malformed diff";
            Q_EMIT error(i18n("The diff is malformed. Some lines could not be parsed and will not be displayed in the diff view."));
            // proceed anyway with the lines which have been parsed
        }
        d->selectedModel = d->firstModel();
        qCDebug(KOMPAREDIFF2_LOG) << "Ok there are differences...";
        d->selectedDifference = d->selectedModel->firstDifference();
        Q_EMIT setStatusBarModelInfo(0, 0, modelCount(), differenceCount(), 0);
    } else {
        // Wow trouble, no models, so no differences...
        qCDebug(KOMPAREDIFF2_LOG) << "Now i'll be damned, there should be models here !!!";
        return -1;
    }

    return 0;
}

bool ModelList::blendOriginalIntoModelList(const QString &localURL)
{
    Q_D(ModelList);

    qCDebug(KOMPAREDIFF2_LOG) << "Hurrah we are blending...";
    QFileInfo fi(localURL);

    bool result = false;
    DiffModel *model;

    QString fileContents;

    if (fi.isDir()) { // is a dir
        qCDebug(KOMPAREDIFF2_LOG) << "Blend Dir";
//      QDir dir( localURL, QString(), QDir::Name|QDir::DirsFirst, QDir::TypeMask );
        DiffModelListIterator modelIt = d->models->begin();
        DiffModelListIterator mEnd = d->models->end();
        for (; modelIt != mEnd; ++modelIt) {
            model = *modelIt;
            qCDebug(KOMPAREDIFF2_LOG) << "Model : " << model;
            QString filename = model->source();
            if (!filename.startsWith(localURL))
                filename = QDir(localURL).filePath(filename);
            QFileInfo fi2(filename);
            if (fi2.exists()) {
                qCDebug(KOMPAREDIFF2_LOG) << "Reading from: " << filename;
                fileContents = d->readFile(filename);
                result = d->blendFile(model, fileContents);
            } else {
                qCDebug(KOMPAREDIFF2_LOG) << "File " << filename << " does not exist !";
                qCDebug(KOMPAREDIFF2_LOG) << "Assume empty file !";
                fileContents.truncate(0);
                result = d->blendFile(model, fileContents);
            }
        }
        qCDebug(KOMPAREDIFF2_LOG) << "End of Blend Dir";
    } else if (fi.isFile()) { // is a file
        qCDebug(KOMPAREDIFF2_LOG) << "Blend File";
        qCDebug(KOMPAREDIFF2_LOG) << "Reading from: " << localURL;
        fileContents = d->readFile(localURL);

        result = d->blendFile((*d->models)[0], fileContents);
        qCDebug(KOMPAREDIFF2_LOG) << "End of Blend File";
    }

    return result;
}

void ModelList::show()
{
    Q_D(ModelList);

    qCDebug(KOMPAREDIFF2_LOG) << "ModelList::Show Number of models = " << d->models->count();
    Q_EMIT modelsChanged(d->models.get());
    Q_EMIT setSelection(d->selectedModel, d->selectedDifference);
}

const DiffModel *ModelList::modelAt(int i) const
{
    Q_D(const ModelList);

    return d->models->at(i);
}

DiffModel *ModelList::modelAt(int i)
{
    Q_D(ModelList);

    return d->models->at(i);
}

int ModelList::findModel(DiffModel *model) const
{
    Q_D(const ModelList);

    return d->models->indexOf(model);
}

int ModelList::currentModel() const
{
    Q_D(const ModelList);

    return d->models->indexOf(d->selectedModel);
}

int ModelList::currentDifference() const
{
    Q_D(const ModelList);

    return d->selectedModel ? d->selectedModel->findDifference(d->selectedDifference) : -1;
}

const DiffModel *ModelList::selectedModel() const
{
    Q_D(const ModelList);

    return d->selectedModel;
}

const Difference *ModelList::selectedDifference() const
{
    Q_D(const ModelList);

    return d->selectedDifference;
}

void ModelList::clear()
{
    Q_D(ModelList);

    if (d->models)
        d->models->clear();

    Q_EMIT modelsChanged(d->models.get());
}

void ModelList::refresh()
{
    Q_D(ModelList);

    // FIXME: I can imagine blending also wants to be refreshed so make a switch case here
    compare(d->info->mode);
}

void ModelList::swap()
{
    Q_D(ModelList);

    // FIXME Not sure if any mode could be swapped
    if (d->info->mode == ComparingFiles)
        compare(d->info->mode);
    else if (d->info->mode == ComparingDirs)
        compare(d->info->mode);
}

bool ModelList::hasUnsavedChanges() const
{
    Q_D(const ModelList);

    if (!d->models) {
        return false;
    }

    return std::any_of(d->models->constBegin(), d->models->constEnd(), [] (const DiffModel *model) {
        return model->hasUnsavedChanges();
    });
}

int ModelList::modelCount() const
{
    Q_D(const ModelList);

    return d->models ? d->models->count() : 0;
}

int ModelList::differenceCount() const
{
    Q_D(const ModelList);

    return d->selectedModel ? d->selectedModel->differenceCount() : -1;
}

int ModelList::appliedCount() const
{
    Q_D(const ModelList);

    return d->selectedModel ? d->selectedModel->appliedCount() : -1;
}

void ModelList::slotKompareInfo(Info *info)
{
    Q_D(ModelList);

    d->info = info;
}

void ModelList::slotActionApplyDifference()
{
    Q_D(ModelList);

    if (!d->selectedDifference->applied())
        slotApplyDifference(true);
    slotNextDifference();
    d->updateModelListActions();
}

void ModelList::slotActionUnApplyDifference()
{
    Q_D(ModelList);

    if (d->selectedDifference->applied())
        slotApplyDifference(false);
    slotPreviousDifference();
    d->updateModelListActions();
}

void ModelList::slotActionApplyAllDifferences()
{
    Q_D(ModelList);

    slotApplyAllDifferences(true);
    d->updateModelListActions();
}

void ModelList::slotActionUnapplyAllDifferences()
{
    Q_D(ModelList);

    slotApplyAllDifferences(false);
    d->updateModelListActions();
}

#include "moc_modellist.cpp"

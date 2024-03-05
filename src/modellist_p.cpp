/*
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2001-2005,2009 Otto Bruggeman <bruggie@gmail.com>
    SPDX-FileCopyrightText: 2007-2008 Kevin Kofler <kevin.kofler@chello.at>
    SPDX-FileCopyrightText: 2012 Jean -Nicolas Artaud <jeannicolasartaud@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "modellist_p.h"

// lib
#include "info.h"
#include <komparediff2_logging.h>
// Qt
#include <QAction>

using namespace KompareDiff2;

QStringList ModelListPrivate::split(const QString &fileContents)
{
    QString contents = fileContents;
    QStringList list;

    int pos = 0;
    int oldpos = 0;
    // split that does not strip the split char
#ifdef QT_OS_MAC
    const QLatin1Char split = QLatin1Char('\r');
#else
    const QLatin1Char split = QLatin1Char('\n');
#endif
    while ((pos = contents.indexOf(split, oldpos)) >= 0) {
        list.append(contents.mid(oldpos, pos - oldpos + 1));
        oldpos = pos + 1;
    }

    if (contents.length() > oldpos) {
        list.append(contents.right(contents.length() - oldpos));
    }

    return list;
}

QString ModelListPrivate::readFile(const QString &fileName)
{
    QStringList list;

    QFile file(fileName);
    file.open(QIODevice::ReadOnly);

    qCDebug(KOMPAREDIFF2_LOG) << "Codec = " << textDecoder.name();
    if (!textDecoder.isValid())
        textDecoder = QStringDecoder(QStringDecoder::System);
    textDecoder.resetState();

    QString contents;
    while (!file.atEnd()) {
        char buffer[4096];
        const auto len = file.read(buffer, 4096);
        contents += textDecoder.decode(QByteArrayView(buffer, len));
    }

    file.close();

    return contents;
}

bool ModelListPrivate::hasPrevModel() const
{
    // qCDebug(KOMPAREDIFF2_LOG) << "ModelListPrivate::hasPrevModel()";

    if (modelIndex > 0) {
//         qCDebug(KOMPAREDIFF2_LOG) << "has prev model";
        return true;
    }

//     qCDebug(KOMPAREDIFF2_LOG) << "doesn't have a prev model, this is the first one...";

    return false;
}

bool ModelListPrivate::hasNextModel() const
{
    // qCDebug(KOMPAREDIFF2_LOG) << "ModelListPrivate::hasNextModel()";

    if (modelIndex < (models->count() - 1)) {
//         qCDebug(KOMPAREDIFF2_LOG) << "has next model";
        return true;
    }

//     qCDebug(KOMPAREDIFF2_LOG) << "doesn't have a next model, this is the last one...";
    return false;
}

bool ModelListPrivate::hasPrevDiff() const
{
//     qCDebug(KOMPAREDIFF2_LOG) << "ModelListPrivate::hasPrevDiff()";
    const int index = selectedModel->diffIndex();

    if (index > 0) {
//         qCDebug(KOMPAREDIFF2_LOG) << "has prev difference in same model";
        return true;
    }

    if (hasPrevModel()) {
//         qCDebug(KOMPAREDIFF2_LOG) << "has prev difference but in prev model";
        return true;
    }

//     qCDebug(KOMPAREDIFF2_LOG) << "doesn't have a prev difference, not even in the previous model because there is no previous model";

    return false;
}

bool ModelListPrivate::hasNextDiff() const
{
//     qCDebug(KOMPAREDIFF2_LOG) << "ModelListPrivate::hasNextDiff()";
    const int index = selectedModel->diffIndex();

    if (index < (selectedModel->differenceCount() - 1)) {
//         qCDebug(KOMPAREDIFF2_LOG) << "has next difference in same model";
        return true;
    }

    if (hasNextModel()) {
//         qCDebug(KOMPAREDIFF2_LOG) << "has next difference but in next model";
        return true;
    }

//     qCDebug(KOMPAREDIFF2_LOG) << "doesn't have a next difference, not even in next model because there is no next model";

    return false;
}

static QString lstripSeparators(const QString &from, uint count)
{
    int position = 0;
    for (uint i = 0; i < count; ++i) {
        position = from.indexOf(QLatin1Char('/'), position);
        if (position == -1) {
            break;
        }
    }
    if (position == -1) {
        return QString();
    }

    return from.mid(position + 1);
}

void ModelListPrivate::setDepthAndApplied()
{
    // Splice to avoid calling ~DiffModelList
    const QList<KompareDiff2::DiffModel *> splicedModelList(*models);
    for (DiffModel *model : splicedModelList) {
        model->setSourceFile(lstripSeparators(model->source(), info->depth));
        model->setDestinationFile(lstripSeparators(model->destination(), info->depth));
        model->applyAllDifferences(info->applied);
    }
}

DiffModel *ModelListPrivate::firstModel()
{
    qCDebug(KOMPAREDIFF2_LOG) << "ModelListPrivate::firstModel()";
    modelIndex = 0;
    qCDebug(KOMPAREDIFF2_LOG) << "modelIndex = " << modelIndex;

    selectedModel = models->first();

    return selectedModel;
}

DiffModel *ModelListPrivate::lastModel()
{
    qCDebug(KOMPAREDIFF2_LOG) << "ModelListPrivate::lastModel()";
    modelIndex = models->count() - 1;
    qCDebug(KOMPAREDIFF2_LOG) << "modelIndex = " << modelIndex;

    selectedModel = models->last();

    return selectedModel;
}

DiffModel *ModelListPrivate::prevModel()
{
    qCDebug(KOMPAREDIFF2_LOG) << "ModelListPrivate::prevModel()";
    if (modelIndex > 0 && --modelIndex < models->count()) {
        qCDebug(KOMPAREDIFF2_LOG) << "modelIndex = " << modelIndex;
        selectedModel = (*models)[modelIndex];
    } else {
        selectedModel = nullptr;
        modelIndex = 0;
        qCDebug(KOMPAREDIFF2_LOG) << "modelIndex = " << modelIndex;
    }

    return selectedModel;
}

DiffModel *ModelListPrivate::nextModel()
{
    qCDebug(KOMPAREDIFF2_LOG) << "ModelListPrivate::nextModel()";
    if (++modelIndex < models->count()) {
        qCDebug(KOMPAREDIFF2_LOG) << "modelIndex = " << modelIndex;
        selectedModel = (*models)[modelIndex];
    } else {
        selectedModel = nullptr;
        modelIndex = 0;
        qCDebug(KOMPAREDIFF2_LOG) << "modelIndex = " << modelIndex;
    }

    return selectedModel;
}

bool ModelListPrivate::setSelectedModel(DiffModel *model)
{
    qCDebug(KOMPAREDIFF2_LOG) << "ModelListPrivate::setSelectedModel( " << model << " )";

    if (model != selectedModel) {
        if (!models->contains(model))
            return false;
        qCDebug(KOMPAREDIFF2_LOG) << "selectedModel (was) = " << selectedModel;
        modelIndex = models->indexOf(model);
        qCDebug(KOMPAREDIFF2_LOG) << "selectedModel (is)  = " << selectedModel;
        selectedModel = model;
    }

    updateModelListActions();

    return true;
}

void ModelListPrivate::updateModelListActions()
{
    if (models && selectedModel && selectedDifference) {
        if (isReadWrite && save) {
            if (selectedModel->appliedCount() != selectedModel->differenceCount())
                applyAll->setEnabled(true);
            else
                applyAll->setEnabled(false);

            if (selectedModel->appliedCount() != 0)
                unapplyAll->setEnabled(true);
            else
                unapplyAll->setEnabled(false);

            applyDifference->setEnabled(selectedDifference->applied() == false);
            unApplyDifference->setEnabled(selectedDifference->applied() == true);
            save->setEnabled(selectedModel->hasUnsavedChanges());
        } else if (save) {
            applyDifference->setEnabled(false);
            unApplyDifference->setEnabled(false);
            applyAll->setEnabled(false);
            unapplyAll->setEnabled(false);
            save->setEnabled(false);
        }

        previousFile->setEnabled(hasPrevModel());
        nextFile->setEnabled(hasNextModel());
        previousDifference->setEnabled(hasPrevDiff());
        nextDifference->setEnabled(hasNextDiff());
    } else {
        if (save) {
            applyDifference->setEnabled(false);
            unApplyDifference->setEnabled(false);
            applyAll->setEnabled(false);
            unapplyAll->setEnabled(false);
            save->setEnabled(false);
        }

        previousFile->setEnabled(false);
        nextFile->setEnabled(false);
        previousDifference->setEnabled(false);
        nextDifference->setEnabled(false);
    }
}

bool ModelListPrivate::blendFile(DiffModel *model, const QString &fileContents)
{
    if (!model) {
        qCDebug(KOMPAREDIFF2_LOG) << "**** model is null :(";
        return false;
    }

    model->setBlended(true);

    int srcLineNo = 1, destLineNo = 1;

    const QStringList lines = split(fileContents);
    auto linesIt = lines.constBegin(), lEnd = lines.constEnd();

    DiffHunkList *hunks = model->hunks();
    qCDebug(KOMPAREDIFF2_LOG) << "Hunks in hunklist: " << hunks->count();
    DiffHunkListIterator hunkIt = hunks->begin();

    DiffHunk *newHunk = nullptr;
    Difference *newDiff = nullptr;

    // FIXME: this approach is not very good, we should first check if the hunk applies cleanly
    // and without offset and if not use that new linenumber with offset to compare against
    // This will only work for files we just diffed with kompare but not for blending where
    // file(s) to patch has/have potentially changed

    for (; hunkIt != hunks->end(); ++hunkIt) {
        // Do we need to insert a new hunk before this one ?
        DiffHunk *hunk = *hunkIt;
        if (srcLineNo < hunk->sourceLineNumber()) {
            newHunk = new DiffHunk(srcLineNo, destLineNo, QString(), DiffHunk::AddedByBlend);

            hunkIt = ++hunks->insert(hunkIt, newHunk);

            newDiff = new Difference(srcLineNo, destLineNo, Difference::Unchanged);

            newHunk->add(newDiff);

            while (srcLineNo < hunk->sourceLineNumber() && linesIt != lEnd) {
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
        if (linesIt > lEnd) {
            linesIt = lEnd;
        }

        srcLineNo += size;
        destLineNo += hunk->destinationLineCount();
    }

    if (linesIt != lEnd) {
        newHunk = new DiffHunk(srcLineNo, destLineNo, QString(), DiffHunk::AddedByBlend);

        model->addHunk(newHunk);

        newDiff = new Difference(srcLineNo, destLineNo, Difference::Unchanged);

        newHunk->add(newDiff);

        while (linesIt != lEnd) {
            newDiff->addSourceLine(*linesIt);
            newDiff->addDestinationLine(*linesIt);
            ++linesIt;
        }
    }
#if 0
    DifferenceList hunkDiffList   = (*hunkIt)->differences();
    DifferenceListIterator diffIt = hunkDiffList.begin();
    DifferenceListIterator dEnd   = hunkDiffList.end();
    qCDebug(KOMPAREDIFF2_LOG) << "Number of differences in hunkDiffList = " << diffList.count();

    DifferenceListIterator tempIt;
    Difference* diff;

    for (; diffIt != dEnd; ++diffIt)
    {
        diff = *diffIt;
        qCDebug(KOMPAREDIFF2_LOG) << "*(Diff it) = " << diff;
        // Check if there are lines in the original file before the difference
        // that are not yet in the diff. If so create new Unchanged diff
        if (srcLineNo < diff->sourceLineNumber())
        {
            newDiff = new Difference(srcLineNo, destLineNo,
                                     Difference::Unchanged | Difference::AddedByBlend);
            newHunk->add(newDiff);
            while (srcLineNo < diff->sourceLineNumber() && linesIt != lEnd)
            {
//                  qCDebug(KOMPAREDIFF2_LOG) << "SourceLine = " << srcLineNo << ": " << *linesIt;
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
            qCDebug(KOMPAREDIFF2_LOG) << "Unchanged";
            for (int i = 0; i < diff->sourceLineCount(); ++i)
            {
                if (linesIt != lEnd && *linesIt != diff->sourceLineAt(i)->string())
                {
                    qCDebug(KOMPAREDIFF2_LOG) << "Conflict: SourceLine = " << srcLineNo << ": " << *linesIt;
                    qCDebug(KOMPAREDIFF2_LOG) << "Conflict: DiffLine   = " << diff->sourceLineNumber() + i << ": " << diff->sourceLineAt(i)->string();

                    // Do conflict resolution (well sort of)
                    diff->sourceLineAt(i)->setConflictString(*linesIt);
                    diff->setConflict(true);
                }
//                  qCDebug(KOMPAREDIFF2_LOG) << "SourceLine = " << srcLineNo << ": " << *linesIt;
//                  qCDebug(KOMPAREDIFF2_LOG) << "DiffLine   = " << diff->sourceLineNumber() + i << ": " << diff->sourceLineAt( i )->string();
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
            qCDebug(KOMPAREDIFF2_LOG) << "Change";

            //QStringListConstIterator saveIt = linesIt;

            for (int i = 0; i < diff->sourceLineCount(); ++i)
            {
                if (linesIt != lEnd && *linesIt != diff->sourceLineAt(i)->string())
                {
                    qCDebug(KOMPAREDIFF2_LOG) << "Conflict: SourceLine = " << srcLineNo << ": " << *linesIt;
                    qCDebug(KOMPAREDIFF2_LOG) << "Conflict: DiffLine   = " << diff->sourceLineNumber() + i << ": " << diff->sourceLineAt(i)->string();

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
            qCDebug(KOMPAREDIFF2_LOG) << "Insert";
            destLineNo += diff->destinationLineCount();
            tempIt = diffIt;
            --diffIt;
            diffList.remove(tempIt);
            newHunk->add(diff);
            newModel->addDiff(diff);
            break;
        case Difference::Delete:
            qCDebug(KOMPAREDIFF2_LOG) << "Delete";
            qCDebug(KOMPAREDIFF2_LOG) << "Number of lines in Delete: " << diff->sourceLineCount();
            for (int i = 0; i < diff->sourceLineCount(); ++i)
            {
                if (linesIt != lEnd && *linesIt != diff->sourceLineAt(i)->string())
                {
                    qCDebug(KOMPAREDIFF2_LOG) << "Conflict: SourceLine = " << srcLineNo << ": " << *linesIt;
                    qCDebug(KOMPAREDIFF2_LOG) << "Conflict: DiffLine   = " << diff->sourceLineNumber() + i << ": " << diff->sourceLineAt(i)->string();

                    // Do conflict resolution (well sort of)
                    diff->sourceLineAt(i)->setConflictString(*linesIt);
                    diff->setConflict(true);
                }

//                  qCDebug(KOMPAREDIFF2_LOG) << "SourceLine = " << srcLineNo << ": " << *it;
//                  qCDebug(KOMPAREDIFF2_LOG) << "DiffLine   = " << diff->sourceLineNumber() + i << ": " << diff->sourceLineAt( i )->string();
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
            qCDebug(KOMPAREDIFF2_LOG) << "****, some diff type we do not know about ???";
        }
    }
}
#endif

/*
    diffList = newModel->differences();

    diff = diffList.first();
    qCDebug(KOMPAREDIFF2_LOG) << "Count = " << diffList.count();
    for ( diff = diffList.first(); diff; diff = diffList.next() )
    {
        qCDebug(KOMPAREDIFF2_LOG) << "sourcelinenumber = " << diff->sourceLineNumber();
    }
*/

    selectedModel = firstModel();

    selectedDifference = selectedModel->firstDifference();

    return true;
}

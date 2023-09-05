/*
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2001-2005,2009 Otto Bruggeman <bruggie@gmail.com>
    SPDX-FileCopyrightText: 2007-2008 Kevin Kofler <kevin.kofler@chello.at>
    SPDX-FileCopyrightText: 2012 Jean -Nicolas Artaud <jeannicolasartaud@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_MODELLIST_H
#define KOMPAREDIFF2_MODELLIST_H

// lib
#include "diffmodel.h"
#include "diffmodellist.h"
#include "info.h"
#include "komparediff2_export.h"
// Qt
#include <QObject>
// Std
#include <memory>

class KActionCollection;

namespace KompareDiff2
{
class DiffSettings;
class ModelListPrivate;

/**
 * @class ModelList modellist.h <KompareDiff2/ModelList>
 *
 * ModelList
 */
class KOMPAREDIFF2_EXPORT ModelList : public QObject
{
    Q_OBJECT
public:
    ModelList(DiffSettings *diffSettings, QObject *parent, bool supportReadWrite = true);
    ~ModelList() override;

public:
    void refresh();
    // Swap source with destination and show differences
    void swap();

    /* Comparing methods */
    bool compare();

    bool compare(Mode);

    bool openDiff(const QString &diff);

    bool openFileAndDiff();
    bool openDirAndDiff();

    bool saveDiff(const QString &url, const QString &directory, DiffSettings *diffSettings);
    bool saveAll();

    bool saveDestination(DiffModel *model);

    void setEncoding(const QString &encoding);

    void setReadWrite(bool isReadWrite);
    bool isReadWrite() const;

    QString recreateDiff() const;

    // This parses the difflines and creates new models
    int parseDiffOutput(const QString &diff);

    // This open the difflines after parsing them
    bool parseAndOpenDiff(const QString &diff);

    // Call this to emit the signals to the rest of the "world" to show the diff
    void show();

    // This will blend the original URL (dir or file) into the diffmodel,
    // this is like patching but with a twist
    bool blendOriginalIntoModelList(const QString &localURL);

    // This mode() method is superfluous now so FIXME
    Mode mode() const;
    const DiffModelList *models() const;

    KActionCollection *actionCollection() const;
    int modelCount() const;
    int differenceCount() const;
    int appliedCount() const;

    const DiffModel *modelAt(int i) const;
    DiffModel *modelAt(int i);
    int findModel(DiffModel *model) const;

    bool hasUnsavedChanges() const;

    int currentModel() const;
    int currentDifference() const;

    const DiffModel *selectedModel() const;
    const Difference *selectedDifference() const;

    void clear();

Q_SIGNALS:
    void status(KompareDiff2::Status status);
    void setStatusBarModelInfo(int modelIndex, int differenceIndex, int modelCount, int differenceCount, int appliedCount);
    void error(const QString &error);
    void modelsChanged(const KompareDiff2::DiffModelList *models);
    void setSelection(const KompareDiff2::DiffModel *model, const KompareDiff2::Difference *diff);
    void setSelection(const KompareDiff2::Difference *diff);
    void applyDifference(bool apply);
    void applyAllDifferences(bool apply);
    void applyDifference(const KompareDiff2::Difference *diff, bool apply);
    void diffString(const QString &);
    void updateActions();

public Q_SLOTS:
    void slotSelectionChanged(const KompareDiff2::DiffModel *model, const KompareDiff2::Difference *diff);
    void slotSelectionChanged(const KompareDiff2::Difference *diff);

    void slotApplyDifference(bool apply);
    void slotApplyAllDifferences(bool apply);
    void slotPreviousModel();
    void slotNextModel();
    void slotPreviousDifference();
    void slotNextDifference();

    void slotKompareInfo(KompareDiff2::Info *);

protected Q_SLOTS:
    void slotDiffProcessFinished(bool success);
    void slotWriteDiffOutput(bool success);

    void slotActionApplyDifference();
    void slotActionUnApplyDifference();
    void slotActionApplyAllDifferences();
    void slotActionUnapplyAllDifferences();

    /** Save the currently selected destination in a multi-file diff,
        or the single destination if a single file diff. */
    void slotSaveDestination();

private:
    Q_DECLARE_PRIVATE(ModelList)
    std::unique_ptr<ModelListPrivate> const d_ptr;
};

} // End of namespace KompareDiff2

#endif

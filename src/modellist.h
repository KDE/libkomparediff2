/*
SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
SPDX-FileCopyrightText: 2001-2005,2009 Otto Bruggeman <bruggie@gmail.com>
SPDX-FileCopyrightText: 2007-2008 Kevin Kofler <kevin.kofler@chello.at>
SPDX-FileCopyrightText: 2012 Jean -Nicolas Artaud <jeannicolasartaud@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_MODELLIST_H
#define KOMPAREDIFF2_MODELLIST_H

#include <QObject>

#include "diffmodel.h"
#include "diffmodellist.h"
#include "info.h"
#include "komparediff2_export.h"

class QAction;
class QTemporaryFile;
class QTextCodec;
class KActionCollection;

class DiffSettings;
class KompareProcess;

namespace KompareDiff2
{

/**
 * @class ModelList modellist.h <KompareDiff2/ModelList>
 *
 * ModelList
 */
class KOMPAREDIFF2_EXPORT ModelList : public QObject
{
    Q_OBJECT
public:
    ModelList(DiffSettings* diffSettings, QObject* parent, bool supportReadWrite = true);
    ~ModelList() override;

public:
    void refresh();
    // Swap source with destination and show differences
    void swap();

    /* Comparing methods */
    bool compare();

    bool compare(Mode);

    bool openDiff(const QString& diff);

    bool openFileAndDiff();
    bool openDirAndDiff();

    bool saveDiff(const QString& url, const QString& directory, DiffSettings* diffSettings);
    bool saveAll();

    bool saveDestination(DiffModel* model);

    void setEncoding(const QString& encoding);

    void setReadWrite(bool isReadWrite);
    bool isReadWrite() const;

    QString recreateDiff() const;

    // This parses the difflines and creates new models
    int parseDiffOutput(const QString& diff);

    // This open the difflines after parsing them
    bool parseAndOpenDiff(const QString& diff);

    // Call this to emit the signals to the rest of the "world" to show the diff
    void show();

    // This will blend the original URL (dir or file) into the diffmodel,
    // this is like patching but with a twist
    bool blendOriginalIntoModelList(const QString& localURL);

    // This mode() method is superfluous now so FIXME
    enum Mode    mode()   const { return m_info->mode; };
    const DiffModelList*  models() const { return m_models; };

    KActionCollection* actionCollection() const;
    int modelCount() const;
    int differenceCount() const;
    int appliedCount() const;

    const DiffModel* modelAt(int i) const { return m_models->at(i); };
    DiffModel* modelAt(int i) { return m_models->at(i); };
    int              findModel(DiffModel* model) const { return m_models->indexOf(model); };

    bool hasUnsavedChanges() const;

    int currentModel() const      { return m_models->indexOf(m_selectedModel); };
    int currentDifference() const { return m_selectedModel ? m_selectedModel->findDifference(m_selectedDifference) : -1; };

    const DiffModel* selectedModel() const       { return m_selectedModel; };
    const Difference* selectedDifference() const { return m_selectedDifference; };

    void clear();

private:
    KompareDiff2::DiffModel* firstModel();
    KompareDiff2::DiffModel* lastModel();
    KompareDiff2::DiffModel* prevModel();
    KompareDiff2::DiffModel* nextModel();

    bool setSelectedModel(KompareDiff2::DiffModel* model);

    void updateModelListActions();

protected:
    bool blendFile(DiffModel* model, const QString& lines);

Q_SIGNALS:
    void status(KompareDiff2::Status status);
    void setStatusBarModelInfo(int modelIndex, int differenceIndex, int modelCount, int differenceCount, int appliedCount);
    void error(const QString &error);
    void modelsChanged(const KompareDiff2::DiffModelList* models);
    void setSelection(const KompareDiff2::DiffModel* model, const KompareDiff2::Difference* diff);
    void setSelection(const KompareDiff2::Difference* diff);
    void applyDifference(bool apply);
    void applyAllDifferences(bool apply);
    void applyDifference(const KompareDiff2::Difference* diff, bool apply);
    void diffString(const QString&);
    void updateActions();

public Q_SLOTS:
    void slotSelectionChanged(const KompareDiff2::DiffModel* model, const KompareDiff2::Difference* diff);
    void slotSelectionChanged(const KompareDiff2::Difference* diff);

    void slotApplyDifference(bool apply);
    void slotApplyAllDifferences(bool apply);
    void slotPreviousModel();
    void slotNextModel();
    void slotPreviousDifference();
    void slotNextDifference();

    void slotKompareInfo(KompareDiff2::Info*);

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

private Q_SLOTS:
    void slotDirectoryChanged(const QString&);
    void slotFileChanged(const QString&);

private: // Helper methods
    bool isDirectory(const QString& url) const;
    bool isDiff(const QString& mimetype) const;
    QString readFile(const QString& fileName);

    bool hasPrevModel() const;
    bool hasNextModel() const;
    bool hasPrevDiff() const;
    bool hasNextDiff() const;

    QStringList split(const QString& diff);
    void setDepthAndApplied();

private: // ### an exported class without a d pointer? Really? What about BC?
    QTemporaryFile*       m_diffTemp;
    QUrl                  m_diffURL;

    KompareProcess*       m_diffProcess;

    DiffSettings*         m_diffSettings;

    DiffModelList*        m_models;

    DiffModel*            m_selectedModel;
    Difference*           m_selectedDifference;

    int                   m_modelIndex;

    class Info*           m_info;

    KActionCollection*    m_actionCollection;
    QAction*              m_applyDifference;
    QAction*              m_unApplyDifference;
    QAction*              m_applyAll;
    QAction*              m_unapplyAll;
    QAction*              m_previousFile;
    QAction*              m_nextFile;
    QAction*              m_previousDifference;
    QAction*              m_nextDifference;

    QAction*              m_save;

    QString               m_encoding;
    QTextCodec*           m_textCodec;

    bool                  m_isReadWrite;
};

} // End of namespace KompareDiff2

#endif

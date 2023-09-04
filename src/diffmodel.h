/*
SPDX-FileCopyrightText: 2001-2004,2009 Otto Bruggeman <bruggie@gmail.com>
SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_DIFFMODEL_H
#define KOMPAREDIFF2_DIFFMODEL_H

// lib
#include "diffhunk.h"
#include "global.h"
#include "komparediff2_export.h"
// Qt
#include <QObject>
#include <QStringList>
// Std
#include <memory>

namespace KompareDiff2
{
class DiffModelPrivate;

/**
 * @class DiffModel diffmodel.h <KompareDiff2/DiffModel>
 *
 * A model describing the differences between two files.
 */
class KOMPAREDIFF2_EXPORT DiffModel : public QObject
{
    Q_OBJECT

public:
    DiffModel(const QString& srcBaseURL, const QString& destBaseURL);
    DiffModel();
    ~DiffModel() override;

public:
    DiffModel& operator=(const DiffModel& model);
    bool operator<(const DiffModel& model) const;

    int parseDiff(enum Format format, const QStringList& list);

    QString recreateDiff() const;

    int hunkCount() const;
    int differenceCount() const;
    int appliedCount() const;

    DiffHunk* hunkAt(int i);
    const Difference* differenceAt(int i) const;
    Difference* differenceAt(int i);

    DiffHunkList*         hunks();
    const DiffHunkList*   hunks() const;
    DifferenceList*       differences();
    const DifferenceList* differences() const;

    int findDifference(Difference* diff) const;

    Difference* firstDifference();
    Difference* lastDifference();
    Difference* prevDifference();
    Difference* nextDifference();

    QString source() const;
    QString destination() const;
    QString sourceFile() const;
    QString destinationFile() const;
    QString sourcePath() const;
    QString destinationPath() const;
    QString sourceTimestamp() const;
    QString destinationTimestamp() const;
    QString sourceRevision() const;
    QString destinationRevision() const;

    void setSourceFile(const QString &path);
    void setDestinationFile(const QString &path);
    void setSourceTimestamp(const QString &timestamp);
    void setDestinationTimestamp(const QString &timestamp);
    void setSourceRevision(const QString &revision);
    void setDestinationRevision(const QString &revision);

    void addHunk(DiffHunk* hunk);
    void addDiff(Difference* diff);
    bool hasUnsavedChanges() const;

    int  diffIndex() const;
    void setDiffIndex(int diffIndex);

    void applyDifference(bool apply);
    void applyAllDifferences(bool apply);

    bool setSelectedDifference(Difference* diff);

    int localeAwareCompareSource(const DiffModel& model) const;

    bool isBlended() const;
    void setBlended(bool blended);

    /**
     * @p oldlines - lines that were removed.
     * @p newLines - lines that were inserted.
     * @p startPos - number of line at which the change occurred
     */
    QPair<QList<Difference*>, QList<Difference*> > linesChanged(const QStringList& oldLines, const QStringList& newLines, int editLineNumber);

private Q_SLOTS:
    void slotDifferenceApplied(Difference* diff);

private:
    Q_DECLARE_PRIVATE(DiffModel)
    std::unique_ptr<DiffModelPrivate> const d_ptr;
};

} // End of namespace KompareDiff2

#endif


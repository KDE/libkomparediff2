/*
SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
SPDX-FileCopyrightText: 2001-2005,2009 Otto Bruggeman <bruggie@gmail.com>
SPDX-FileCopyrightText: 2007-2008 Kevin Kofler <kevin.kofler@chello.at>
SPDX-FileCopyrightText: 2012 Jean -Nicolas Artaud <jeannicolasartaud@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_MODELLIST_P_H
#define KOMPAREDIFF2_MODELLIST_P_H

// lib
#include "diffmodellist.h"
// Qt
#include <QUrl>
#include <QFileInfo>

class KActionCollection;
class QAction;
class QTemporaryFile;
class QTextCodec;

class KompareProcess;

namespace KompareDiff2
{
class DiffSettings;
class DiffModelList;
class DiffModel;
class Difference;
class Info;

class ModelListPrivate
{
public:
    ModelListPrivate(DiffSettings* diffSettings, bool supportReadWrite);
    ~ModelListPrivate();

public: // Helper methods
    static bool isDirectory(const QString& url);
    static bool isDiff(const QString& mimetype);

    static QStringList split(const QString& diff);

    QString readFile(const QString& fileName);

    bool hasPrevModel() const;
    bool hasNextModel() const;
    bool hasPrevDiff() const;
    bool hasNextDiff() const;

    void setDepthAndApplied();

    DiffModel* firstModel();
    DiffModel* lastModel();
    DiffModel* prevModel();
    DiffModel* nextModel();

    bool setSelectedModel(DiffModel* model);

    void updateModelListActions();

    bool blendFile(DiffModel* model, const QString& lines);

public:
    QTemporaryFile* diffTemp;
    QUrl diffURL;

    KompareProcess* diffProcess = nullptr;

    DiffSettings* diffSettings;

    DiffModelList* models = nullptr;

    DiffModel* selectedModel = nullptr;
    Difference* selectedDifference = nullptr;

    int modelIndex = 0;

    Info* info = nullptr;

    KActionCollection* actionCollection;
    QAction* applyDifference;
    QAction* unApplyDifference;
    QAction* applyAll;
    QAction* unapplyAll;
    QAction* previousFile;
    QAction* nextFile;
    QAction* previousDifference;
    QAction* nextDifference;

    QAction* save;

    QString encoding;
    QTextCodec* textCodec = nullptr;

    bool isReadWrite;
};

inline
ModelListPrivate::ModelListPrivate(DiffSettings* diffSettings, bool supportReadWrite)
    : diffSettings(diffSettings),
      isReadWrite(supportReadWrite)
{
}

inline
ModelListPrivate::~ModelListPrivate()
{
    selectedModel = nullptr;
    selectedDifference = nullptr;
    info = nullptr;
    delete models;
}

inline
bool ModelListPrivate::isDirectory(const QString& url)
{
    QFileInfo fi(url);
    return fi.isDir();
}

inline
bool ModelListPrivate::isDiff(const QString& mimeType)
{
    return (mimeType == QLatin1String("text/x-patch"));
}

}

#endif

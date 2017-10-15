/***************************************************************************
                                settingsbase.h
                                --------------
        begin                   : Sun Mar 4 2001
        Copyright 2001 Otto Bruggeman <otto.bruggeman@home.nl>
        Copyright 2001 John Firebaugh <jfirebaugh@kde.org>
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#ifndef SETTINGSBASE_H
#define SETTINGSBASE_H

#include <QObject>

#include "kompare.h"
#include "diff2_export.h"

class QWidget;
class KConfig;

class DIFF2_EXPORT SettingsBase : public QObject
{
    Q_OBJECT
public:
    explicit SettingsBase(QWidget* parent);
    ~SettingsBase() override;

public:
    virtual void loadSettings(KConfig* config);
    virtual void saveSettings(KConfig* config);
};

#endif

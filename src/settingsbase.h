/*
SPDX-FileCopyrightText: 2001 Otto Bruggeman <otto.bruggeman@home.nl>
SPDX-FileCopyrightText: 2001 John Firebaugh <jfirebaugh@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SETTINGSBASE_H
#define SETTINGSBASE_H

#include <QObject>

#include "komparediff2_export.h"

class QWidget;
class KConfig;

/**
 * @class SettingsBase settingsbase.h <KompareDiff2/SettingsBase>
 *
 * Base class for settings classes.
 */
class KOMPAREDIFF2_EXPORT SettingsBase : public QObject
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

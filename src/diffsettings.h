/*
SPDX-FileCopyrightText: 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DIFFSETTINGS_H
#define DIFFSETTINGS_H

#include <QStringList>

#include "kompare.h"
#include "settingsbase.h"
#include "komparediff2_export.h"
#if KOMPAREDIFF2_ENABLE_DEPRECATED_SINCE(5, 4)
#include "diff2_export_p.h"
#endif

/**
 * The settings for a diff.
 */
class KOMPAREDIFF2_EXPORT  DiffSettings : public SettingsBase
{
    Q_OBJECT
public:
    explicit DiffSettings(QWidget* parent);
    ~DiffSettings() override;
public:
    // some virtual functions that will be overloaded from the base class
    void loadSettings(KConfig* config) override;
    void saveSettings(KConfig* config) override;

public:
    QString m_diffProgram;
    int m_linesOfContext;
    Kompare::Format m_format;
    bool m_largeFiles;                           // -H
    bool m_ignoreWhiteSpace;                     // -b
    bool m_ignoreAllWhiteSpace;                  // -w
    bool m_ignoreEmptyLines;                     // -B
    bool m_ignoreChangesDueToTabExpansion;       // -E
    bool m_createSmallerDiff;                    // -d
    bool m_ignoreChangesInCase;                  // -i
    bool m_showCFunctionChange;                  // -p
    bool m_convertTabsToSpaces;                  // -t
    bool m_ignoreRegExp;                         // -I
    QString m_ignoreRegExpText;                  // the RE for -I
    QStringList m_ignoreRegExpTextHistory;
    bool m_recursive;                            // -r
    bool m_newFiles;                             // -N
//  bool m_allText;                              // -a
    bool m_excludeFilePattern;                   // -x
    QStringList m_excludeFilePatternList;        // The list of patterns for -x
    bool m_excludeFilesFile;                     // -X
    QString m_excludeFilesFileURL;               // The filename to -X
    QStringList m_excludeFilesFileHistoryList;   // The history list of filenames
};

#endif

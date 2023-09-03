/*
SPDX-FileCopyrightText: 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_KOMPARE_H
#define KOMPAREDIFF2_KOMPARE_H

namespace KompareDiff2
{
/**
 * Patch format enum.
 */
enum Format {
    Context       = 0,
    Ed            = 1,
    Normal        = 2,
    RCS           = 3,
    Unified       = 4,
    SideBySide    = 5,
    UnknownFormat = -1
};

/**
 * Patch generator enum.
 */
enum Generator {
    CVSDiff          = 0,
    Diff             = 1,
    Perforce         = 2,
    SubVersion       = 3,
    Reserved2        = 4,
    Reserved3        = 5,
    Reserved4        = 6,
    Reserved5        = 7,
    Reserved6        = 8,
    Reserved7        = 9,
    UnknownGenerator = -1
};

/**
 * Mode
 */
enum Mode {
    ComparingFiles,      // compareFiles
    ComparingFileString, // Compare a source file with a destination string
    ComparingStringFile, // Compare a source string with a destination file
    ComparingDirs,       // compareDirs
    ShowingDiff,         // openDiff
    BlendingDir,         // openDirAndDiff
    BlendingFile,        // openFileAndDiff
    UnknownMode          // Used to initialize the Infoi struct
};

/**
 * DiffMode
 */
enum DiffMode {
    Default,
    Custom,
    UnknownDiffMode // Use to initialize the Info struct
};

/**
 * State
 */
enum Status {
    RunningDiff,
    Parsing,
    FinishedParsing,
    FinishedWritingDiff,
    ReRunningDiff   // When a change has been detected after diff has run
};

} // End of namespace KompareDiff2

#endif

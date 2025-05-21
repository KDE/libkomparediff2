/*
    SPDX-FileCopyrightText: 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREDIFF2_GLOBAL_H
#define KOMPAREDIFF2_GLOBAL_H

namespace KompareDiff2
{
/*!
 * \enum KompareDiff2::Format
 * \brief Patch format enum.
 * \value UnknownFormat
 * \value Context
 * \value Ed
 * \value Normal
 * \value RCS
 * \value Unified
 * \value SideBySide
 */
enum Format {
    UnknownFormat = -1,
    Context       = 0,
    Ed            = 1,
    Normal        = 2,
    RCS           = 3,
    Unified       = 4,
    SideBySide    = 5,
};

/*!
 * \enum KompareDiff2::Generator
 * \brief Patch generator enum.
 * \value UnknownGenerator
 * \value CVSDiff
 * \value Diff
 * \value Perforce
 * \value SubVersion
 * \value Reserved2
 * \value Reserved3
 * \value Reserved4
 * \value Reserved5
 * \value Reserved6
 * \value Reserved7
 */
enum Generator {
    UnknownGenerator = -1,
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
};

/*!
 * \enum KompareDiff2::Mode
 * \value ComparingFiles
 *        compareFiles
 * \value ComparingFileString
 *        Compare a source file with a destination string.
 * \value ComparingStringFile
 *        Compare a source string with a destination file.
 * \value ComparingDirs
 *        compareDirs
 * \value ShowingDiff
 *        openDiff
 * \value BlendingDir
 *        openDirAndDiff
 * \value BlendingFile
 *        openFileAndDiff
 * \value UnknownMode
 *        Used to initialize the Info struct.
 */
enum Mode {
    ComparingFiles,
    ComparingFileString,
    ComparingStringFile,
    ComparingDirs,
    ShowingDiff,
    BlendingDir,
    BlendingFile,
    UnknownMode,
};

/*!
 * \enum KompareDiff2::DiffMode
 * \value Default
 * \value Custom
 * \value UnknownDiffMode
 *        Use to initialize the Info struct.
 */
enum DiffMode {
    Default,
    Custom,
    UnknownDiffMode,
};

/*!
 * \enum KompareDiff2::Status
 * \value RunningDiff
 * \value Parsing
 * \value FinishedParsing
 * \value FinishedWritingDiff
 * \value ReRunningDiff
 *        When a change has been detected after the diff has run.
 */
enum Status {
    RunningDiff,
    Parsing,
    FinishedParsing,
    FinishedWritingDiff,
    ReRunningDiff,
};

} // End of namespace KompareDiff2

#endif

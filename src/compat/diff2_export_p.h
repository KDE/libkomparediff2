/*
SPDX-FileCopyrightText: 2023 Friedrich W. H. Kossebau <kossebau@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

// Private header,do not use.
// Provides backward compat definitions.
// Assumes komparediff2_export.h was included before

#ifndef DIFF2_EXPORT_P_H
#define DIFF2_EXPORT_P_H

#ifndef DIFF2_EXPORT
#  define DIFF2_EXPORT KOMPAREDIFF2_EXPORT
#endif

#ifndef DIFF2_NO_EXPORT
#  define DIFF2_NO_EXPORT KOMPAREDIFF2_NO_EXPORT
#endif

#ifndef DIFF2_DEPRECATED
#  define DIFF2_DEPRECATED KOMPAREDIFF2_DEPRECATED
#endif

#ifndef DIFF2_DEPRECATED_EXPORT
#  define DIFF2_DEPRECATED_EXPORT KOMPAREDIFF2_DEPRECATED_EXPORT
#endif

#ifndef DIFF2_DEPRECATED_NO_EXPORT
#  define DIFF2_DEPRECATED_NO_EXPORT KOMPAREDIFF2_DEPRECATED_NO_EXPORT
#endif

#endif /* DIFF2_EXPORT_P_H */

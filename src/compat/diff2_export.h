/*
SPDX-FileCopyrightText: 2023 Friedrich W. H. Kossebau <kossebau@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DIFF2_EXPORT_H
#define DIFF2_EXPORT_H

#include <komparediff2/komparediff2_export.h>

#if KOMPAREDIFF2_ENABLE_DEPRECATED_SINCE(5, 4)
#  include "diff2_export_p.h"
#  if KOMPAREDIFF2_DEPRECATED_WARNINGS_SINCE >= 0x050400
#      pragma message("Deprecated header. Since 5.4, use #include <komparediff2/komparediff2_export.h> instead")
#  endif
#else
#   error "Include of deprecated header is disabled"
#endif

#endif /* DIFF2_EXPORT_H */

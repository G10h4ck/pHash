/*

    pHash, the open source perceptual hash library
    Copyright (C) 2009 Aetilius, Inc.

    HEIF / HEIC / AVIF loader -- thin wrapper around libheif that decodes
    into a CImg<uint8_t>. Header is always available; the implementation
    is only compiled / linked when HAVE_HEIF is defined.

*/

#ifndef _PH_HEIF_H
#define _PH_HEIF_H

#include <stdint.h>

#define cimg_display 0
#define cimg_debug 0
#include "CImg.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Returns non-zero when the path ends in .heic / .heif / .avif (case
 * insensitive). Always available -- callers use this to decide whether
 * to call ph_load_heif. */
int ph_is_heif_path(const char *file);

#ifdef __cplusplus
}  // extern "C"

#ifdef HAVE_HEIF
/* Load a HEIF / HEIC / AVIF file into img. Always produces a 3-channel
 * RGB CImg<uint8_t>. Returns 0 on success, -1 on any failure. */
int ph_load_heif(const char *file, cimg_library::CImg<uint8_t> &img);
#endif

#endif /* __cplusplus */

#endif /* _PH_HEIF_H */

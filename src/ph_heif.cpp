/*

    pHash, the open source perceptual hash library
    Copyright (C) 2009 Aetilius, Inc.

    HEIF / HEIC / AVIF support via libheif (issue #18).

*/

#include "ph_heif.h"
// pHash.h carries the CMake-generated HAVE_HEIF #define. Without this
// include, the libheif-using body below would be silently compiled out
// even when WITH_HEIF=ON, and the link of pHash.cpp -> ph_load_heif
// would fail with an undefined symbol.
#include "pHash.h"

#include <strings.h>

extern "C" int ph_is_heif_path(const char *file) {
    if (file == NULL) return 0;
    const char *dot = strrchr(file, '.');
    if (dot == NULL) return 0;
    return strcasecmp(dot, ".heic") == 0 ||
           strcasecmp(dot, ".heif") == 0 ||
           strcasecmp(dot, ".avif") == 0;
}

#ifdef HAVE_HEIF

#include <libheif/heif.h>

using cimg_library::CImg;

int ph_load_heif(const char *file, CImg<uint8_t> &img) {
    if (file == NULL) return -1;

    heif_context *ctx = heif_context_alloc();
    if (ctx == NULL) return -1;

    heif_image_handle *handle = NULL;
    heif_image *himg = NULL;
    int rc = -1;

    if (heif_context_read_from_file(ctx, file, NULL).code != heif_error_Ok)
        goto cleanup;
    if (heif_context_get_primary_image_handle(ctx, &handle).code !=
        heif_error_Ok)
        goto cleanup;

    // Decode to interleaved 24-bit RGB. libheif handles the YUV-to-RGB
    // conversion (including 10-bit HEVC sources downsampled to 8-bit),
    // alpha discard, and orientation -- callers receive the same RGB
    // bytes they'd get from libpng / libjpeg.
    if (heif_decode_image(handle, &himg, heif_colorspace_RGB,
                          heif_chroma_interleaved_RGB, NULL)
            .code != heif_error_Ok)
        goto cleanup;

    {
        int width = heif_image_get_width(himg, heif_channel_interleaved);
        int height = heif_image_get_height(himg, heif_channel_interleaved);
        if (width <= 0 || height <= 0) goto cleanup;

        int stride = 0;
        const uint8_t *plane = heif_image_get_plane_readonly(
            himg, heif_channel_interleaved, &stride);
        if (plane == NULL || stride < width * 3) goto cleanup;

        // libheif gives interleaved RGB; CImg stores planar. Build an
        // (channels, width, height) buffer then permute to the standard
        // (width, height, depth=1, channels) layout -- same trick used by
        // cimgffmpeg.cpp for FFmpeg-decoded frames.
        try {
            CImg<uint8_t> interleaved(3, width, height, 1, 0);
            for (int y = 0; y < height; y++) {
                const uint8_t *src = plane + (size_t)y * stride;
                uint8_t *dst = interleaved.data(0, 0, y, 0);
                for (int x = 0; x < width * 3; x++) dst[x] = src[x];
            }
            img = interleaved.get_permute_axes("yzcx");
        } catch (...) {
            goto cleanup;
        }

        rc = 0;
    }

cleanup:
    if (himg) heif_image_release(himg);
    if (handle) heif_image_handle_release(handle);
    heif_context_free(ctx);
    return rc;
}

#endif /* HAVE_HEIF */

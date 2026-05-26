/*
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.0 of the PHP license,       |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_0.txt.                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Evan Klinger <eklinger@phash.org>                           |
   +----------------------------------------------------------------------+
*/

#ifndef PHP_PHASH_H
#define PHP_PHASH_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <php.h>

#ifdef HAVE_PHASH
#define PHP_PHASH_VERSION "1.0.0"

#include <SAPI.h>
#include <Zend/zend_extensions.h>
#include <php_ini.h>
#include <ext/standard/info.h>

#include <pHash.h>
#ifdef HAVE_AUDIO_HASH
#include <audiophash.h>
#endif

extern zend_module_entry pHash_module_entry;
#define phpext_pHash_ptr &pHash_module_entry

#ifdef PHP_WIN32
#define PHP_PHASH_API __declspec(dllexport)
#else
#define PHP_PHASH_API
#endif

PHP_MINIT_FUNCTION(pHash);
PHP_MSHUTDOWN_FUNCTION(pHash);
PHP_RINIT_FUNCTION(pHash);
PHP_RSHUTDOWN_FUNCTION(pHash);
PHP_MINFO_FUNCTION(pHash);

/* ------------------------------------------------------------------ */
/* arg_info -- PHP 8.x ZEND_ARG_*_INFO style                          */
/* ------------------------------------------------------------------ */

#ifdef HAVE_VIDEO_HASH
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ph_dct_videohash_arg_info, 0, 1, IS_MIXED, 0)
    ZEND_ARG_TYPE_INFO(0, file, IS_STRING, 0)
ZEND_END_ARG_INFO()
PHP_FUNCTION(ph_dct_videohash);

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ph_video_dist_arg_info, 0, 2, IS_DOUBLE, 0)
    ZEND_ARG_INFO(0, h1)
    ZEND_ARG_INFO(0, h2)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, thresh, IS_LONG, 0, "21")
ZEND_END_ARG_INFO()
PHP_FUNCTION(ph_video_dist);
#endif /* HAVE_VIDEO_HASH */

#ifdef HAVE_IMAGE_HASH
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ph_dct_imagehash_arg_info, 0, 1, IS_MIXED, 0)
    ZEND_ARG_TYPE_INFO(0, file, IS_STRING, 0)
ZEND_END_ARG_INFO()
PHP_FUNCTION(ph_dct_imagehash);

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ph_image_dist_arg_info, 0, 2, IS_DOUBLE, 0)
    ZEND_ARG_INFO(0, h1)
    ZEND_ARG_INFO(0, h2)
ZEND_END_ARG_INFO()
PHP_FUNCTION(ph_image_dist);

/* Marr-Hildreth wavelet image hash. */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ph_mh_imagehash_arg_info, 0, 1, IS_MIXED, 0)
    ZEND_ARG_TYPE_INFO(0, file, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, alpha, IS_DOUBLE, 0, "2.0")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, level, IS_DOUBLE, 0, "1.0")
ZEND_END_ARG_INFO()
PHP_FUNCTION(ph_mh_imagehash);

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ph_mh_imagehash_from_pixels_arg_info, 0, 4, IS_MIXED, 0)
    ZEND_ARG_TYPE_INFO(0, pixels,   IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, width,    IS_LONG,   0)
    ZEND_ARG_TYPE_INFO(0, height,   IS_LONG,   0)
    ZEND_ARG_TYPE_INFO(0, channels, IS_LONG,   0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, alpha, IS_DOUBLE, 0, "2.0")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, level, IS_DOUBLE, 0, "1.0")
ZEND_END_ARG_INFO()
PHP_FUNCTION(ph_mh_imagehash_from_pixels);

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ph_mh_dist_arg_info, 0, 2, IS_DOUBLE, 0)
    ZEND_ARG_INFO(0, h1)
    ZEND_ARG_INFO(0, h2)
ZEND_END_ARG_INFO()
PHP_FUNCTION(ph_mh_dist);

/* Radon-projection (radial) image hash. */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ph_radial_imagehash_arg_info, 0, 1, IS_MIXED, 0)
    ZEND_ARG_TYPE_INFO(0, file, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, sigma, IS_DOUBLE, 0, "3.5")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, gamma, IS_DOUBLE, 0, "1.0")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, N,     IS_LONG,   0, "180")
ZEND_END_ARG_INFO()
PHP_FUNCTION(ph_radial_imagehash);

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ph_radial_dist_arg_info, 0, 2, IS_DOUBLE, 0)
    ZEND_ARG_INFO(0, h1)
    ZEND_ARG_INFO(0, h2)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, threshold, IS_DOUBLE, 0, "0.9")
ZEND_END_ARG_INFO()
PHP_FUNCTION(ph_radial_dist);

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ph_compare_images_arg_info, 0, 2, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO(0, file1, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, file2, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, sigma,     IS_DOUBLE, 0, "3.5")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, gamma,     IS_DOUBLE, 0, "1.0")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, N,         IS_LONG,   0, "180")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, threshold, IS_DOUBLE, 0, "0.9")
ZEND_END_ARG_INFO()
PHP_FUNCTION(ph_compare_images);
#endif /* HAVE_IMAGE_HASH */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ph_texthash_arg_info, 0, 1, IS_MIXED, 0)
    ZEND_ARG_TYPE_INFO(0, file, IS_STRING, 0)
ZEND_END_ARG_INFO()
PHP_FUNCTION(ph_texthash);

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ph_compare_text_hashes_arg_info, 0, 2, IS_MIXED, 0)
    ZEND_ARG_INFO(0, h1)
    ZEND_ARG_INFO(0, h2)
ZEND_END_ARG_INFO()
PHP_FUNCTION(ph_compare_text_hashes);

#ifdef HAVE_AUDIO_HASH
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ph_audiohash_arg_info, 0, 1, IS_MIXED, 0)
    ZEND_ARG_TYPE_INFO(0, file, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, sample_rate, IS_LONG, 0, "5512")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, channels,    IS_LONG, 0, "1")
ZEND_END_ARG_INFO()
PHP_FUNCTION(ph_audiohash);

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ph_audio_dist_arg_info, 0, 2, IS_DOUBLE, 0)
    ZEND_ARG_INFO(0, h1)
    ZEND_ARG_INFO(0, h2)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, block_size, IS_LONG,   0, "256")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, thresh,     IS_DOUBLE, 0, "0.30")
ZEND_END_ARG_INFO()
PHP_FUNCTION(ph_audio_dist);
#endif /* HAVE_AUDIO_HASH */

#endif /* HAVE_PHASH */

#endif /* PHP_PHASH_H */

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

#include "php_pHash.h"

#ifdef HAVE_PHASH

/* ============================================================ */
/* Resource payload types                                       */
/* ============================================================ */

struct ph_audio_hash {
    uint32_t *hash;
    int len;
};
struct ph_video_hash {
    ulong64 *hash;
    int len;
};
struct ph_text_hash {
    TxtHashPoint *p;
    int count;
};
struct ph_mh_hash {
    uint8_t *hash;
    int len;
};
struct ph_radial_hash {
    uint8_t *coeffs;
    int size;
};

/* ============================================================ */
/* Resource destructors -- PHP 7+ signature (zend_resource*)    */
/* ============================================================ */

static int le_ph_video_hash;
static int le_ph_image_hash;
static int le_ph_audio_hash;
static int le_ph_txt_hash;
static int le_ph_mh_hash;
static int le_ph_radial_hash;

static void ph_video_hash_dtor(zend_resource *rsrc) {
    ph_video_hash *r = (ph_video_hash *)rsrc->ptr;
    if (r) {
        free(r->hash);
        free(r);
    }
}
static void ph_image_hash_dtor(zend_resource *rsrc) {
    ulong64 *r = (ulong64 *)rsrc->ptr;
    if (r) free(r);
}
static void ph_audio_hash_dtor(zend_resource *rsrc) {
    ph_audio_hash *r = (ph_audio_hash *)rsrc->ptr;
    if (r) {
        free(r->hash);
        free(r);
    }
}
static void ph_txt_hash_dtor(zend_resource *rsrc) {
    ph_text_hash *r = (ph_text_hash *)rsrc->ptr;
    if (r) {
        free(r->p);
        free(r);
    }
}
static void ph_mh_hash_dtor(zend_resource *rsrc) {
    ph_mh_hash *r = (ph_mh_hash *)rsrc->ptr;
    if (r) {
        free(r->hash);
        free(r);
    }
}
static void ph_radial_hash_dtor(zend_resource *rsrc) {
    ph_radial_hash *r = (ph_radial_hash *)rsrc->ptr;
    if (r) {
        free(r->coeffs);
        free(r);
    }
}

/* ============================================================ */
/* Module table                                                 */
/* ============================================================ */

zend_function_entry pHash_functions[] = {
#ifdef HAVE_VIDEO_HASH
    PHP_FE(ph_dct_videohash, ph_dct_videohash_arg_info)
    PHP_FE(ph_video_dist,    ph_video_dist_arg_info)
#endif
#ifdef HAVE_IMAGE_HASH
    PHP_FE(ph_dct_imagehash,             ph_dct_imagehash_arg_info)
    PHP_FE(ph_image_dist,                ph_image_dist_arg_info)
    PHP_FE(ph_mh_imagehash,              ph_mh_imagehash_arg_info)
    PHP_FE(ph_mh_imagehash_from_pixels,  ph_mh_imagehash_from_pixels_arg_info)
    PHP_FE(ph_mh_dist,                   ph_mh_dist_arg_info)
    PHP_FE(ph_radial_imagehash,          ph_radial_imagehash_arg_info)
    PHP_FE(ph_radial_dist,               ph_radial_dist_arg_info)
    PHP_FE(ph_compare_images,            ph_compare_images_arg_info)
#endif
    PHP_FE(ph_texthash,             ph_texthash_arg_info)
    PHP_FE(ph_compare_text_hashes,  ph_compare_text_hashes_arg_info)
#ifdef HAVE_AUDIO_HASH
    PHP_FE(ph_audiohash,  ph_audiohash_arg_info)
    PHP_FE(ph_audio_dist, ph_audio_dist_arg_info)
#endif
    PHP_FE_END
};

zend_module_entry pHash_module_entry = {
    STANDARD_MODULE_HEADER,
    "pHash",
    pHash_functions,
    PHP_MINIT(pHash),
    PHP_MSHUTDOWN(pHash),
    PHP_RINIT(pHash),
    PHP_RSHUTDOWN(pHash),
    PHP_MINFO(pHash),
    PHP_PHASH_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_PHASH
extern "C" {
ZEND_GET_MODULE(pHash)
}
#endif

PHP_MINIT_FUNCTION(pHash) {
    le_ph_video_hash  = zend_register_list_destructors_ex(
        ph_video_hash_dtor,  NULL, "ph_video_hash",  module_number);
    le_ph_image_hash  = zend_register_list_destructors_ex(
        ph_image_hash_dtor,  NULL, "ph_image_hash",  module_number);
    le_ph_audio_hash  = zend_register_list_destructors_ex(
        ph_audio_hash_dtor,  NULL, "ph_audio_hash",  module_number);
    le_ph_txt_hash    = zend_register_list_destructors_ex(
        ph_txt_hash_dtor,    NULL, "ph_txt_hash",    module_number);
    le_ph_mh_hash     = zend_register_list_destructors_ex(
        ph_mh_hash_dtor,     NULL, "ph_mh_hash",     module_number);
    le_ph_radial_hash = zend_register_list_destructors_ex(
        ph_radial_hash_dtor, NULL, "ph_radial_hash", module_number);
    return SUCCESS;
}
PHP_MSHUTDOWN_FUNCTION(pHash) { return SUCCESS; }
PHP_RINIT_FUNCTION(pHash)     { return SUCCESS; }
PHP_RSHUTDOWN_FUNCTION(pHash) { return SUCCESS; }

PHP_MINFO_FUNCTION(pHash) {
    php_info_print_table_start();
    php_info_print_table_row(2, "pHash support", "enabled");
    php_info_print_table_row(2, "Extension version", PHP_PHASH_VERSION);
#ifdef HAVE_IMAGE_HASH
    php_info_print_table_row(2, "Image hashes",
                             "DCT, Marr-Hildreth, Radial");
#endif
#ifdef HAVE_VIDEO_HASH
    php_info_print_table_row(2, "Video hash", "DCT (keyframe-based)");
#endif
#ifdef HAVE_AUDIO_HASH
    php_info_print_table_row(2, "Audio hash", "Haitsma-Kalker style");
#endif
    php_info_print_table_row(2, "Text hash", "Winnowing (Schleimer/Aiken)");
    php_info_print_table_end();
}

/* ============================================================ */
/* Helpers                                                       */
/* ============================================================ */

static void *fetch_res(zval *zv, const char *type_name, int le) {
    if (Z_TYPE_P(zv) != IS_RESOURCE) return NULL;
    return zend_fetch_resource(Z_RES_P(zv), type_name, le);
}

/* ============================================================ */
/* DCT video hash                                               */
/* ============================================================ */

#ifdef HAVE_VIDEO_HASH
PHP_FUNCTION(ph_dct_videohash) {
    char *file = NULL;
    size_t file_len = 0;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_PATH(file, file_len)
    ZEND_PARSE_PARAMETERS_END();

    int len = 0;
    ulong64 *video_hash = ph_dct_videohash(file, len);
    if (!video_hash) RETURN_FALSE;

    ph_video_hash *p = (ph_video_hash *)malloc(sizeof(ph_video_hash));
    if (!p) { free(video_hash); RETURN_FALSE; }
    p->hash = video_hash;
    p->len = len;
    RETURN_RES(zend_register_resource(p, le_ph_video_hash));
}

PHP_FUNCTION(ph_video_dist) {
    zval *zh1, *zh2;
    zend_long thresh = 21;
    ZEND_PARSE_PARAMETERS_START(2, 3)
        Z_PARAM_RESOURCE(zh1)
        Z_PARAM_RESOURCE(zh2)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(thresh)
    ZEND_PARSE_PARAMETERS_END();

    ph_video_hash *h1 = (ph_video_hash *)fetch_res(zh1, "ph_video_hash", le_ph_video_hash);
    ph_video_hash *h2 = (ph_video_hash *)fetch_res(zh2, "ph_video_hash", le_ph_video_hash);
    if (!h1 || !h2) RETURN_DOUBLE(-1);

    RETURN_DOUBLE(ph_dct_videohash_dist(h1->hash, h1->len, h2->hash, h2->len, (int)thresh));
}
#endif /* HAVE_VIDEO_HASH */

/* ============================================================ */
/* DCT image hash                                               */
/* ============================================================ */

#ifdef HAVE_IMAGE_HASH
PHP_FUNCTION(ph_dct_imagehash) {
    char *file = NULL;
    size_t file_len = 0;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_PATH(file, file_len)
    ZEND_PARSE_PARAMETERS_END();

    ulong64 *hash = (ulong64 *)malloc(sizeof(ulong64));
    if (!hash) RETURN_FALSE;
    if (ph_dct_imagehash(file, *hash) != 0) {
        free(hash);
        RETURN_FALSE;
    }
    RETURN_RES(zend_register_resource(hash, le_ph_image_hash));
}

PHP_FUNCTION(ph_image_dist) {
    zval *zh1, *zh2;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zh1)
        Z_PARAM_RESOURCE(zh2)
    ZEND_PARSE_PARAMETERS_END();

    ulong64 *h1 = (ulong64 *)fetch_res(zh1, "ph_image_hash", le_ph_image_hash);
    ulong64 *h2 = (ulong64 *)fetch_res(zh2, "ph_image_hash", le_ph_image_hash);
    if (!h1 || !h2) RETURN_DOUBLE(-1);

    RETURN_DOUBLE((double)ph_hamming_distance(*h1, *h2));
}

/* ------------------------------------------------------------ */
/* Marr-Hildreth wavelet hash                                   */
/* ------------------------------------------------------------ */

PHP_FUNCTION(ph_mh_imagehash) {
    char *file = NULL;
    size_t file_len = 0;
    double alpha = 2.0, level = 1.0;
    ZEND_PARSE_PARAMETERS_START(1, 3)
        Z_PARAM_PATH(file, file_len)
        Z_PARAM_OPTIONAL
        Z_PARAM_DOUBLE(alpha)
        Z_PARAM_DOUBLE(level)
    ZEND_PARSE_PARAMETERS_END();

    int N = 0;
    uint8_t *raw = ph_mh_imagehash(file, N, (float)alpha, (float)level);
    if (!raw || N <= 0) {
        free(raw);
        RETURN_FALSE;
    }
    ph_mh_hash *r = (ph_mh_hash *)malloc(sizeof(ph_mh_hash));
    if (!r) { free(raw); RETURN_FALSE; }
    r->hash = raw;
    r->len = N;
    RETURN_RES(zend_register_resource(r, le_ph_mh_hash));
}

PHP_FUNCTION(ph_mh_imagehash_from_pixels) {
    char *pixels = NULL;
    size_t pixels_len = 0;
    zend_long width = 0, height = 0, channels = 0;
    double alpha = 2.0, level = 1.0;
    ZEND_PARSE_PARAMETERS_START(4, 6)
        Z_PARAM_STRING(pixels, pixels_len)
        Z_PARAM_LONG(width)
        Z_PARAM_LONG(height)
        Z_PARAM_LONG(channels)
        Z_PARAM_OPTIONAL
        Z_PARAM_DOUBLE(alpha)
        Z_PARAM_DOUBLE(level)
    ZEND_PARSE_PARAMETERS_END();

    if (width <= 0 || height <= 0 ||
        (channels != 1 && channels != 3 && channels != 4)) {
        RETURN_FALSE;
    }
    if ((zend_long)pixels_len < width * height * channels) {
        RETURN_FALSE;
    }

    int N = 0;
    uint8_t *raw = ph_mh_imagehash_from_pixels(
        (const uint8_t *)pixels, (int)width, (int)height, (int)channels,
        &N, (float)alpha, (float)level);
    if (!raw || N <= 0) {
        free(raw);
        RETURN_FALSE;
    }
    ph_mh_hash *r = (ph_mh_hash *)malloc(sizeof(ph_mh_hash));
    if (!r) { free(raw); RETURN_FALSE; }
    r->hash = raw;
    r->len = N;
    RETURN_RES(zend_register_resource(r, le_ph_mh_hash));
}

PHP_FUNCTION(ph_mh_dist) {
    zval *zh1, *zh2;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zh1)
        Z_PARAM_RESOURCE(zh2)
    ZEND_PARSE_PARAMETERS_END();

    ph_mh_hash *h1 = (ph_mh_hash *)fetch_res(zh1, "ph_mh_hash", le_ph_mh_hash);
    ph_mh_hash *h2 = (ph_mh_hash *)fetch_res(zh2, "ph_mh_hash", le_ph_mh_hash);
    if (!h1 || !h2) RETURN_DOUBLE(-1);

    // Normalised Hamming distance in [0, 1].
    RETURN_DOUBLE(ph_hammingdistance2(h1->hash, h1->len, h2->hash, h2->len));
}

/* ------------------------------------------------------------ */
/* Radial (Radon-projection) hash                               */
/* ------------------------------------------------------------ */

PHP_FUNCTION(ph_radial_imagehash) {
    char *file = NULL;
    size_t file_len = 0;
    double sigma = 3.5, gamma = 1.0;
    zend_long N = 180;
    ZEND_PARSE_PARAMETERS_START(1, 4)
        Z_PARAM_PATH(file, file_len)
        Z_PARAM_OPTIONAL
        Z_PARAM_DOUBLE(sigma)
        Z_PARAM_DOUBLE(gamma)
        Z_PARAM_LONG(N)
    ZEND_PARSE_PARAMETERS_END();

    Digest d = {};
    if (ph_image_digest(file, sigma, gamma, d, (int)N) < 0) {
        free(d.coeffs);
        RETURN_FALSE;
    }
    ph_radial_hash *r = (ph_radial_hash *)malloc(sizeof(ph_radial_hash));
    if (!r) { free(d.coeffs); RETURN_FALSE; }
    r->coeffs = d.coeffs;
    r->size = d.size;
    RETURN_RES(zend_register_resource(r, le_ph_radial_hash));
}

PHP_FUNCTION(ph_radial_dist) {
    zval *zh1, *zh2;
    double threshold = 0.9;
    ZEND_PARSE_PARAMETERS_START(2, 3)
        Z_PARAM_RESOURCE(zh1)
        Z_PARAM_RESOURCE(zh2)
        Z_PARAM_OPTIONAL
        Z_PARAM_DOUBLE(threshold)
    ZEND_PARSE_PARAMETERS_END();

    ph_radial_hash *h1 = (ph_radial_hash *)fetch_res(zh1, "ph_radial_hash", le_ph_radial_hash);
    ph_radial_hash *h2 = (ph_radial_hash *)fetch_res(zh2, "ph_radial_hash", le_ph_radial_hash);
    if (!h1 || !h2) RETURN_DOUBLE(-1);

    Digest d1, d2;
    d1.id = NULL; d1.coeffs = h1->coeffs; d1.size = h1->size;
    d2.id = NULL; d2.coeffs = h2->coeffs; d2.size = h2->size;
    double pcc = 0;
    if (ph_crosscorr(d1, d2, pcc, threshold) < 0) RETURN_DOUBLE(-1);
    RETURN_DOUBLE(pcc);
}

PHP_FUNCTION(ph_compare_images) {
    char *file1 = NULL, *file2 = NULL;
    size_t file1_len = 0, file2_len = 0;
    double sigma = 3.5, gamma = 1.0, threshold = 0.9;
    zend_long N = 180;
    ZEND_PARSE_PARAMETERS_START(2, 6)
        Z_PARAM_PATH(file1, file1_len)
        Z_PARAM_PATH(file2, file2_len)
        Z_PARAM_OPTIONAL
        Z_PARAM_DOUBLE(sigma)
        Z_PARAM_DOUBLE(gamma)
        Z_PARAM_LONG(N)
        Z_PARAM_DOUBLE(threshold)
    ZEND_PARSE_PARAMETERS_END();

    double pcc = 0;
    int rc = ph_compare_images(file1, file2, pcc, sigma, gamma, (int)N, threshold);
    if (rc < 0) RETURN_DOUBLE(-1);
    RETURN_DOUBLE(pcc);
}
#endif /* HAVE_IMAGE_HASH */

/* ============================================================ */
/* Text hash                                                    */
/* ============================================================ */

PHP_FUNCTION(ph_texthash) {
    char *file = NULL;
    size_t file_len = 0;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_PATH(file, file_len)
    ZEND_PARSE_PARAMETERS_END();

    int num = 0;
    TxtHashPoint *txtHash = ph_texthash(file, &num);
    if (!txtHash) RETURN_FALSE;

    ph_text_hash *h = (ph_text_hash *)malloc(sizeof(ph_text_hash));
    if (!h) { free(txtHash); RETURN_FALSE; }
    h->p = txtHash;
    h->count = num;
    RETURN_RES(zend_register_resource(h, le_ph_txt_hash));
}

PHP_FUNCTION(ph_compare_text_hashes) {
    zval *zh1, *zh2;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zh1)
        Z_PARAM_RESOURCE(zh2)
    ZEND_PARSE_PARAMETERS_END();

    ph_text_hash *h1 = (ph_text_hash *)fetch_res(zh1, "ph_txt_hash", le_ph_txt_hash);
    ph_text_hash *h2 = (ph_text_hash *)fetch_res(zh2, "ph_txt_hash", le_ph_txt_hash);
    if (!h1 || !h2) RETURN_FALSE;

    int count = 0;
    TxtMatch *m = ph_compare_text_hashes(h1->p, h1->count, h2->p, h2->count, &count);
    if (!m) RETURN_FALSE;

    array_init(return_value);
    for (int i = 0; i < count; ++i) {
        zval entry;
        array_init(&entry);
        add_assoc_long(&entry, "begin",  (zend_long)m[i].first_index);
        add_assoc_long(&entry, "end",    (zend_long)m[i].second_index);
        add_assoc_long(&entry, "length", (zend_long)m[i].length);
        add_next_index_zval(return_value, &entry);
    }
    free(m);
}

/* ============================================================ */
/* Audio hash                                                   */
/* ============================================================ */

#ifdef HAVE_AUDIO_HASH
PHP_FUNCTION(ph_audiohash) {
    char *file = NULL;
    size_t file_len = 0;
    zend_long sample_rate = 5512, channels = 1;
    ZEND_PARSE_PARAMETERS_START(1, 3)
        Z_PARAM_PATH(file, file_len)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(sample_rate)
        Z_PARAM_LONG(channels)
    ZEND_PARSE_PARAMETERS_END();

    int n = 0;
    float *audiobuf = ph_readaudio(file, (int)sample_rate, (int)channels, NULL, n);
    if (!audiobuf) RETURN_FALSE;

    int nb_frames = 0;
    uint32_t *hash = ph_audiohash(audiobuf, n, (int)sample_rate, nb_frames);
    free(audiobuf);
    if (!hash || nb_frames <= 0) {
        free(hash);
        RETURN_FALSE;
    }

    ph_audio_hash *h = (ph_audio_hash *)malloc(sizeof(ph_audio_hash));
    if (!h) { free(hash); RETURN_FALSE; }
    h->hash = hash;
    h->len = nb_frames;
    RETURN_RES(zend_register_resource(h, le_ph_audio_hash));
}

PHP_FUNCTION(ph_audio_dist) {
    zval *zh1, *zh2;
    zend_long block_size = 256;
    double thresh = 0.30;
    ZEND_PARSE_PARAMETERS_START(2, 4)
        Z_PARAM_RESOURCE(zh1)
        Z_PARAM_RESOURCE(zh2)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(block_size)
        Z_PARAM_DOUBLE(thresh)
    ZEND_PARSE_PARAMETERS_END();

    ph_audio_hash *h1 = (ph_audio_hash *)fetch_res(zh1, "ph_audio_hash", le_ph_audio_hash);
    ph_audio_hash *h2 = (ph_audio_hash *)fetch_res(zh2, "ph_audio_hash", le_ph_audio_hash);
    if (!h1 || !h2) RETURN_DOUBLE(-1);

    int Nc = 0;
    double *cs = ph_audio_distance_ber(h1->hash, h1->len, h2->hash, h2->len,
                                       (float)thresh, (int)block_size, Nc);
    if (!cs) RETURN_DOUBLE(-1);

    double max_cs = 0.0;
    for (int i = 0; i < Nc; ++i) {
        if (cs[i] > max_cs) max_cs = cs[i];
    }
    free(cs);
    RETURN_DOUBLE(max_cs);
}
#endif /* HAVE_AUDIO_HASH */

#endif /* HAVE_PHASH */

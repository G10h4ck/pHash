# pHash PHP binding

A PHP 8.x extension that exposes every public entry point of the pHash
C++ library.

| C++ API                     | PHP function                                 | Returns                |
| ---                         | ---                                          | ---                    |
| `ph_dct_imagehash`          | `ph_dct_imagehash($file)`                    | `resource(ph_image_hash)` or `false` |
| `ph_hamming_distance`       | `ph_image_dist($h1, $h2)`                    | `float`                |
| `ph_mh_imagehash`           | `ph_mh_imagehash($file [, $alpha, $level])`  | `resource(ph_mh_hash)` or `false` |
| `ph_mh_imagehash_from_pixels` | `ph_mh_imagehash_from_pixels($pixels, $w, $h, $channels [, $alpha, $level])` | `resource(ph_mh_hash)` or `false` |
| `ph_hammingdistance2`       | `ph_mh_dist($h1, $h2)`                       | `float` (0..1)         |
| `ph_image_digest`           | `ph_radial_imagehash($file [, $sigma, $gamma, $N])` | `resource(ph_radial_hash)` or `false` |
| `ph_crosscorr`              | `ph_radial_dist($h1, $h2 [, $threshold])`    | `float` (pcc)          |
| `ph_compare_images`         | `ph_compare_images($file1, $file2 [, $sigma, $gamma, $N, $threshold])` | `float` (pcc)          |
| `ph_texthash`               | `ph_texthash($file)`                         | `resource(ph_txt_hash)` or `false` |
| `ph_compare_text_hashes`    | `ph_compare_text_hashes($h1, $h2)`           | `array` of `[begin, end, length]` |
| `ph_audiohash`              | `ph_audiohash($file [, $sample_rate, $channels])` | `resource(ph_audio_hash)` or `false` |
| `ph_audio_distance_ber`     | `ph_audio_dist($h1, $h2 [, $block_size, $thresh])` | `float`                |
| `ph_dct_videohash`          | `ph_dct_videohash($file)`                    | `resource(ph_video_hash)` or `false` |
| `ph_dct_videohash_dist`     | `ph_video_dist($h1, $h2 [, $thresh])`        | `float`                |

## Requirements

- PHP 7.0+ (rewritten for the modern Zend API; tested with PHP 8.5)
- libpHash 1.0+ installed (`pHash.h`, `audiophash.h` if audio enabled,
  `CImg.h`, `libpHash.dylib`/`.so`)
- A C++14 compiler

## Build

```sh
# Build and install libpHash:
cd /path/to/pHash
mkdir -p build && cd build
cmake -DWITH_AUDIO_HASH=ON -DWITH_VIDEO_HASH=ON ..
make
sudo make install              # installs pHash.h, audiophash.h, ph_fft.h,
                               # CImg.h, libpHash.dylib, pHash.pc

# Build the PHP extension:
cd ../bindings/php
phpize
./configure --with-pHash=/usr/local
make
sudo make install
```

Enable it in your `php.ini`:

```ini
extension=pHash.so
```

Confirm:

```sh
php -m | grep -i phash
php -r 'foreach (["ph_dct_imagehash","ph_mh_imagehash","ph_radial_imagehash",
                  "ph_compare_images","ph_audiohash","ph_dct_videohash",
                  "ph_texthash"] as $f) printf("%-26s %s\n", $f,
                  function_exists($f) ? "ok" : "MISSING");'
```

## Usage

```php
// DCT hash: 64-bit perceptual hash, small Hamming distance => similar
$h1 = ph_dct_imagehash('cat.jpg');
$h2 = ph_dct_imagehash('cat_resized.jpg');
echo ph_image_dist($h1, $h2), "\n";  // 0..64

// Marr-Hildreth wavelet hash: 576 bits, normalised distance 0..1
$m1 = ph_mh_imagehash('cat.jpg');
$m2 = ph_mh_imagehash('cat_blurred.jpg');
echo ph_mh_dist($m1, $m2), "\n";     // 0.0..1.0

// Radial / Radon hash: rotation-invariant
$r1 = ph_radial_imagehash('cat.jpg');
$r2 = ph_radial_imagehash('cat_rotated.jpg');
echo ph_radial_dist($r1, $r2), "\n"; // peak cross-correlation

// Shortcut: direct file-to-file radial comparison
echo ph_compare_images('cat.jpg', 'cat_rotated.jpg'), "\n";

// Hash an in-memory image (no temp file needed):
$pixels = file_get_contents('rgb_pixels.bin');     // width*height*3 bytes
$mh = ph_mh_imagehash_from_pixels($pixels, 248, 432, 3);

// Audio: identical = 1.0, unrelated ~= 0.5
$a1 = ph_audiohash('track.wav');
$a2 = ph_audiohash('track_remix.wav');
echo ph_audio_dist($a1, $a2), "\n";

// Video: keyframe-based DCT hash
$v1 = ph_dct_videohash('clip.mp4');
$v2 = ph_dct_videohash('clip_reencoded.mp4');
echo ph_video_dist($v1, $v2), "\n";

// Text: winnowing
$t1 = ph_texthash('document.txt');
$t2 = ph_texthash('document_revised.txt');
foreach (ph_compare_text_hashes($t1, $t2) as $m) {
    printf("match @ %d..%d (len %d)\n", $m['begin'], $m['end'], $m['length']);
}
```

## Notes

- All hash resources are reference-counted by Zend and freed automatically
  when they go out of scope.
- The phpinfo logo registration from the PHP 4/5 era is gone (the
  `php_register_info_logo` API was removed in PHP 7).

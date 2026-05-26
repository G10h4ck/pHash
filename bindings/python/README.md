# pHash Python binding

A Cython binding for the image-hash entry points of [pHash](https://github.com/aetilius/pHash). Currently exposes:

| C++ API | Python class / method |
|---|---|
| `ph_dct_imagehash` | `DCTImageHash.from_path(path)` |
| `ph_mh_imagehash` | `MHImageHash.from_path(path, alpha=2.0, level=1.0)` |
| `ph_mh_imagehash_from_pixels` | `MHImageHash.from_pixels(pixels, w, h, channels)` |
| `ph_hamming_distance` (64-bit) | `DCTImageHash.hamming_distance(other)` |
| `ph_hammingdistance2` (byte array) | `MHImageHash.hamming_distance(other)` |

Audio, video, text, and the Radon-based image hash are not yet bound. PRs welcome.

## Requirements

- Python 3.7+
- Cython (`pip install cython`)
- A C++14 compiler
- The pHash native dependencies: `libpng`, `libjpeg`, `libtiff`, `zlib`

## Build

```sh
# 1. Configure the source tree once so CMake generates src/pHash.h
#    (the C++ public header is a CMake template).
cd ../..               # to the project root
mkdir -p build && cd build && cmake .. && cd ..

# 2. Build the Cython extension in place.
cd bindings/python
pip install cython
python setup.py build_ext --inplace
```

Produces `phash*.so` (Linux/macOS) or `phash*.pyd` (Windows) next to `phash.pyx`.

## Usage

```python
from phash import DCTImageHash, MHImageHash

# DCT hash (64-bit perceptual hash; small Hamming distance = visually similar)
h1 = DCTImageHash.from_path("cat.jpg")
h2 = DCTImageHash.from_path("cat_resized.jpg")
print(h1.hamming_distance(h2))    # int 0..64

# Marr-Hildreth wavelet hash (576-bit; reported as percentage 0..100)
m1 = MHImageHash.from_path("cat.jpg")
m2 = MHImageHash.from_path("dog.jpg")
print(m1.hamming_distance(m2))    # int 0..100 (smaller = more similar)

# Hashing an image already held in memory (no temp file round-trip)
import numpy as np
from PIL import Image
img = np.asarray(Image.open("cat.jpg").convert("RGB"))  # H x W x 3 uint8
h, w, c = img.shape
mh = MHImageHash.from_pixels(img.tobytes(), w, h, c)
```

`DCTImageHash` and `MHImageHash` are picklable, so you can build an index and store it:

```python
import pickle
with open("hashes.pkl", "wb") as f:
    pickle.dump(hashes, f)
```

See `phash_add_directory.py` and `phash_show_similar.py` for runnable example scripts.

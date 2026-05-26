"""
Build the pHash Python extension.

Builds an in-tree extension by recompiling src/pHash.cpp into the Cython
extension; we don't need a separate libpHash.so for the image-only
binding. Audio / video hash features are intentionally not exposed yet.

Before running this, configure the source tree at least once so the
CMake-generated src/pHash.h exists:

    mkdir build && cd build && cmake .. && cd ..

Then:

    python setup.py build_ext --inplace
"""
import os
from setuptools import setup
from distutils.extension import Extension
from Cython.Build import cythonize

HERE = os.path.abspath(os.path.dirname(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))

phash_ext = Extension(
    'phash',
    sources=[
        os.path.join(HERE, 'phash.pyx'),
        os.path.join(PROJECT_ROOT, 'src', 'pHash.cpp'),
    ],
    include_dirs=[
        # CImg.h is bundled and used by pHash.cpp.
        os.path.join(PROJECT_ROOT, 'third-party', 'CImg'),
        os.path.join(PROJECT_ROOT, 'src'),
    ],
    libraries=['png', 'jpeg', 'tiff', 'z'],
    # HAVE_IMAGE_HASH comes from the CMake-configured pHash.h, so we don't
    # need to redefine it here. The build will fail with a clear error if
    # pHash.h hasn't been configured (see the README for that step).
    language='c++',
    extra_compile_args=['-std=c++14', '-Wno-deprecated-declarations'],
)

setup(
    name='phash',
    description='Python bindings for the pHash perceptual hashing library',
    ext_modules=cythonize([phash_ext], language_level='3'),
)

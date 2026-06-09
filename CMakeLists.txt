cmake_minimum_required (VERSION 3.14)

include(ExternalProject)
include(GNUInstallDirs)

project(pHash VERSION 1.0.0)
set(CMAKE_MACOSX_RPATH 1)
set(pHash_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(pHash_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(pHash_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(VERSION ${PROJECT_VERSION})

OPTION(PHASH_DYNAMIC   "Build pHash dynamic library"                     ON)
OPTION(PHASH_STATIC    "Build pHash static library"                     OFF)
OPTION(WITH_AUDIO_HASH "Audio hash support"                             OFF)
OPTION(WITH_VIDEO_HASH "Video hash support"                             OFF)
OPTION(WITH_HEIF       "HEIF/HEIC/AVIF image support via libheif"       OFF)
OPTION(PHASH_BINDINGS  "Compile foreign programming languages bindings" OFF)
OPTION(PHASH_EXAMPLES  "Compile examples"                               OFF)

set(HAVE_IMAGE_HASH 1)

if(WITH_AUDIO_HASH)
    set(HAVE_AUDIO_HASH 1)
endif()

if(WITH_VIDEO_HASH)
    set(HAVE_VIDEO_HASH 1)
endif()

if(WITH_HEIF)
    find_package(PkgConfig)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(LIBHEIF libheif)
    endif()
    if(NOT LIBHEIF_FOUND)
        # Fallback: look for the header and library directly under common
        # prefixes. libheif's pkg-config is sometimes missing on Linux
        # distros that ship the package as `libheif-dev`.
        find_path(LIBHEIF_INCLUDE_DIRS libheif/heif.h
                  PATHS /opt/homebrew/include /usr/local/include /usr/include)
        find_library(LIBHEIF_LIBRARIES NAMES heif libheif
                     PATHS /opt/homebrew/lib /usr/local/lib /usr/lib)
        if(LIBHEIF_INCLUDE_DIRS AND LIBHEIF_LIBRARIES)
            set(LIBHEIF_FOUND TRUE)
        endif()
    endif()
    if(NOT LIBHEIF_FOUND)
        message(FATAL_ERROR
            "WITH_HEIF=ON but libheif could not be located. "
            "Install libheif (Homebrew: `brew install libheif`, "
            "Debian: `apt install libheif-dev`) or set -DWITH_HEIF=OFF.")
    endif()
    set(HAVE_HEIF 1)
    message(STATUS "libheif found: ${LIBHEIF_INCLUDE_DIRS} ${LIBHEIF_LIBRARIES}")
endif()

find_path(CIMG_H_DIR NAMES CImg.h PATHS "${CMAKE_CURRENT_SOURCE_DIR}/third-party/CImg")
include_directories(${CIMG_H_DIR})

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/pHash.h.cmake ${CMAKE_CURRENT_SOURCE_DIR}/src/pHash.h)

# Populate pHash.pc.in's @VERSION@/@prefix@/etc. so the installed pkg-config
# file actually has a real version (the template was previously copied but
# never configured).
set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix ${CMAKE_INSTALL_PREFIX})
set(libdir ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
set(includedir ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/pHash.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/pHash.pc @ONLY)


if(NOT WIN32)
       set(MMAN_FILE "")
       set(DIRENT_FILE "")
    EXEC_PROGRAM(uname ARGS -m OUTPUT_VARIABLE BUILD_SYSTEM)
    EXEC_PROGRAM(uname ARGS -s OUTPUT_VARIABLE CMAKE_SYSTEM_NAME)
else()
    set(MMAN_FILE "win/mman.cpp")
    set(DIRENT_FILE "win/dirent.cpp")
    add_definitions("-D_EXPORTING")
    include_directories("win")
endif()

if(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif()

if(PHASH_BINDINGS)
  add_subdirectory(bindings)
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BUILD_TYPE}")

# Pick up libpng / libjpeg / libtiff / etc. from common Homebrew prefixes
# (/opt/homebrew on Apple Silicon, /usr/local on Intel) and from system paths.
foreach(_prefix /opt/homebrew /usr/local)
    if(IS_DIRECTORY ${_prefix}/lib)
        link_directories(${_prefix}/lib)
    endif()
    if(IS_DIRECTORY ${_prefix}/include)
        include_directories(${_prefix}/include)
    endif()
endforeach()


set(LIBS_DEPS png z jpeg tiff)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
set(LIBS_DEPS ${LIBS_DEPS} Threads::Threads)

find_library(LIBMPG123 mpg123)
if(LIBMPG123)
    message("libmpg123 found at ${LIBMPG123}")
    set(HAVE_LIBMPG123 1)
    list(APPEND LIBS_DEPS ${LIBMPG123})
endif()

if(HAVE_AUDIO_HASH)
    set(EXTRA_SRC src/audiophash.cpp src/ph_fft.cpp)
    set(LIBS_DEPS ${LIBS_DEPS} sndfile samplerate vorbis vorbisenc ogg)
endif()

if(HAVE_VIDEO_HASH)
    list(APPEND EXTRA_SRC src/cimgffmpeg.cpp)
    set(LIBS_DEPS ${LIBS_DEPS} avcodec avformat swscale avutil)
endif()

# ph_heif.cpp is always compiled (it carries the ph_is_heif_path
# extension probe, which the load_image helper in pHash.cpp needs to
# distinguish HEIF paths). The libheif-using body inside it is gated
# behind HAVE_HEIF in the source, so the file builds cleanly even when
# libheif isn't installed.
list(APPEND EXTRA_SRC src/ph_heif.cpp)
if(HAVE_HEIF)
    include_directories(${LIBHEIF_INCLUDE_DIRS})
    list(APPEND LIBS_DEPS ${LIBHEIF_LIBRARIES})
endif()


if(PHASH_DYNAMIC)
	add_library(pHash SHARED ${MMAN_FILE} ${DIRENT_FILE} src/pHash.cpp ${EXTRA_SRC})
endif()

if(PHASH_STATIC)
	add_library(pHash STATIC ${MMAN_FILE} ${DIRENT_FILE} src/pHash.cpp ${EXTRA_SRC})
endif()

set_property(TARGET pHash PROPERTY VERSION "${pHash_VERSION_MAJOR}.${pHash_VERSION_MINOR}.${pHash_VERSION_PATCH}")
target_link_libraries(pHash ${LIBS_DEPS})

install(TARGETS pHash DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES src/pHash.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
# pHash.h #includes CImg.h (its public Projections struct exposes a
# CImg<> *). Consumers therefore need CImg.h on their include path; ship
# the bundled copy alongside.
install(FILES ${CIMG_H_DIR}/CImg.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES src/ph_heif.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
if(HAVE_AUDIO_HASH)
    install(FILES src/audiophash.h src/ph_fft.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pHash.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

if(PHASH_EXAMPLES)
set(EXAMPLEDIR examples)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

if(HAVE_IMAGE_HASH)
add_executable(TestDCT "${EXAMPLEDIR}/test_imagephash.cpp")
target_link_libraries (TestDCT pHash)
install(TARGETS TestDCT DESTINATION bin)

add_executable(TestRadish "${EXAMPLEDIR}/test_radish.cpp")
target_link_libraries (TestRadish pHash)
install(TARGETS TestRadish DESTINATION bin)

add_executable(TestMH "${EXAMPLEDIR}/test_mhimagehash.cpp")
target_link_libraries (TestMH pHash)
install(TARGETS TestMH DESTINATION bin)
endif()

if(HAVE_AUDIO_HASH)
add_executable(TestAudio "${EXAMPLEDIR}/test_audiophash.cpp")
target_link_libraries (TestAudio pHash)
install(TARGETS TestAudio DESTINATION bin)
endif()

if(HAVE_VIDEO_HASH)
add_executable(TestVideoHash "${EXAMPLEDIR}/test_dctvideohash.cpp")
target_link_libraries (TestVideoHash pHash)
install(TARGETS TestVideoHash DESTINATION bin)
endif()
endif() # PHASH_EXAMPLES

include (InstallRequiredSystemLibraries)

set(CPACK_GENERATOR "TGZ")
set(CPACK_SOURCE_GENERATOR "TGZ")
set (CPACK_PACKAGE_VERSION_MAJOR "${pHash_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${pHash_VERSION_MINOR}")

include (CPack)

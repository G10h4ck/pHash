dnl
dnl $ Id: $
dnl

PHP_ARG_WITH(pHash, whether pHash is available,[  --with-pHash[=DIR]   With pHash support])


if test "$PHP_PHASH" != "no"; then
  PHP_REQUIRE_CXX
  AC_LANG_CPLUSPLUS
  PHP_ADD_LIBRARY(stdc++,,PHASH_SHARED_LIBADD)


  if test -r "$PHP_PHASH/include/pHash.h"; then
	PHP_PHASH_DIR="$PHP_PHASH"
  else
	AC_MSG_CHECKING(for pHash in default path)
	for i in /usr /usr/local; do
	  if test -r "$i/include/pHash.h"; then
		PHP_PHASH_DIR=$i
		AC_MSG_RESULT(found in $i)
		break
	  fi
	done
	if test "x" = "x$PHP_PHASH_DIR"; then
	  AC_MSG_ERROR(not found)
	fi
  fi

  PHP_ADD_INCLUDE($PHP_PHASH_DIR/include)
  # CImg.h (transitively included by pHash.h) needs png.h / jpeg.h / tiff.h.
  # Pick those up from common Homebrew prefixes so the conftest compiles
  # and the real extension build works without a separate -CFLAGS setting.
  for _phash_extra_inc in /opt/homebrew/include /usr/local/include; do
    if test -d "$_phash_extra_inc"; then
      PHP_ADD_INCLUDE($_phash_extra_inc)
      _phash_extra_incs="$_phash_extra_incs -I$_phash_extra_inc"
    fi
  done

  export OLD_CPPFLAGS="$CPPFLAGS"
  # pHash.h sets HAVE_IMAGE_HASH itself when CMake configured it that way;
  # don't redefine it here.
  export CPPFLAGS="$CPPFLAGS -I$PHP_PHASH_DIR/include $_phash_extra_incs -DHAVE_PHASH"
  AC_CHECK_HEADER([pHash.h], [], AC_MSG_ERROR('pHash.h' header not found))
  # audiophash.h is only required when the audio extension is enabled in
  # libpHash. Probe for it, but don't fail if it's absent.
  AC_CHECK_HEADER([audiophash.h],
                  [AC_DEFINE(HAVE_AUDIO_HASH, 1, [audio hash])],
                  [AC_MSG_WARN([audiophash.h not found -- audio hash bindings disabled])])
  PHP_SUBST(PHASH_SHARED_LIBADD)


  PHP_CHECK_LIBRARY(pHash, ph_texthash,
  [
	PHP_ADD_LIBRARY_WITH_PATH(pHash, $PHP_PHASH_DIR/lib, PHASH_SHARED_LIBADD)
  ],[
	AC_MSG_ERROR([wrong pHash lib version or lib not found])
  ],[
	-L$PHP_PHASH_DIR/lib
  ])
  export CPPFLAGS="$OLD_CPPFLAGS"

  export OLD_CPPFLAGS="$CPPFLAGS"
  export CPPFLAGS="$CPPFLAGS $INCLUDES -DHAVE_PHASH"

  AC_MSG_CHECKING(PHP version)
  AC_TRY_COMPILE([#include <php_version.h>], [
#if PHP_VERSION_ID < 70000
#error  this extension requires at least PHP 7.0
#endif
],
[AC_MSG_RESULT(ok)],
[AC_MSG_ERROR([need at least PHP 7.0 (the binding was rewritten for the modern Zend API)])])

  export CPPFLAGS="$OLD_CPPFLAGS"


  PHP_SUBST(PHASH_SHARED_LIBADD)
  AC_DEFINE(HAVE_PHASH, 1, [ ])

  PHP_NEW_EXTENSION(pHash, pHash.cpp , $ext_shared)

fi


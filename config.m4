dnl $Id$
dnl config.m4 for extension cabocha

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(cabocha, for cabocha support,
dnl Make sure that the comment is aligned:
dnl [  --with-cabocha             Include cabocha support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(cabocha, whether to enable cabocha support,
dnl Make sure that the comment is aligned:
dnl [  --enable-cabocha           Enable cabocha support])

if test "$PHP_CABOCHA" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-cabocha -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/cabocha.h"  # you most likely want to change this
  dnl if test -r $PHP_CABOCHA/$SEARCH_FOR; then # path given as parameter
  dnl   CABOCHA_DIR=$PHP_CABOCHA
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for cabocha files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       CABOCHA_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$CABOCHA_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the cabocha distribution])
  dnl fi

  dnl # --with-cabocha -> add include path
  dnl PHP_ADD_INCLUDE($CABOCHA_DIR/include)

  dnl # --with-cabocha -> check for lib and symbol presence
  dnl LIBNAME=cabocha # you may want to change this
  dnl LIBSYMBOL=cabocha # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $CABOCHA_DIR/$PHP_LIBDIR, CABOCHA_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_CABOCHALIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong cabocha lib version or lib not found])
  dnl ],[
  dnl   -L$CABOCHA_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(CABOCHA_SHARED_LIBADD)

  PHP_NEW_EXTENSION(cabocha, cabocha.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi

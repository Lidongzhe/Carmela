dnl $Id$
dnl config.m4 for extension carmela

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(carmela, for carmela support,
Make sure that the comment is aligned:
[  --with-carmela             Include carmela support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(carmela, whether to enable carmela support,
dnl Make sure that the comment is aligned:
dnl [  --enable-carmela           Enable carmela support])

if test "$PHP_CARMELA" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-carmela -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/carmela.h"  # you most likely want to change this
  dnl if test -r $PHP_CARMELA/$SEARCH_FOR; then # path given as parameter
  dnl   CARMELA_DIR=$PHP_CARMELA
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for carmela files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       CARMELA_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$CARMELA_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the carmela distribution])
  dnl fi

  dnl # --with-carmela -> add include path
  dnl PHP_ADD_INCLUDE($CARMELA_DIR/include)

  dnl # --with-carmela -> check for lib and symbol presence
  dnl LIBNAME=carmela # you may want to change this
  dnl LIBSYMBOL=carmela # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $CARMELA_DIR/lib, CARMELA_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_CARMELALIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong carmela lib version or lib not found])
  dnl ],[
  dnl   -L$CARMELA_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(CARMELA_SHARED_LIBADD)

  PHP_NEW_EXTENSION(carmela, carmela.c, $ext_shared)
fi

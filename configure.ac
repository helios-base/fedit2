#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(fedit2, [20160613], akky@users.sourceforge.jp)

AC_CONFIG_AUX_DIR([config])
#AM_INIT_AUTOMAKE([gnu check-news])
AM_INIT_AUTOMAKE([gnu])

AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_LANG([C++])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)

# Checks for libraries.

# boost
AX_BOOST_BASE([1.38.0])
#AX_BOOST_UNIT_TEST_FRAMEWORK
CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"

# Qt4
AC_MSG_NOTICE(enabled Qt4)
QT4MODULES="QtCore QtGui QtNetwork"

AC_ARG_ENABLE(gl,
              AC_HELP_STRING([--enable-gl],
                             [enable GLWidget for Qt4 (default no)]))
if test "x$enable_gl" == "xyes"; then
  AC_MSG_NOTICE(enabled GLWidget)
  CFLAGS="-DUSE_GLWIDGET $CFLAGS"
  CXXFLAGS="-DUSE_GLWIDGET $CXXFLAGS"
  QT4MODULES="$QT4MODULES QtOpenGL"
fi

AX_QT4([4.3.0],[$QT4MODULES])
if test x$have_qt4 != xyes ; then
  AC_MSG_ERROR([The Qt4 library could not be found.])
fi


# check librcsc path
AC_SUBST(LIBRCSCLIB)
LIBRCSCLIB="/usr/local/lib"
librcsc_prefix=""
AC_ARG_WITH(librcsc,
            [  --with-librcsc=PREFIX prefix where librcsc is installed (optional)],
            librcsc_prefix="$withval",
            librcsc_prefix="")
if test x$librcsc_prefix != x; then
  CPPFLAGS="$CPPFLAGS -I$librcsc_prefix/include"
  LDFLAGS="$LDFLAGS -L$librcsc_prefix/lib"
  LIBRCSCLIB="$librcsc_prefix/lib"
else
  for librcsc_path_tmp in $HOME $HOME/local $HOME/rcss /opt/robocup /opt /opt/local ; do
    if test -d "$librcsc_path_tmp/include/rcsc" && test -r "$librcsc_path_tmp/include/rcsc/types.h" ; then
      CPPFLAGS="$CPPFLAGS -I$librcsc_path_tmp/include"
      LDFLAGS="$LDFLAGS -L$librcsc_path_tmp/lib"
      LIBRCSCLIB="$librcsc_path_tmp/lib"
      break;
    fi
  done
fi

# Checks for libraries.
AC_CHECK_LIB([m], [cos],
             [LIBS="-lm $LIBS"],
             [AC_MSG_ERROR([*** -lm not found! ***])])
AC_CHECK_LIB([rcsc_geom], [main],
             [LIBS="-lrcsc_geom $LIBS"],
             [AC_MSG_ERROR([*** -lrcsc_geom not found! ***])])
AC_CHECK_LIB([rcsc_rcg], [main],
             [LIBS="-lrcsc_rcg $LIBS"],
             [AC_MSG_ERROR([*** -lrcsc_rcg not found! ***])])
AC_CHECK_LIB([rcsc_gz], [main])
AC_CHECK_LIB([rcsc_param], [main],
             [LIBS="-lrcsc_param $LIBS"],
             [AC_MSG_ERROR([*** -lrcsc_param not found! ***])])
AC_CHECK_LIB([rcsc_net], [main],
             [LIBS="-lrcsc_net $LIBS"],
             [AC_MSG_ERROR([*** -lrcsc_net not found! ***])])
AC_CHECK_LIB([rcsc_time], [main],
             [LIBS="-lrcsc_time $LIBS"],
             [AC_MSG_ERROR([*** -lrcsc_time not found! ***])])
AC_CHECK_LIB([rcsc_ann], [main],
             [LIBS="-lrcsc_ann $LIBS"],
             [AC_MSG_ERROR([*** -lrcsc_ann not found! ***])])
AC_CHECK_LIB([rcsc_clang], [main],
             [LIBS="-lrcsc_clang $LIBS"],
             [AC_MSG_ERROR([*** -lrcsc_ann not found! ***])])
AC_CHECK_LIB([rcsc_agent], [main],
             [LIBS="-lrcsc_agent $LIBS"],
             [AC_MSG_ERROR([*** -lrcsc_agent not found! ***])])


# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_HEADER_STDC
AC_CHECK_FUNCS([sqrt rint])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 tool/Makefile
                 tool/dttrainer/Makefile])

AC_OUTPUT

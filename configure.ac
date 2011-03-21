AC_INIT([elc], [0.1], [s.glass@nicta.com.au])
AM_INIT_AUTOMAKE([-Wall -Werror])
AC_CONFIG_MACRO_DIR([m4])

# Checks for prereequisite programs
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL

# You what?
# AC_LANG(C++)

# Check for prerequisite libraries
AX_BOOST_BASE([1.40], , AC_MSG_ERROR([boost not found.]))
#if test "$HAS_BOOST" != "yes"; then
#	AC_MSG_ERROR([boost is required.])
#fi

AX_BOOST_SIGNALS
AX_BOOST_THREAD
AX_BOOST_UNIT_TEST_FRAMEWORK

CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"

# Check header files
AC_CHECK_HEADERS([])

# Check required libraries
AC_CHECK_LIB([nl], [nl_cache_free, nl_socket_alloc, nl_socket_free], ,AC_MSG_ERROR([libnl2-dev not found!]))
AC_CHECK_LIB([nl-genl], [genl_connect, genl_ctrl_alloc_cache, genl_ctrl_search_by_name, genl_family_put], ,AC_MSG_ERROR([libnl-genl not found!]))

# Compile source files in their directories (non-recursive make)
AM_INIT_AUTOMAKE([subdir-objects])

# Generate these output files
AC_CONFIG_FILES([Makefile lib/Makefile src/Makefile src/elc/Makefile])

AC_OUTPUT

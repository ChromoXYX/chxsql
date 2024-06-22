AC_INIT([0.0], [chxsql])
AM_INIT_AUTOMAKE([-Werror -Wall foreign])

AC_SUBST(__CHXSQL_VERSION, ["0.0"])

AC_SUBST(__CHXSQL_TEST_LDADD)
AC_SUBST(__CHXSQL_TEST_CXXFLAGS)

AC_ARG_WITH([liburing],
    AS_HELP_STRING([--with-liburing=<liburing-DIR>],
        [use liburing located in <liburing-DIR>]),
[
    AC_MSG_CHECKING([for include/liburing.h in $withval])
    AS_IF([test -f "$withval/include/liburing.h"], [
        AC_MSG_RESULT([yes])
        __CHXSQL_TEST_CXXFLAGS="$__CHXSQL_TEST_CXXFLAGS -I$withval/include"

        AC_MSG_CHECKING([for lib/liburing.a in $withval])
        AS_IF([test -f "$withval/lib/liburing.a"], [
            AC_MSG_RESULT([yes])
            __CHXSQL_TEST_LDADD="$__CHXSQL_TEST_LDADD $withval/lib/liburing.a"
        ], [
            AC_MSG_RESULT([no])
            AC_MSG_ERROR([cannot find lib/liburing.a in $withval])
        ])
    ], [
        AC_MSG_RESULT([no])
        AC_MSG_ERROR([cannot find include/liburing.h in $withval])
    ])
], [
    AC_CHECK_HEADER([liburing.h], [], [
        AC_MSG_ERROR([cannot find liburing.h in system include dirs])
    ])
    AC_CHECK_LIB([uring], [io_uring_queue_init], [
        __CHXSQL_TEST_LDADD="$__CHXSQL_TEST_LDADD -luring"
    ], [
        AC_MSG_ERROR([cannot link against liburing, -luring failed])
    ])
]
)

AX_BOOST_BASE([1.75.0], [
    __CHXSQL_TEST_CXXFLAGS="$__CHXSQL_TEST_CXXFLAGS $BOOST_CPPFLAGS"
    __CHXSQL_TEST_LDADD="$__CHXSQL_TEST_LDADD $BOOST_LDFLAGS"
], [
    AC_MSG_ERROR(["cannot find boost"])
])

AC_PROG_CXX
AC_CONFIG_FILES([
    Makefile
    include/Makefile
    test/Makefile
])
AC_OUTPUT
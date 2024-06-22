AM_CXXFLAGS = -std=c++20 -I../chxnet/include -I../chxser2/include -I../include @__CHXSQL_TEST_CXXFLAGS@
LDADD = @__CHXSQL_TEST_LDADD@
check_PROGRAMS = \
    test

test_SOURCES = test.cpp

TESTS = $(check_PROGRAMS)
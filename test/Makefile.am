AM_CXXFLAGS = -std=c++17 -I../chxnet/include -I../chxlog/include -I../include @__CHXSQL_TEST_CXXFLAGS@
LDADD = @__CHXSQL_TEST_LDADD@
check_PROGRAMS = \
    test

test_SOURCES = test.cpp

TESTS = $(check_PROGRAMS)
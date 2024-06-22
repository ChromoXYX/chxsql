#pragma once

#include <system_error>

namespace chx::sql::mysql {
struct errc {
    enum errc_impl : int {
        EE_CANTCREATEFILE = 1,
        EE_READ = 2,
        EE_WRITE = 3,
        EE_BADCLOSE = 4,
        EE_OUTOFMEMORY = 5,
        EE_DELETE = 6,
        EE_LINK = 7,
        EE_EOFERR = 9,
        EE_CANTLOCK = 10,
        EE_CANTUNLOCK = 11,
        EE_DIR = 12,
        EE_STAT = 13,
        EE_CANT_CHSIZE = 14,
        EE_CANT_OPEN_STREAM = 15,
        EE_GETWD = 16,
        EE_SETWD = 17,
        EE_LINK_WARNING = 18,
        EE_OPEN_WARNING = 19,
        EE_DISK_FULL = 20,
        EE_CANT_MKDIR = 21,
        EE_UNKNOWN_CHARSET = 22,
        EE_OUT_OF_FILERESOURCES = 23,
        EE_CANT_READLINK = 24,
        EE_CANT_SYMLINK = 25,
        EE_REALPATH = 26,
        EE_SYNC = 27,
        EE_UNKNOWN_COLLATION = 28,
        EE_FILENOTFOUND = 29,
        EE_FILE_NOT_CLOSED = 30,
        EE_CHANGE_OWNERSHIP = 31,
        EE_CHANGE_PERMISSIONS = 32,
        EE_CANT_SEEK = 33,
        EE_CAPACITY_EXCEEDED = 34,
        EE_DISK_FULL_WITH_RETRY_MSG = 35,
        EE_FAILED_TO_CREATE_TIMER = 36,
        EE_FAILED_TO_DELETE_TIMER = 37,
        EE_FAILED_TO_CREATE_TIMER_QUEUE = 38,
        EE_FAILED_TO_START_TIMER_NOTIFY_THREAD = 39,
        EE_FAILED_TO_CREATE_TIMER_NOTIFY_THREAD_INTERRUPT_EVENT = 40,
        EE_EXITING_TIMER_NOTIFY_THREAD = 41,
        EE_WIN_LIBRARY_LOAD_FAILED = 42,
        EE_WIN_RUN_TIME_ERROR_CHECK = 43,
        EE_FAILED_TO_DETERMINE_LARGE_PAGE_SIZE = 44,
        EE_FAILED_TO_KILL_ALL_THREADS = 45,
        EE_FAILED_TO_CREATE_IO_COMPLETION_PORT = 46,
        EE_FAILED_TO_OPEN_DEFAULTS_FILE = 47,
        EE_FAILED_TO_HANDLE_DEFAULTS_FILE = 48,
        EE_WRONG_DIRECTIVE_IN_CONFIG_FILE = 49,
        EE_SKIPPING_DIRECTIVE_DUE_TO_MAX_INCLUDE_RECURSION = 50,
        EE_INCORRECT_GRP_DEFINITION_IN_CONFIG_FILE = 51,
        EE_OPTION_WITHOUT_GRP_IN_CONFIG_FILE = 52,
        EE_CONFIG_FILE_PERMISSION_ERROR = 53,
        EE_IGNORE_WORLD_WRITABLE_CONFIG_FILE = 54,
        EE_USING_DISABLED_OPTION = 55,
        EE_USING_DISABLED_SHORT_OPTION = 56,
        EE_USING_PASSWORD_ON_CLI_IS_INSECURE = 57,
        EE_UNKNOWN_SUFFIX_FOR_VARIABLE = 58,
        EE_SSL_ERROR_FROM_FILE = 59,
        EE_SSL_ERROR = 60,
        EE_NET_SEND_ERROR_IN_BOOTSTRAP = 61,
        EE_PACKETS_OUT_OF_ORDER = 62,
        EE_UNKNOWN_PROTOCOL_OPTION = 63,
        EE_FAILED_TO_LOCATE_SERVER_PUBLIC_KEY = 64,
        EE_PUBLIC_KEY_NOT_IN_PEM_FORMAT = 65,
        EE_DEBUG_INFO = 66,
        EE_UNKNOWN_VARIABLE = 67,
        EE_UNKNOWN_OPTION = 68,
        EE_UNKNOWN_SHORT_OPTION = 69,
        EE_OPTION_WITHOUT_ARGUMENT = 70,
        EE_OPTION_REQUIRES_ARGUMENT = 71,
        EE_SHORT_OPTION_REQUIRES_ARGUMENT = 72,
        EE_OPTION_IGNORED_DUE_TO_INVALID_VALUE = 73,
        EE_OPTION_WITH_EMPTY_VALUE = 74,
        EE_FAILED_TO_ASSIGN_MAX_VALUE_TO_OPTION = 75,
        EE_INCORRECT_BOOLEAN_VALUE_FOR_OPTION = 76,
        EE_FAILED_TO_SET_OPTION_VALUE = 77,
        EE_INCORRECT_INT_VALUE_FOR_OPTION = 78,
        EE_INCORRECT_UINT_VALUE_FOR_OPTION = 79,
        EE_ADJUSTED_SIGNED_VALUE_FOR_OPTION = 80,
        EE_ADJUSTED_UNSIGNED_VALUE_FOR_OPTION = 81,
        EE_ADJUSTED_ULONGLONG_VALUE_FOR_OPTION = 82,
        EE_ADJUSTED_DOUBLE_VALUE_FOR_OPTION = 83,
        EE_INVALID_DECIMAL_VALUE_FOR_OPTION = 84,
        EE_COLLATION_PARSER_ERROR = 85,
        EE_FAILED_TO_RESET_BEFORE_PRIMARY_IGNORABLE_CHAR = 86,
        EE_FAILED_TO_RESET_BEFORE_TERTIARY_IGNORABLE_CHAR = 87,
        EE_SHIFT_CHAR_OUT_OF_RANGE = 88,
        EE_RESET_CHAR_OUT_OF_RANGE = 89,
        EE_UNKNOWN_LDML_TAG = 90,
        EE_FAILED_TO_RESET_BEFORE_SECONDARY_IGNORABLE_CHAR = 91,
        EE_FAILED_PROCESSING_DIRECTIVE = 92,
        EE_PTHREAD_KILL_FAILED = 93,

        CR_UNKNOWN_ERROR = 2000,
        CR_SOCKET_CREATE_ERROR = 2001,
        CR_CONNECTION_ERROR = 2002,
        CR_CONN_HOST_ERROR = 2003,
        CR_IPSOCK_ERROR = 2004,
        CR_UNKNOWN_HOST = 2005,
        CR_SERVER_GONE_ERROR = 2006,
        CR_VERSION_ERROR = 2007,
        CR_OUT_OF_MEMORY = 2008,
        CR_WRONG_HOST_INFO = 2009,
        CR_LOCALHOST_CONNECTION = 2010,
        CR_TCP_CONNECTION = 2011,
        CR_SERVER_HANDSHAKE_ERR = 2012,
        CR_SERVER_LOST = 2013,
        CR_NAMEDPIPE_CONNECTION = 2015,
        CR_NAMEDPIPEWAIT_ERROR = 2016,
        CR_NAMEDPIPEOPEN_ERROR = 2017,
        CR_NAMEDPIPESETSTATE_ERROR = 2018,
        CR_CANT_READ_CHARSET = 2019,
        CR_NET_PACKET_TOO_LARGE = 2020,
        CR_EMBEDDED_CONNECTION = 2021,
        CR_PROBE_REPLICA_STATUS = 2022,
        CR_PROBE_REPLICA_HOSTS = 2023,
        CR_PROBE_REPLICA_CONNECT = 2024,
        CR_PROBE_SOURCE_CONNECT = 2025,
        CR_SSL_CONNECTION_ERROR = 2026,
        CR_MALFORMED_PACKET = 2027,
        CR_WRONG_LICENSE = 2028,
        CR_NULL_POINTER = 2029,
        CR_NO_PREPARE_STMT = 2030,
        CR_PARAMS_NOT_BOUND = 2031,
        CR_DATA_TRUNCATED = 2032,
        CR_NO_PARAMETERS_EXISTS = 2033,
        CR_INVALID_BUFFER_USE = 2035,
        CR_UNSUPPORTED_PARAM_TYPE = 2036,
        CR_SHARED_MEMORY_CONNECTION = 2037,
        CR_SHARED_MEMORY_CONNECT_REQUEST_ERROR = 2038,
        CR_SHARED_MEMORY_CONNECT_ANSWER_ERROR = 2039,
        CR_SHARED_MEMORY_CONNECT_FILE_MAP_ERROR = 2040,
        CR_SHARED_MEMORY_CONNECT_MAP_ERROR = 2041,
        CR_SHARED_MEMORY_FILE_MAP_ERROR = 2042,
        CR_SHARED_MEMORY_MAP_ERROR = 2043,
        CR_SHARED_MEMORY_EVENT_ERROR = 2044,
        CR_SHARED_MEMORY_CONNECT_ABANDONED_ERROR = 2045,
        CR_SHARED_MEMORY_CONNECT_SET_ERROR = 2046,
        CR_CONN_UNKNOW_PROTOCOL = 2047,
        CR_INVALID_CONN_HANDLE = 2048,
        CR_UNUSED_1 = 2049,
        CR_FETCH_CANCELED = 2050,
        CR_NO_DATA = 2051,
        CR_NO_STMT_METADATA = 2052,
        CR_NO_RESULT_SET = 2053,
        CR_NOT_IMPLEMENTED = 2054,
        CR_SERVER_LOST_EXTENDED = 2055,
        CR_STMT_CLOSED = 2056,
        CR_NEW_STMT_METADATA = 2057,
        CR_ALREADY_CONNECTED = 2058,
        CR_AUTH_PLUGIN_CANNOT_LOAD = 2059,
        CR_AUTH_PLUGIN_ERR = 2061,
        CR_INSECURE_API_ERR = 2062,
        CR_FILE_NAME_TOO_LONG = 2063,
        CR_SSL_FIPS_MODE_ERR = 2064,
        CR_DEPRECATED_COMPRESSION_NOT_SUPPORTED = 2065,
        CR_COMPRESSION_WRONGLY_CONFIGURED = 2066,
        CR_KERBEROS_USER_NOT_FOUND = 2067,
        CR_LOAD_DATA_LOCAL_INFILE_REJECTED = 2068,
        CR_LOAD_DATA_LOCAL_INFILE_REALPATH_FAIL = 2069,
        CR_DNS_SRV_LOOKUP_FAILED = 2070,
        CR_MANDATORY_TRACKER_NOT_FOUND = 2071,
        CR_INVALID_FACTOR_NO = 2072,
        CR_CANT_GET_SESSION_DATA = 2073,
        CR_INVALID_CLIENT_CHARSET = 2074,
        CR_TLS_SERVER_NOT_FOUND = 2075
    };
};
}  // namespace chx::sql::mysql

namespace std {
template <>
struct is_error_condition_enum<::chx::sql::mysql::errc::errc_impl>
    : std::true_type {};
}  // namespace std

namespace chx::sql::mysql {
inline std::error_category& error_category() {
    class __category : public std::error_category {
      public:
        const char* name() const noexcept(true) override {
            return "chxsql.mysql error_category";
        }

        std::error_condition default_error_condition(int ev) const
            noexcept(true) override {
            return std::error_condition(ev, mysql::error_category());
        }

        bool equivalent(const std::error_code& ec, int ev) const
            noexcept(true) override {
            return *this == ec.category() && static_cast<int>(ec.value()) == ev;
        }

        std::string message(int ev) const override {
            switch (ev) {
            case 0: {
                return "No Error";
            }
            case errc::EE_CANTCREATEFILE:
                return "EE_CANTCREATEFILE";
            case errc::EE_READ:
                return "EE_READ";
            case errc::EE_WRITE:
                return "EE_WRITE";
            case errc::EE_BADCLOSE:
                return "EE_BADCLOSE";
            case errc::EE_OUTOFMEMORY:
                return "EE_OUTOFMEMORY";
            case errc::EE_DELETE:
                return "EE_DELETE";
            case errc::EE_LINK:
                return "EE_LINK";
            case errc::EE_EOFERR:
                return "EE_EOFERR";
            case errc::EE_CANTLOCK:
                return "EE_CANTLOCK";
            case errc::EE_CANTUNLOCK:
                return "EE_CANTUNLOCK";
            case errc::EE_DIR:
                return "EE_DIR";
            case errc::EE_STAT:
                return "EE_STAT";
            case errc::EE_CANT_CHSIZE:
                return "EE_CANT_CHSIZE";
            case errc::EE_CANT_OPEN_STREAM:
                return "EE_CANT_OPEN_STREAM";
            case errc::EE_GETWD:
                return "EE_GETWD";
            case errc::EE_SETWD:
                return "EE_SETWD";
            case errc::EE_LINK_WARNING:
                return "EE_LINK_WARNING";
            case errc::EE_OPEN_WARNING:
                return "EE_OPEN_WARNING";
            case errc::EE_DISK_FULL:
                return "EE_DISK_FULL";
            case errc::EE_CANT_MKDIR:
                return "EE_CANT_MKDIR";
            case errc::EE_UNKNOWN_CHARSET:
                return "EE_UNKNOWN_CHARSET";
            case errc::EE_OUT_OF_FILERESOURCES:
                return "EE_OUT_OF_FILERESOURCES";
            case errc::EE_CANT_READLINK:
                return "EE_CANT_READLINK";
            case errc::EE_CANT_SYMLINK:
                return "EE_CANT_SYMLINK";
            case errc::EE_REALPATH:
                return "EE_REALPATH";
            case errc::EE_SYNC:
                return "EE_SYNC";
            case errc::EE_UNKNOWN_COLLATION:
                return "EE_UNKNOWN_COLLATION";
            case errc::EE_FILENOTFOUND:
                return "EE_FILENOTFOUND";
            case errc::EE_FILE_NOT_CLOSED:
                return "EE_FILE_NOT_CLOSED";
            case errc::EE_CHANGE_OWNERSHIP:
                return "EE_CHANGE_OWNERSHIP";
            case errc::EE_CHANGE_PERMISSIONS:
                return "EE_CHANGE_PERMISSIONS";
            case errc::EE_CANT_SEEK:
                return "EE_CANT_SEEK";
            case errc::EE_CAPACITY_EXCEEDED:
                return "EE_CAPACITY_EXCEEDED";
            case errc::EE_DISK_FULL_WITH_RETRY_MSG:
                return "EE_DISK_FULL_WITH_RETRY_MSG";
            case errc::EE_FAILED_TO_CREATE_TIMER:
                return "EE_FAILED_TO_CREATE_TIMER";
            case errc::EE_FAILED_TO_DELETE_TIMER:
                return "EE_FAILED_TO_DELETE_TIMER";
            case errc::EE_FAILED_TO_CREATE_TIMER_QUEUE:
                return "EE_FAILED_TO_CREATE_TIMER_QUEUE";
            case errc::EE_FAILED_TO_START_TIMER_NOTIFY_THREAD:
                return "EE_FAILED_TO_START_TIMER_NOTIFY_THREAD";
            case errc::EE_FAILED_TO_CREATE_TIMER_NOTIFY_THREAD_INTERRUPT_EVENT:
                return "EE_FAILED_TO_CREATE_TIMER_NOTIFY_THREAD_INTERRUPT_"
                       "EVENT";
            case errc::EE_EXITING_TIMER_NOTIFY_THREAD:
                return "EE_EXITING_TIMER_NOTIFY_THREAD";
            case errc::EE_WIN_LIBRARY_LOAD_FAILED:
                return "EE_WIN_LIBRARY_LOAD_FAILED";
            case errc::EE_WIN_RUN_TIME_ERROR_CHECK:
                return "EE_WIN_RUN_TIME_ERROR_CHECK";
            case errc::EE_FAILED_TO_DETERMINE_LARGE_PAGE_SIZE:
                return "EE_FAILED_TO_DETERMINE_LARGE_PAGE_SIZE";
            case errc::EE_FAILED_TO_KILL_ALL_THREADS:
                return "EE_FAILED_TO_KILL_ALL_THREADS";
            case errc::EE_FAILED_TO_CREATE_IO_COMPLETION_PORT:
                return "EE_FAILED_TO_CREATE_IO_COMPLETION_PORT";
            case errc::EE_FAILED_TO_OPEN_DEFAULTS_FILE:
                return "EE_FAILED_TO_OPEN_DEFAULTS_FILE";
            case errc::EE_FAILED_TO_HANDLE_DEFAULTS_FILE:
                return "EE_FAILED_TO_HANDLE_DEFAULTS_FILE";
            case errc::EE_WRONG_DIRECTIVE_IN_CONFIG_FILE:
                return "EE_WRONG_DIRECTIVE_IN_CONFIG_FILE";
            case errc::EE_SKIPPING_DIRECTIVE_DUE_TO_MAX_INCLUDE_RECURSION:
                return "EE_SKIPPING_DIRECTIVE_DUE_TO_MAX_INCLUDE_RECURSION";
            case errc::EE_INCORRECT_GRP_DEFINITION_IN_CONFIG_FILE:
                return "EE_INCORRECT_GRP_DEFINITION_IN_CONFIG_FILE";
            case errc::EE_OPTION_WITHOUT_GRP_IN_CONFIG_FILE:
                return "EE_OPTION_WITHOUT_GRP_IN_CONFIG_FILE";
            case errc::EE_CONFIG_FILE_PERMISSION_ERROR:
                return "EE_CONFIG_FILE_PERMISSION_ERROR";
            case errc::EE_IGNORE_WORLD_WRITABLE_CONFIG_FILE:
                return "EE_IGNORE_WORLD_WRITABLE_CONFIG_FILE";
            case errc::EE_USING_DISABLED_OPTION:
                return "EE_USING_DISABLED_OPTION";
            case errc::EE_USING_DISABLED_SHORT_OPTION:
                return "EE_USING_DISABLED_SHORT_OPTION";
            case errc::EE_USING_PASSWORD_ON_CLI_IS_INSECURE:
                return "EE_USING_PASSWORD_ON_CLI_IS_INSECURE";
            case errc::EE_UNKNOWN_SUFFIX_FOR_VARIABLE:
                return "EE_UNKNOWN_SUFFIX_FOR_VARIABLE";
            case errc::EE_SSL_ERROR_FROM_FILE:
                return "EE_SSL_ERROR_FROM_FILE";
            case errc::EE_SSL_ERROR:
                return "EE_SSL_ERROR";
            case errc::EE_NET_SEND_ERROR_IN_BOOTSTRAP:
                return "EE_NET_SEND_ERROR_IN_BOOTSTRAP";
            case errc::EE_PACKETS_OUT_OF_ORDER:
                return "EE_PACKETS_OUT_OF_ORDER";
            case errc::EE_UNKNOWN_PROTOCOL_OPTION:
                return "EE_UNKNOWN_PROTOCOL_OPTION";
            case errc::EE_FAILED_TO_LOCATE_SERVER_PUBLIC_KEY:
                return "EE_FAILED_TO_LOCATE_SERVER_PUBLIC_KEY";
            case errc::EE_PUBLIC_KEY_NOT_IN_PEM_FORMAT:
                return "EE_PUBLIC_KEY_NOT_IN_PEM_FORMAT";
            case errc::EE_DEBUG_INFO:
                return "EE_DEBUG_INFO";
            case errc::EE_UNKNOWN_VARIABLE:
                return "EE_UNKNOWN_VARIABLE";
            case errc::EE_UNKNOWN_OPTION:
                return "EE_UNKNOWN_OPTION";
            case errc::EE_UNKNOWN_SHORT_OPTION:
                return "EE_UNKNOWN_SHORT_OPTION";
            case errc::EE_OPTION_WITHOUT_ARGUMENT:
                return "EE_OPTION_WITHOUT_ARGUMENT";
            case errc::EE_OPTION_REQUIRES_ARGUMENT:
                return "EE_OPTION_REQUIRES_ARGUMENT";
            case errc::EE_SHORT_OPTION_REQUIRES_ARGUMENT:
                return "EE_SHORT_OPTION_REQUIRES_ARGUMENT";
            case errc::EE_OPTION_IGNORED_DUE_TO_INVALID_VALUE:
                return "EE_OPTION_IGNORED_DUE_TO_INVALID_VALUE";
            case errc::EE_OPTION_WITH_EMPTY_VALUE:
                return "EE_OPTION_WITH_EMPTY_VALUE";
            case errc::EE_FAILED_TO_ASSIGN_MAX_VALUE_TO_OPTION:
                return "EE_FAILED_TO_ASSIGN_MAX_VALUE_TO_OPTION";
            case errc::EE_INCORRECT_BOOLEAN_VALUE_FOR_OPTION:
                return "EE_INCORRECT_BOOLEAN_VALUE_FOR_OPTION";
            case errc::EE_FAILED_TO_SET_OPTION_VALUE:
                return "EE_FAILED_TO_SET_OPTION_VALUE";
            case errc::EE_INCORRECT_INT_VALUE_FOR_OPTION:
                return "EE_INCORRECT_INT_VALUE_FOR_OPTION";
            case errc::EE_INCORRECT_UINT_VALUE_FOR_OPTION:
                return "EE_INCORRECT_UINT_VALUE_FOR_OPTION";
            case errc::EE_ADJUSTED_SIGNED_VALUE_FOR_OPTION:
                return "EE_ADJUSTED_SIGNED_VALUE_FOR_OPTION";
            case errc::EE_ADJUSTED_UNSIGNED_VALUE_FOR_OPTION:
                return "EE_ADJUSTED_UNSIGNED_VALUE_FOR_OPTION";
            case errc::EE_ADJUSTED_ULONGLONG_VALUE_FOR_OPTION:
                return "EE_ADJUSTED_ULONGLONG_VALUE_FOR_OPTION";
            case errc::EE_ADJUSTED_DOUBLE_VALUE_FOR_OPTION:
                return "EE_ADJUSTED_DOUBLE_VALUE_FOR_OPTION";
            case errc::EE_INVALID_DECIMAL_VALUE_FOR_OPTION:
                return "EE_INVALID_DECIMAL_VALUE_FOR_OPTION";
            case errc::EE_COLLATION_PARSER_ERROR:
                return "EE_COLLATION_PARSER_ERROR";
            case errc::EE_FAILED_TO_RESET_BEFORE_PRIMARY_IGNORABLE_CHAR:
                return "EE_FAILED_TO_RESET_BEFORE_PRIMARY_IGNORABLE_CHAR";
            case errc::EE_FAILED_TO_RESET_BEFORE_TERTIARY_IGNORABLE_CHAR:
                return "EE_FAILED_TO_RESET_BEFORE_TERTIARY_IGNORABLE_CHAR";
            case errc::EE_SHIFT_CHAR_OUT_OF_RANGE:
                return "EE_SHIFT_CHAR_OUT_OF_RANGE";
            case errc::EE_RESET_CHAR_OUT_OF_RANGE:
                return "EE_RESET_CHAR_OUT_OF_RANGE";
            case errc::EE_UNKNOWN_LDML_TAG:
                return "EE_UNKNOWN_LDML_TAG";
            case errc::EE_FAILED_TO_RESET_BEFORE_SECONDARY_IGNORABLE_CHAR:
                return "EE_FAILED_TO_RESET_BEFORE_SECONDARY_IGNORABLE_CHAR";
            case errc::EE_FAILED_PROCESSING_DIRECTIVE:
                return "EE_FAILED_PROCESSING_DIRECTIVE";
            case errc::EE_PTHREAD_KILL_FAILED:
                return "EE_PTHREAD_KILL_FAILED";
            case errc::CR_UNKNOWN_ERROR:
                return "CR_UNKNOWN_ERROR";
            case errc::CR_SOCKET_CREATE_ERROR:
                return "CR_SOCKET_CREATE_ERROR";
            case errc::CR_CONNECTION_ERROR:
                return "CR_CONNECTION_ERROR";
            case errc::CR_CONN_HOST_ERROR:
                return "CR_CONN_HOST_ERROR";
            case errc::CR_IPSOCK_ERROR:
                return "CR_IPSOCK_ERROR";
            case errc::CR_UNKNOWN_HOST:
                return "CR_UNKNOWN_HOST";
            case errc::CR_SERVER_GONE_ERROR:
                return "CR_SERVER_GONE_ERROR";
            case errc::CR_VERSION_ERROR:
                return "CR_VERSION_ERROR";
            case errc::CR_OUT_OF_MEMORY:
                return "CR_OUT_OF_MEMORY";
            case errc::CR_WRONG_HOST_INFO:
                return "CR_WRONG_HOST_INFO";
            case errc::CR_LOCALHOST_CONNECTION:
                return "CR_LOCALHOST_CONNECTION";
            case errc::CR_TCP_CONNECTION:
                return "CR_TCP_CONNECTION";
            case errc::CR_SERVER_HANDSHAKE_ERR:
                return "CR_SERVER_HANDSHAKE_ERR";
            case errc::CR_SERVER_LOST:
                return "CR_SERVER_LOST";
            case errc::CR_NAMEDPIPE_CONNECTION:
                return "CR_NAMEDPIPE_CONNECTION";
            case errc::CR_NAMEDPIPEWAIT_ERROR:
                return "CR_NAMEDPIPEWAIT_ERROR";
            case errc::CR_NAMEDPIPEOPEN_ERROR:
                return "CR_NAMEDPIPEOPEN_ERROR";
            case errc::CR_NAMEDPIPESETSTATE_ERROR:
                return "CR_NAMEDPIPESETSTATE_ERROR";
            case errc::CR_CANT_READ_CHARSET:
                return "CR_CANT_READ_CHARSET";
            case errc::CR_NET_PACKET_TOO_LARGE:
                return "CR_NET_PACKET_TOO_LARGE";
            case errc::CR_EMBEDDED_CONNECTION:
                return "CR_EMBEDDED_CONNECTION";
            case errc::CR_PROBE_REPLICA_STATUS:
                return "CR_PROBE_REPLICA_STATUS";
            case errc::CR_PROBE_REPLICA_HOSTS:
                return "CR_PROBE_REPLICA_HOSTS";
            case errc::CR_PROBE_REPLICA_CONNECT:
                return "CR_PROBE_REPLICA_CONNECT";
            case errc::CR_PROBE_SOURCE_CONNECT:
                return "CR_PROBE_SOURCE_CONNECT";
            case errc::CR_SSL_CONNECTION_ERROR:
                return "CR_SSL_CONNECTION_ERROR";
            case errc::CR_MALFORMED_PACKET:
                return "CR_MALFORMED_PACKET";
            case errc::CR_WRONG_LICENSE:
                return "CR_WRONG_LICENSE";
            case errc::CR_NULL_POINTER:
                return "CR_NULL_POINTER";
            case errc::CR_NO_PREPARE_STMT:
                return "CR_NO_PREPARE_STMT";
            case errc::CR_PARAMS_NOT_BOUND:
                return "CR_PARAMS_NOT_BOUND";
            case errc::CR_DATA_TRUNCATED:
                return "CR_DATA_TRUNCATED";
            case errc::CR_NO_PARAMETERS_EXISTS:
                return "CR_NO_PARAMETERS_EXISTS";
            case errc::CR_INVALID_BUFFER_USE:
                return "CR_INVALID_BUFFER_USE";
            case errc::CR_UNSUPPORTED_PARAM_TYPE:
                return "CR_UNSUPPORTED_PARAM_TYPE";
            case errc::CR_SHARED_MEMORY_CONNECTION:
                return "CR_SHARED_MEMORY_CONNECTION";
            case errc::CR_SHARED_MEMORY_CONNECT_REQUEST_ERROR:
                return "CR_SHARED_MEMORY_CONNECT_REQUEST_ERROR";
            case errc::CR_SHARED_MEMORY_CONNECT_ANSWER_ERROR:
                return "CR_SHARED_MEMORY_CONNECT_ANSWER_ERROR";
            case errc::CR_SHARED_MEMORY_CONNECT_FILE_MAP_ERROR:
                return "CR_SHARED_MEMORY_CONNECT_FILE_MAP_ERROR";
            case errc::CR_SHARED_MEMORY_CONNECT_MAP_ERROR:
                return "CR_SHARED_MEMORY_CONNECT_MAP_ERROR";
            case errc::CR_SHARED_MEMORY_FILE_MAP_ERROR:
                return "CR_SHARED_MEMORY_FILE_MAP_ERROR";
            case errc::CR_SHARED_MEMORY_MAP_ERROR:
                return "CR_SHARED_MEMORY_MAP_ERROR";
            case errc::CR_SHARED_MEMORY_EVENT_ERROR:
                return "CR_SHARED_MEMORY_EVENT_ERROR";
            case errc::CR_SHARED_MEMORY_CONNECT_ABANDONED_ERROR:
                return "CR_SHARED_MEMORY_CONNECT_ABANDONED_ERROR";
            case errc::CR_SHARED_MEMORY_CONNECT_SET_ERROR:
                return "CR_SHARED_MEMORY_CONNECT_SET_ERROR";
            case errc::CR_CONN_UNKNOW_PROTOCOL:
                return "CR_CONN_UNKNOW_PROTOCOL";
            case errc::CR_INVALID_CONN_HANDLE:
                return "CR_INVALID_CONN_HANDLE";
            case errc::CR_UNUSED_1:
                return "CR_UNUSED_1";
            case errc::CR_FETCH_CANCELED:
                return "CR_FETCH_CANCELED";
            case errc::CR_NO_DATA:
                return "CR_NO_DATA";
            case errc::CR_NO_STMT_METADATA:
                return "CR_NO_STMT_METADATA";
            case errc::CR_NO_RESULT_SET:
                return "CR_NO_RESULT_SET";
            case errc::CR_NOT_IMPLEMENTED:
                return "CR_NOT_IMPLEMENTED";
            case errc::CR_SERVER_LOST_EXTENDED:
                return "CR_SERVER_LOST_EXTENDED";
            case errc::CR_STMT_CLOSED:
                return "CR_STMT_CLOSED";
            case errc::CR_NEW_STMT_METADATA:
                return "CR_NEW_STMT_METADATA";
            case errc::CR_ALREADY_CONNECTED:
                return "CR_ALREADY_CONNECTED";
            case errc::CR_AUTH_PLUGIN_CANNOT_LOAD:
                return "CR_AUTH_PLUGIN_CANNOT_LOAD";
            case errc::CR_AUTH_PLUGIN_ERR:
                return "CR_AUTH_PLUGIN_ERR";
            case errc::CR_INSECURE_API_ERR:
                return "CR_INSECURE_API_ERR";
            case errc::CR_FILE_NAME_TOO_LONG:
                return "CR_FILE_NAME_TOO_LONG";
            case errc::CR_SSL_FIPS_MODE_ERR:
                return "CR_SSL_FIPS_MODE_ERR";
            case errc::CR_DEPRECATED_COMPRESSION_NOT_SUPPORTED:
                return "CR_DEPRECATED_COMPRESSION_NOT_SUPPORTED";
            case errc::CR_COMPRESSION_WRONGLY_CONFIGURED:
                return "CR_COMPRESSION_WRONGLY_CONFIGURED";
            case errc::CR_KERBEROS_USER_NOT_FOUND:
                return "CR_KERBEROS_USER_NOT_FOUND";
            case errc::CR_LOAD_DATA_LOCAL_INFILE_REJECTED:
                return "CR_LOAD_DATA_LOCAL_INFILE_REJECTED";
            case errc::CR_LOAD_DATA_LOCAL_INFILE_REALPATH_FAIL:
                return "CR_LOAD_DATA_LOCAL_INFILE_REALPATH_FAIL";
            case errc::CR_DNS_SRV_LOOKUP_FAILED:
                return "CR_DNS_SRV_LOOKUP_FAILED";
            case errc::CR_MANDATORY_TRACKER_NOT_FOUND:
                return "CR_MANDATORY_TRACKER_NOT_FOUND";
            case errc::CR_INVALID_FACTOR_NO:
                return "CR_INVALID_FACTOR_NO";
            case errc::CR_CANT_GET_SESSION_DATA:
                return "CR_CANT_GET_SESSION_DATA";
            case errc::CR_INVALID_CLIENT_CHARSET:
                return "CR_INVALID_CLIENT_CHARSET";
            case errc::CR_TLS_SERVER_NOT_FOUND:
                return "CR_TLS_SERVER_NOT_FOUND";
            default:
                return "Unknown error code";
            }
        }
    } static __c;
    return __c;
}

namespace detail {
inline std::error_code
make_ec(int code,
        const std::error_category& category = error_category()) noexcept(true) {
    return {code, category};
}
}  // namespace detail
}  // namespace chx::sql::mysql
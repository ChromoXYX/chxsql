#pragma once

#include <system_error>
#include <unordered_map>
#include <string_view>

namespace chx::sql::postgresql {
enum class errc : int {
    successful_completion = 0,
    warning = 1,
    dynamic_result_sets_returned = 2,
    implicit_zero_bit_padding = 3,
    null_value_eliminated_in_set_function = 4,
    privilege_not_granted = 5,
    privilege_not_revoked = 6,
    string_data_right_truncation = 7,
    deprecated_feature = 8,
    no_data = 9,
    no_additional_dynamic_result_sets_returned = 10,
    sql_statement_not_yet_complete = 11,
    connection_exception = 12,
    connection_does_not_exist = 13,
    connection_failure = 14,
    sqlclient_unable_to_establish_sqlconnection = 15,
    sqlserver_rejected_establishment_of_sqlconnection = 16,
    transaction_resolution_unknown = 17,
    protocol_violation = 18,
    triggered_action_exception = 19,
    feature_not_supported = 20,
    invalid_transaction_initiation = 21,
    locator_exception = 22,
    invalid_locator_specification = 23,
    invalid_grantor = 24,
    invalid_grant_operation = 25,
    invalid_role_specification = 26,
    diagnostics_exception = 27,
    stacked_diagnostics_accessed_without_active_handler = 28,
    case_not_found = 29,
    cardinality_violation = 30,
    data_exception = 31,
    array_subscript_error = 32,
    character_not_in_repertoire = 33,
    datetime_field_overflow = 34,
    division_by_zero = 35,
    error_in_assignment = 36,
    escape_character_conflict = 37,
    indicator_overflow = 38,
    interval_field_overflow = 39,
    invalid_argument_for_logarithm = 40,
    invalid_argument_for_ntile_function = 41,
    invalid_argument_for_nth_value_function = 42,
    invalid_argument_for_power_function = 43,
    invalid_argument_for_width_bucket_function = 44,
    invalid_character_value_for_cast = 45,
    invalid_datetime_format = 46,
    invalid_escape_character = 47,
    invalid_escape_octet = 48,
    invalid_escape_sequence = 49,
    nonstandard_use_of_escape_character = 50,
    invalid_indicator_parameter_value = 51,
    invalid_parameter_value = 52,
    invalid_preceding_or_following_size = 53,
    invalid_regular_expression = 54,
    invalid_row_count_in_limit_clause = 55,
    invalid_row_count_in_result_offset_clause = 56,
    invalid_tablesample_argument = 57,
    invalid_tablesample_repeat = 58,
    invalid_time_zone_displacement_value = 59,
    invalid_use_of_escape_character = 60,
    most_specific_type_mismatch = 61,
    null_value_not_allowed = 62,
    null_value_no_indicator_parameter = 63,
    numeric_value_out_of_range = 64,
    sequence_generator_limit_exceeded = 65,
    string_data_length_mismatch = 66,
    string_data_right_truncation_22001 = 67,
    substring_error = 68,
    trim_error = 69,
    unterminated_c_string = 70,
    zero_length_character_string = 71,
    floating_point_exception = 72,
    invalid_text_representation = 73,
    invalid_binary_representation = 74,
    bad_copy_file_format = 75,
    untranslatable_character = 76,
    not_an_xml_document = 77,
    invalid_xml_document = 78,
    invalid_xml_content = 79,
    invalid_xml_comment = 80,
    invalid_xml_processing_instruction = 81,
    duplicate_json_object_key_value = 82,
    invalid_argument_for_sql_json_datetime_function = 83,
    invalid_json_text = 84,
    invalid_sql_json_subscript = 85,
    more_than_one_sql_json_item = 86,
    no_sql_json_item = 87,
    non_numeric_sql_json_item = 88,
    non_unique_keys_in_a_json_object = 89,
    singleton_sql_json_item_required = 90,
    sql_json_array_not_found = 91,
    sql_json_member_not_found = 92,
    sql_json_number_not_found = 93,
    sql_json_object_not_found = 94,
    too_many_json_array_elements = 95,
    too_many_json_object_members = 96,
    sql_json_scalar_required = 97,
    sql_json_item_cannot_be_cast_to_target_type = 98,
    integrity_constraint_violation = 99,
    restrict_violation = 100,
    not_null_violation = 101,
    foreign_key_violation = 102,
    unique_violation = 103,
    check_violation = 104,
    exclusion_violation = 105,
    invalid_cursor_state = 106,
    invalid_transaction_state = 107,
    active_sql_transaction = 108,
    branch_transaction_already_active = 109,
    held_cursor_requires_same_isolation_level = 110,
    inappropriate_access_mode_for_branch_transaction = 111,
    inappropriate_isolation_level_for_branch_transaction = 112,
    no_active_sql_transaction_for_branch_transaction = 113,
    read_only_sql_transaction = 114,
    schema_and_data_statement_mixing_not_supported = 115,
    no_active_sql_transaction = 116,
    in_failed_sql_transaction = 117,
    idle_in_transaction_session_timeout = 118,
    invalid_sql_statement_name = 119,
    triggered_data_change_violation = 120,
    invalid_authorization_specification = 121,
    invalid_password = 122,
    dependent_privilege_descriptors_still_exist = 123,
    dependent_objects_still_exist = 124,
    invalid_transaction_termination = 125,
    sql_routine_exception = 126,
    function_executed_no_return_statement = 127,
    modifying_sql_data_not_permitted = 128,
    prohibited_sql_statement_attempted = 129,
    reading_sql_data_not_permitted = 130,
    invalid_cursor_name = 131,
    external_routine_exception = 132,
    containing_sql_not_permitted = 133,
    modifying_sql_data_not_permitted_38002 = 134,
    prohibited_sql_statement_attempted_38003 = 135,
    reading_sql_data_not_permitted_38004 = 136,
    external_routine_invocation_exception = 137,
    invalid_sqlstate_returned = 138,
    null_value_not_allowed_39004 = 139,
    trigger_protocol_violated = 140,
    srf_protocol_violated = 141,
    event_trigger_protocol_violated = 142,
    savepoint_exception = 143,
    invalid_savepoint_specification = 144,
    invalid_catalog_name = 145,
    invalid_schema_name = 146,
    transaction_rollback = 147,
    transaction_integrity_constraint_violation = 148,
    serialization_failure = 149,
    statement_completion_unknown = 150,
    deadlock_detected = 151,
    syntax_error_or_access_rule_violation = 152,
    syntax_error = 153,
    insufficient_privilege = 154,
    cannot_coerce = 155,
    grouping_error = 156,
    windowing_error = 157,
    invalid_recursion = 158,
    invalid_foreign_key = 159,
    invalid_name = 160,
    name_too_long = 161,
    reserved_name = 162,
    datatype_mismatch = 163,
    indeterminate_datatype = 164,
    collation_mismatch = 165,
    indeterminate_collation = 166,
    wrong_object_type = 167,
    generated_always = 168,
    undefined_column = 169,
    undefined_function = 170,
    undefined_table = 171,
    undefined_parameter = 172,
    undefined_object = 173,
    duplicate_column = 174,
    duplicate_cursor = 175,
    duplicate_database = 176,
    duplicate_function = 177,
    duplicate_prepared_statement = 178,
    duplicate_schema = 179,
    duplicate_table = 180,
    duplicate_alias = 181,
    duplicate_object = 182,
    ambiguous_column = 183,
    ambiguous_function = 184,
    ambiguous_parameter = 185,
    ambiguous_alias = 186,
    invalid_column_reference = 187,
    invalid_column_definition = 188,
    invalid_cursor_definition = 189,
    invalid_database_definition = 190,
    invalid_function_definition = 191,
    invalid_prepared_statement_definition = 192,
    invalid_schema_definition = 193,
    invalid_table_definition = 194,
    invalid_object_definition = 195,
    with_check_option_violation = 196,
    insufficient_resources = 197,
    disk_full = 198,
    out_of_memory = 199,
    too_many_connections = 200,
    configuration_limit_exceeded = 201,
    program_limit_exceeded = 202,
    statement_too_complex = 203,
    too_many_columns = 204,
    too_many_arguments = 205,
    object_not_in_prerequisite_state = 206,
    object_in_use = 207,
    cant_change_runtime_param = 208,
    lock_not_available = 209,
    unsafe_new_enum_value_usage = 210,
    operator_intervention = 211,
    query_canceled = 212,
    admin_shutdown = 213,
    crash_shutdown = 214,
    cannot_connect_now = 215,
    database_dropped = 216,
    idle_session_timeout = 217,
    system_error = 218,
    io_error = 219,
    undefined_file = 220,
    duplicate_file = 221,
    snapshot_too_old = 222,
    config_file_error = 223,
    lock_file_exists = 224,
    fdw_error = 225,
    fdw_column_name_not_found = 226,
    fdw_dynamic_parameter_value_needed = 227,
    fdw_function_sequence_error = 228,
    fdw_inconsistent_descriptor_information = 229,
    fdw_invalid_attribute_value = 230,
    fdw_invalid_column_name = 231,
    fdw_invalid_column_number = 232,
    fdw_invalid_data_type = 233,
    fdw_invalid_data_type_descriptors = 234,
    fdw_invalid_descriptor_field_identifier = 235,
    fdw_invalid_handle = 236,
    fdw_invalid_option_index = 237,
    fdw_invalid_option_name = 238,
    fdw_invalid_string_length_or_buffer_length = 239,
    fdw_invalid_string_format = 240,
    fdw_invalid_use_of_null_pointer = 241,
    fdw_too_many_handles = 242,
    fdw_out_of_memory = 243,
    fdw_no_schemas = 244,
    fdw_option_name_not_found = 245,
    fdw_reply_handle = 246,
    fdw_schema_not_found = 247,
    fdw_table_not_found = 248,
    fdw_unable_to_create_execution = 249,
    fdw_unable_to_create_reply = 250,
    fdw_unable_to_establish_connection = 251,
    plpgsql_error = 252,
    raise_exception = 253,
    no_data_found = 254,
    too_many_rows = 255,
    assert_failure = 256,
    internal_error = 257,
    data_corrupted = 258,
    index_corrupted = 259,

    malformed_message = 260,
    unknown_sqlstate = 261
};
}

namespace std {
template <>
struct is_error_condition_enum<::chx::sql::postgresql::errc> : std::true_type {
};
}  // namespace std

namespace chx::sql::postgresql {
inline std::error_category& error_category() {
    class __impl : public std::error_category {
      public:
        virtual const char* name() const noexcept(true) override {
            return "PostgreSQL error_category";
        }

        virtual std::error_condition default_error_condition(int ev) const
            noexcept(true) override {
            return std::error_condition(ev, postgresql::error_category());
        }

        virtual bool equivalent(const std::error_code& ec, int ev) const
            noexcept(true) override {
            return *this == ec.category() && static_cast<int>(ec.value()) == ev;
        }

        virtual std::string message(int ev) const override {
            switch (static_cast<errc>(ev)) {
            case errc::successful_completion:
                return "successful completion";
            case errc::warning:
                return "warning";
            case errc::dynamic_result_sets_returned:
                return "dynamic result sets returned";
            case errc::implicit_zero_bit_padding:
                return "implicit zero bit padding";
            case errc::null_value_eliminated_in_set_function:
                return "null value eliminated in set function";
            case errc::privilege_not_granted:
                return "privilege not granted";
            case errc::privilege_not_revoked:
                return "privilege not revoked";
            case errc::string_data_right_truncation:
                return "string data right truncation";
            case errc::deprecated_feature:
                return "deprecated feature";
            case errc::no_data:
                return "no data";
            case errc::no_additional_dynamic_result_sets_returned:
                return "no additional dynamic result sets returned";
            case errc::sql_statement_not_yet_complete:
                return "sql statement not yet complete";
            case errc::connection_exception:
                return "connection exception";
            case errc::connection_does_not_exist:
                return "connection does not exist";
            case errc::connection_failure:
                return "connection failure";
            case errc::sqlclient_unable_to_establish_sqlconnection:
                return "sqlclient unable to establish sqlconnection";
            case errc::sqlserver_rejected_establishment_of_sqlconnection:
                return "sqlserver rejected establishment of sqlconnection";
            case errc::transaction_resolution_unknown:
                return "transaction resolution unknown";
            case errc::protocol_violation:
                return "protocol violation";
            case errc::triggered_action_exception:
                return "triggered action exception";
            case errc::feature_not_supported:
                return "feature not supported";
            case errc::invalid_transaction_initiation:
                return "invalid transaction initiation";
            case errc::locator_exception:
                return "locator exception";
            case errc::invalid_locator_specification:
                return "invalid locator specification";
            case errc::invalid_grantor:
                return "invalid grantor";
            case errc::invalid_grant_operation:
                return "invalid grant operation";
            case errc::invalid_role_specification:
                return "invalid role specification";
            case errc::diagnostics_exception:
                return "diagnostics exception";
            case errc::stacked_diagnostics_accessed_without_active_handler:
                return "stacked diagnostics accessed without active handler";
            case errc::case_not_found:
                return "case not found";
            case errc::cardinality_violation:
                return "cardinality violation";
            case errc::data_exception:
                return "data exception";
            case errc::array_subscript_error:
                return "array subscript error";
            case errc::character_not_in_repertoire:
                return "character not in repertoire";
            case errc::datetime_field_overflow:
                return "datetime field overflow";
            case errc::division_by_zero:
                return "division by zero";
            case errc::error_in_assignment:
                return "error in assignment";
            case errc::escape_character_conflict:
                return "escape character conflict";
            case errc::indicator_overflow:
                return "indicator overflow";
            case errc::interval_field_overflow:
                return "interval field overflow";
            case errc::invalid_argument_for_logarithm:
                return "invalid argument for logarithm";
            case errc::invalid_argument_for_ntile_function:
                return "invalid argument for ntile function";
            case errc::invalid_argument_for_nth_value_function:
                return "invalid argument for nth value function";
            case errc::invalid_argument_for_power_function:
                return "invalid argument for power function";
            case errc::invalid_argument_for_width_bucket_function:
                return "invalid argument for width bucket function";
            case errc::invalid_character_value_for_cast:
                return "invalid character value for cast";
            case errc::invalid_datetime_format:
                return "invalid datetime format";
            case errc::invalid_escape_character:
                return "invalid escape character";
            case errc::invalid_escape_octet:
                return "invalid escape octet";
            case errc::invalid_escape_sequence:
                return "invalid escape sequence";
            case errc::nonstandard_use_of_escape_character:
                return "nonstandard use of escape character";
            case errc::invalid_indicator_parameter_value:
                return "invalid indicator parameter value";
            case errc::invalid_parameter_value:
                return "invalid parameter value";
            case errc::invalid_preceding_or_following_size:
                return "invalid preceding or following size";
            case errc::invalid_regular_expression:
                return "invalid regular expression";
            case errc::invalid_row_count_in_limit_clause:
                return "invalid row count in limit clause";
            case errc::invalid_row_count_in_result_offset_clause:
                return "invalid row count in result offset clause";
            case errc::invalid_tablesample_argument:
                return "invalid tablesample argument";
            case errc::invalid_tablesample_repeat:
                return "invalid tablesample repeat";
            case errc::invalid_time_zone_displacement_value:
                return "invalid time zone displacement value";
            case errc::invalid_use_of_escape_character:
                return "invalid use of escape character";
            case errc::most_specific_type_mismatch:
                return "most specific type mismatch";
            case errc::null_value_not_allowed:
                return "null value not allowed";
            case errc::null_value_no_indicator_parameter:
                return "null value no indicator parameter";
            case errc::numeric_value_out_of_range:
                return "numeric value out of range";
            case errc::sequence_generator_limit_exceeded:
                return "sequence generator limit exceeded";
            case errc::string_data_length_mismatch:
                return "string data length mismatch";
            case errc::string_data_right_truncation_22001:
                return "string data right truncation 22001";
            case errc::substring_error:
                return "substring error";
            case errc::trim_error:
                return "trim error";
            case errc::unterminated_c_string:
                return "unterminated c string";
            case errc::zero_length_character_string:
                return "zero length character string";
            case errc::floating_point_exception:
                return "floating point exception";
            case errc::invalid_text_representation:
                return "invalid text representation";
            case errc::invalid_binary_representation:
                return "invalid binary representation";
            case errc::bad_copy_file_format:
                return "bad copy file format";
            case errc::untranslatable_character:
                return "untranslatable character";
            case errc::not_an_xml_document:
                return "not an xml document";
            case errc::invalid_xml_document:
                return "invalid xml document";
            case errc::invalid_xml_content:
                return "invalid xml content";
            case errc::invalid_xml_comment:
                return "invalid xml comment";
            case errc::invalid_xml_processing_instruction:
                return "invalid xml processing instruction";
            case errc::duplicate_json_object_key_value:
                return "duplicate json object key value";
            case errc::invalid_argument_for_sql_json_datetime_function:
                return "invalid argument for sql json datetime function";
            case errc::invalid_json_text:
                return "invalid json text";
            case errc::invalid_sql_json_subscript:
                return "invalid sql json subscript";
            case errc::more_than_one_sql_json_item:
                return "more than one sql json item";
            case errc::no_sql_json_item:
                return "no sql json item";
            case errc::non_numeric_sql_json_item:
                return "non numeric sql json item";
            case errc::non_unique_keys_in_a_json_object:
                return "non unique keys in a json object";
            case errc::singleton_sql_json_item_required:
                return "singleton sql json item required";
            case errc::sql_json_array_not_found:
                return "sql json array not found";
            case errc::sql_json_member_not_found:
                return "sql json member not found";
            case errc::sql_json_number_not_found:
                return "sql json number not found";
            case errc::sql_json_object_not_found:
                return "sql json object not found";
            case errc::too_many_json_array_elements:
                return "too many json array elements";
            case errc::too_many_json_object_members:
                return "too many json object members";
            case errc::sql_json_scalar_required:
                return "sql json scalar required";
            case errc::sql_json_item_cannot_be_cast_to_target_type:
                return "sql json item cannot be cast to target type";
            case errc::integrity_constraint_violation:
                return "integrity constraint violation";
            case errc::restrict_violation:
                return "restrict violation";
            case errc::not_null_violation:
                return "not null violation";
            case errc::foreign_key_violation:
                return "foreign key violation";
            case errc::unique_violation:
                return "unique violation";
            case errc::check_violation:
                return "check violation";
            case errc::exclusion_violation:
                return "exclusion violation";
            case errc::invalid_cursor_state:
                return "invalid cursor state";
            case errc::invalid_transaction_state:
                return "invalid transaction state";
            case errc::active_sql_transaction:
                return "active sql transaction";
            case errc::branch_transaction_already_active:
                return "branch transaction already active";
            case errc::held_cursor_requires_same_isolation_level:
                return "held cursor requires same isolation level";
            case errc::inappropriate_access_mode_for_branch_transaction:
                return "inappropriate access mode for branch transaction";
            case errc::inappropriate_isolation_level_for_branch_transaction:
                return "inappropriate isolation level for branch transaction";
            case errc::no_active_sql_transaction_for_branch_transaction:
                return "no active sql transaction for branch transaction";
            case errc::read_only_sql_transaction:
                return "read only sql transaction";
            case errc::schema_and_data_statement_mixing_not_supported:
                return "schema and data statement mixing not supported";
            case errc::no_active_sql_transaction:
                return "no active sql transaction";
            case errc::in_failed_sql_transaction:
                return "in failed sql transaction";
            case errc::idle_in_transaction_session_timeout:
                return "idle in transaction session timeout";
            case errc::invalid_sql_statement_name:
                return "invalid sql statement name";
            case errc::triggered_data_change_violation:
                return "triggered data change violation";
            case errc::invalid_authorization_specification:
                return "invalid authorization specification";
            case errc::invalid_password:
                return "invalid password";
            case errc::dependent_privilege_descriptors_still_exist:
                return "dependent privilege descriptors still exist";
            case errc::dependent_objects_still_exist:
                return "dependent objects still exist";
            case errc::invalid_transaction_termination:
                return "invalid transaction termination";
            case errc::sql_routine_exception:
                return "sql routine exception";
            case errc::function_executed_no_return_statement:
                return "function executed no return statement";
            case errc::modifying_sql_data_not_permitted:
                return "modifying sql data not permitted";
            case errc::prohibited_sql_statement_attempted:
                return "prohibited sql statement attempted";
            case errc::reading_sql_data_not_permitted:
                return "reading sql data not permitted";
            case errc::invalid_cursor_name:
                return "invalid cursor name";
            case errc::external_routine_exception:
                return "external routine exception";
            case errc::containing_sql_not_permitted:
                return "containing sql not permitted";
            case errc::modifying_sql_data_not_permitted_38002:
                return "modifying sql data not permitted 38002";
            case errc::prohibited_sql_statement_attempted_38003:
                return "prohibited sql statement attempted 38003";
            case errc::reading_sql_data_not_permitted_38004:
                return "reading sql data not permitted 38004";
            case errc::external_routine_invocation_exception:
                return "external routine invocation exception";
            case errc::invalid_sqlstate_returned:
                return "invalid sqlstate returned";
            case errc::null_value_not_allowed_39004:
                return "null value not allowed 39004";
            case errc::trigger_protocol_violated:
                return "trigger protocol violated";
            case errc::srf_protocol_violated:
                return "srf protocol violated";
            case errc::event_trigger_protocol_violated:
                return "event trigger protocol violated";
            case errc::savepoint_exception:
                return "savepoint exception";
            case errc::invalid_savepoint_specification:
                return "invalid savepoint specification";
            case errc::invalid_catalog_name:
                return "invalid catalog name";
            case errc::invalid_schema_name:
                return "invalid schema name";
            case errc::transaction_rollback:
                return "transaction rollback";
            case errc::transaction_integrity_constraint_violation:
                return "transaction integrity constraint violation";
            case errc::serialization_failure:
                return "serialization failure";
            case errc::statement_completion_unknown:
                return "statement completion unknown";
            case errc::deadlock_detected:
                return "deadlock detected";
            case errc::syntax_error_or_access_rule_violation:
                return "syntax error or access rule violation";
            case errc::syntax_error:
                return "syntax error";
            case errc::insufficient_privilege:
                return "insufficient privilege";
            case errc::cannot_coerce:
                return "cannot coerce";
            case errc::grouping_error:
                return "grouping error";
            case errc::windowing_error:
                return "windowing error";
            case errc::invalid_recursion:
                return "invalid recursion";
            case errc::invalid_foreign_key:
                return "invalid foreign key";
            case errc::invalid_name:
                return "invalid name";
            case errc::name_too_long:
                return "name too long";
            case errc::reserved_name:
                return "reserved name";
            case errc::datatype_mismatch:
                return "datatype mismatch";
            case errc::indeterminate_datatype:
                return "indeterminate datatype";
            case errc::collation_mismatch:
                return "collation mismatch";
            case errc::indeterminate_collation:
                return "indeterminate collation";
            case errc::wrong_object_type:
                return "wrong object type";
            case errc::generated_always:
                return "generated always";
            case errc::undefined_column:
                return "undefined column";
            case errc::undefined_function:
                return "undefined function";
            case errc::undefined_table:
                return "undefined table";
            case errc::undefined_parameter:
                return "undefined parameter";
            case errc::undefined_object:
                return "undefined object";
            case errc::duplicate_column:
                return "duplicate column";
            case errc::duplicate_cursor:
                return "duplicate cursor";
            case errc::duplicate_database:
                return "duplicate database";
            case errc::duplicate_function:
                return "duplicate function";
            case errc::duplicate_prepared_statement:
                return "duplicate prepared statement";
            case errc::duplicate_schema:
                return "duplicate schema";
            case errc::duplicate_table:
                return "duplicate table";
            case errc::duplicate_alias:
                return "duplicate alias";
            case errc::duplicate_object:
                return "duplicate object";
            case errc::ambiguous_column:
                return "ambiguous column";
            case errc::ambiguous_function:
                return "ambiguous function";
            case errc::ambiguous_parameter:
                return "ambiguous parameter";
            case errc::ambiguous_alias:
                return "ambiguous alias";
            case errc::invalid_column_reference:
                return "invalid column reference";
            case errc::invalid_column_definition:
                return "invalid column definition";
            case errc::invalid_cursor_definition:
                return "invalid cursor definition";
            case errc::invalid_database_definition:
                return "invalid database definition";
            case errc::invalid_function_definition:
                return "invalid function definition";
            case errc::invalid_prepared_statement_definition:
                return "invalid prepared statement definition";
            case errc::invalid_schema_definition:
                return "invalid schema definition";
            case errc::invalid_table_definition:
                return "invalid table definition";
            case errc::invalid_object_definition:
                return "invalid object definition";
            case errc::with_check_option_violation:
                return "with check option violation";
            case errc::insufficient_resources:
                return "insufficient resources";
            case errc::disk_full:
                return "disk full";
            case errc::out_of_memory:
                return "out of memory";
            case errc::too_many_connections:
                return "too many connections";
            case errc::configuration_limit_exceeded:
                return "configuration limit exceeded";
            case errc::program_limit_exceeded:
                return "program limit exceeded";
            case errc::statement_too_complex:
                return "statement too complex";
            case errc::too_many_columns:
                return "too many columns";
            case errc::too_many_arguments:
                return "too many arguments";
            case errc::object_not_in_prerequisite_state:
                return "object not in prerequisite state";
            case errc::object_in_use:
                return "object in use";
            case errc::cant_change_runtime_param:
                return "cant change runtime param";
            case errc::lock_not_available:
                return "lock not available";
            case errc::unsafe_new_enum_value_usage:
                return "unsafe new enum value usage";
            case errc::operator_intervention:
                return "operator intervention";
            case errc::query_canceled:
                return "query canceled";
            case errc::admin_shutdown:
                return "admin shutdown";
            case errc::crash_shutdown:
                return "crash shutdown";
            case errc::cannot_connect_now:
                return "cannot connect now";
            case errc::database_dropped:
                return "database dropped";
            case errc::idle_session_timeout:
                return "idle session timeout";
            case errc::system_error:
                return "system error";
            case errc::io_error:
                return "io error";
            case errc::undefined_file:
                return "undefined file";
            case errc::duplicate_file:
                return "duplicate file";
            case errc::snapshot_too_old:
                return "snapshot too old";
            case errc::config_file_error:
                return "config file error";
            case errc::lock_file_exists:
                return "lock file exists";
            case errc::fdw_error:
                return "fdw error";
            case errc::fdw_column_name_not_found:
                return "fdw column name not found";
            case errc::fdw_dynamic_parameter_value_needed:
                return "fdw dynamic parameter value needed";
            case errc::fdw_function_sequence_error:
                return "fdw function sequence error";
            case errc::fdw_inconsistent_descriptor_information:
                return "fdw inconsistent descriptor information";
            case errc::fdw_invalid_attribute_value:
                return "fdw invalid attribute value";
            case errc::fdw_invalid_column_name:
                return "fdw invalid column name";
            case errc::fdw_invalid_column_number:
                return "fdw invalid column number";
            case errc::fdw_invalid_data_type:
                return "fdw invalid data type";
            case errc::fdw_invalid_data_type_descriptors:
                return "fdw invalid data type descriptors";
            case errc::fdw_invalid_descriptor_field_identifier:
                return "fdw invalid descriptor field identifier";
            case errc::fdw_invalid_handle:
                return "fdw invalid handle";
            case errc::fdw_invalid_option_index:
                return "fdw invalid option index";
            case errc::fdw_invalid_option_name:
                return "fdw invalid option name";
            case errc::fdw_invalid_string_length_or_buffer_length:
                return "fdw invalid string length or buffer length";
            case errc::fdw_invalid_string_format:
                return "fdw invalid string format";
            case errc::fdw_invalid_use_of_null_pointer:
                return "fdw invalid use of null pointer";
            case errc::fdw_too_many_handles:
                return "fdw too many handles";
            case errc::fdw_out_of_memory:
                return "fdw out of memory";
            case errc::fdw_no_schemas:
                return "fdw no schemas";
            case errc::fdw_option_name_not_found:
                return "fdw option name not found";
            case errc::fdw_reply_handle:
                return "fdw reply handle";
            case errc::fdw_schema_not_found:
                return "fdw schema not found";
            case errc::fdw_table_not_found:
                return "fdw table not found";
            case errc::fdw_unable_to_create_execution:
                return "fdw unable to create execution";
            case errc::fdw_unable_to_create_reply:
                return "fdw unable to create reply";
            case errc::fdw_unable_to_establish_connection:
                return "fdw unable to establish connection";
            case errc::plpgsql_error:
                return "plpgsql error";
            case errc::raise_exception:
                return "raise exception";
            case errc::no_data_found:
                return "no data found";
            case errc::too_many_rows:
                return "too many rows";
            case errc::assert_failure:
                return "assert failure";
            case errc::internal_error:
                return "internal error";
            case errc::data_corrupted:
                return "data corrupted";
            case errc::index_corrupted:
                return "index corrupted";
            case errc::malformed_message: {
                return "malformed message";
            }
            default:
                return "Unknown SQLSTATE error code";
            }
        }
    } static __c;
    return __c;
}

inline std::error_condition make_error_condition(errc e) {
    return {static_cast<int>(e), error_category()};
}

namespace detail {
inline std::error_code
make_ec(errc code,
        const std::error_category& category = error_category()) noexcept(true) {
    return {static_cast<int>(code), category};
}

inline errc sqlstate_to_errc(std::string_view sqlstate) noexcept(true) {
    static const std::unordered_map<std::string_view, errc> sqlstate_map = {
        {"00000", errc::successful_completion},
        {"01000", errc::warning},
        {"0100C", errc::dynamic_result_sets_returned},
        {"01008", errc::implicit_zero_bit_padding},
        {"01003", errc::null_value_eliminated_in_set_function},
        {"01007", errc::privilege_not_granted},
        {"01006", errc::privilege_not_revoked},
        {"01004", errc::string_data_right_truncation},
        {"01P01", errc::deprecated_feature},
        {"02000", errc::no_data},
        {"02001", errc::no_additional_dynamic_result_sets_returned},
        {"03000", errc::sql_statement_not_yet_complete},
        {"08000", errc::connection_exception},
        {"08003", errc::connection_does_not_exist},
        {"08006", errc::connection_failure},
        {"08001", errc::sqlclient_unable_to_establish_sqlconnection},
        {"08004", errc::sqlserver_rejected_establishment_of_sqlconnection},
        {"08007", errc::transaction_resolution_unknown},
        {"08P01", errc::protocol_violation},
        {"09000", errc::triggered_action_exception},
        {"0A000", errc::feature_not_supported},
        {"0B000", errc::invalid_transaction_initiation},
        {"0F000", errc::locator_exception},
        {"0F001", errc::invalid_locator_specification},
        {"0L000", errc::invalid_grantor},
        {"0LP01", errc::invalid_grant_operation},
        {"0P000", errc::invalid_role_specification},
        {"0Z000", errc::diagnostics_exception},
        {"0Z002", errc::stacked_diagnostics_accessed_without_active_handler},
        {"20000", errc::case_not_found},
        {"21000", errc::cardinality_violation},
        {"22000", errc::data_exception},
        {"2202E", errc::array_subscript_error},
        {"22021", errc::character_not_in_repertoire},
        {"22008", errc::datetime_field_overflow},
        {"22012", errc::division_by_zero},
        {"22005", errc::error_in_assignment},
        {"2200B", errc::escape_character_conflict},
        {"22022", errc::indicator_overflow},
        {"22015", errc::interval_field_overflow},
        {"2201E", errc::invalid_argument_for_logarithm},
        {"22014", errc::invalid_argument_for_ntile_function},
        {"22016", errc::invalid_argument_for_nth_value_function},
        {"2201F", errc::invalid_argument_for_power_function},
        {"2201G", errc::invalid_argument_for_width_bucket_function},
        {"22018", errc::invalid_character_value_for_cast},
        {"22007", errc::invalid_datetime_format},
        {"22019", errc::invalid_escape_character},
        {"2200D", errc::invalid_escape_octet},
        {"22025", errc::invalid_escape_sequence},
        {"22P06", errc::nonstandard_use_of_escape_character},
        {"22010", errc::invalid_indicator_parameter_value},
        {"22023", errc::invalid_parameter_value},
        {"22013", errc::invalid_preceding_or_following_size},
        {"2201B", errc::invalid_regular_expression},
        {"2201W", errc::invalid_row_count_in_limit_clause},
        {"2201X", errc::invalid_row_count_in_result_offset_clause},
        {"2202H", errc::invalid_tablesample_argument},
        {"2202G", errc::invalid_tablesample_repeat},
        {"22009", errc::invalid_time_zone_displacement_value},
        {"2200C", errc::invalid_use_of_escape_character},
        {"2200G", errc::most_specific_type_mismatch},
        {"22004", errc::null_value_not_allowed},
        {"22002", errc::null_value_no_indicator_parameter},
        {"22003", errc::numeric_value_out_of_range},
        {"2200H", errc::sequence_generator_limit_exceeded},
        {"22026", errc::string_data_length_mismatch},
        {"22001", errc::string_data_right_truncation_22001},
        {"22011", errc::substring_error},
        {"22027", errc::trim_error},
        {"22024", errc::unterminated_c_string},
        {"2200F", errc::zero_length_character_string},
        {"22P01", errc::floating_point_exception},
        {"22P02", errc::invalid_text_representation},
        {"22P03", errc::invalid_binary_representation},
        {"22P04", errc::bad_copy_file_format},
        {"22P05", errc::untranslatable_character},
        {"2200L", errc::not_an_xml_document},
        {"2200M", errc::invalid_xml_document},
        {"2200N", errc::invalid_xml_content},
        {"2200S", errc::invalid_xml_comment},
        {"2200T", errc::invalid_xml_processing_instruction},
        {"22030", errc::duplicate_json_object_key_value},
        {"22031", errc::invalid_argument_for_sql_json_datetime_function},
        {"22032", errc::invalid_json_text},
        {"22033", errc::invalid_sql_json_subscript},
        {"22034", errc::more_than_one_sql_json_item},
        {"22035", errc::no_sql_json_item},
        {"22036", errc::non_numeric_sql_json_item},
        {"22037", errc::non_unique_keys_in_a_json_object},
        {"22038", errc::singleton_sql_json_item_required},
        {"22039", errc::sql_json_array_not_found},
        {"2203A", errc::sql_json_member_not_found},
        {"2203B", errc::sql_json_number_not_found},
        {"2203C", errc::sql_json_object_not_found},
        {"2203D", errc::too_many_json_array_elements},
        {"2203E", errc::too_many_json_object_members},
        {"2203F", errc::sql_json_scalar_required},
        {"2203G", errc::sql_json_item_cannot_be_cast_to_target_type},
        {"23000", errc::integrity_constraint_violation},
        {"23001", errc::restrict_violation},
        {"23502", errc::not_null_violation},
        {"23503", errc::foreign_key_violation},
        {"23505", errc::unique_violation},
        {"23514", errc::check_violation},
        {"23P01", errc::exclusion_violation},
        {"24000", errc::invalid_cursor_state},
        {"25000", errc::invalid_transaction_state},
        {"25001", errc::active_sql_transaction},
        {"25002", errc::branch_transaction_already_active},
        {"25008", errc::held_cursor_requires_same_isolation_level},
        {"25003", errc::inappropriate_access_mode_for_branch_transaction},
        {"25004", errc::inappropriate_isolation_level_for_branch_transaction},
        {"25005", errc::no_active_sql_transaction_for_branch_transaction},
        {"25006", errc::read_only_sql_transaction},
        {"25007", errc::schema_and_data_statement_mixing_not_supported},
        {"25P01", errc::no_active_sql_transaction},
        {"25P02", errc::in_failed_sql_transaction},
        {"25P03", errc::idle_in_transaction_session_timeout},
        {"26000", errc::invalid_sql_statement_name},
        {"27000", errc::triggered_data_change_violation},
        {"28000", errc::invalid_authorization_specification},
        {"28P01", errc::invalid_password},
        {"2B000", errc::dependent_privilege_descriptors_still_exist},
        {"2BP01", errc::dependent_objects_still_exist},
        {"2D000", errc::invalid_transaction_termination},
        {"2F000", errc::sql_routine_exception},
        {"2F005", errc::function_executed_no_return_statement},
        {"2F002", errc::modifying_sql_data_not_permitted},
        {"2F003", errc::prohibited_sql_statement_attempted},
        {"2F004", errc::reading_sql_data_not_permitted},
        {"34000", errc::invalid_cursor_name},
        {"38000", errc::external_routine_exception},
        {"38001", errc::containing_sql_not_permitted},
        {"38002", errc::modifying_sql_data_not_permitted_38002},
        {"38003", errc::prohibited_sql_statement_attempted_38003},
        {"38004", errc::reading_sql_data_not_permitted_38004},
        {"39000", errc::external_routine_invocation_exception},
        {"39001", errc::invalid_sqlstate_returned},
        {"39004", errc::null_value_not_allowed_39004},
        {"39P01", errc::trigger_protocol_violated},
        {"39P02", errc::srf_protocol_violated},
        {"39P03", errc::event_trigger_protocol_violated},
        {"3B000", errc::savepoint_exception},
        {"3B001", errc::invalid_savepoint_specification},
        {"3D000", errc::invalid_catalog_name},
        {"3F000", errc::invalid_schema_name},
        {"40000", errc::transaction_rollback},
        {"40002", errc::transaction_integrity_constraint_violation},
        {"40001", errc::serialization_failure},
        {"40003", errc::statement_completion_unknown},
        {"40P01", errc::deadlock_detected},
        {"42000", errc::syntax_error_or_access_rule_violation},
        {"42601", errc::syntax_error},
        {"42501", errc::insufficient_privilege},
        {"42846", errc::cannot_coerce},
        {"42803", errc::grouping_error},
        {"42P20", errc::windowing_error},
        {"42P19", errc::invalid_recursion},
        {"42830", errc::invalid_foreign_key},
        {"42602", errc::invalid_name},
        {"42622", errc::name_too_long},
        {"42939", errc::reserved_name},
        {"42804", errc::datatype_mismatch},
        {"42P18", errc::indeterminate_datatype},
        {"42P21", errc::collation_mismatch},
        {"42P22", errc::indeterminate_collation},
        {"42809", errc::wrong_object_type},
        {"428C9", errc::generated_always},
        {"42703", errc::undefined_column},
        {"42883", errc::undefined_function},
        {"42P01", errc::undefined_table},
        {"42P02", errc::undefined_parameter},
        {"42704", errc::undefined_object},
        {"42701", errc::duplicate_column},
        {"42P03", errc::duplicate_cursor},
        {"42P04", errc::duplicate_database},
        {"42723", errc::duplicate_function},
        {"42P05", errc::duplicate_prepared_statement},
        {"42P06", errc::duplicate_schema},
        {"42P07", errc::duplicate_table},
        {"42712", errc::duplicate_alias},
        {"42710", errc::duplicate_object},
        {"42702", errc::ambiguous_column},
        {"42725", errc::ambiguous_function},
        {"42P08", errc::ambiguous_parameter},
        {"42P09", errc::ambiguous_alias},
        {"42P10", errc::invalid_column_reference},
        {"42611", errc::invalid_column_definition},
        {"42P11", errc::invalid_cursor_definition},
        {"42P12", errc::invalid_database_definition},
        {"42P13", errc::invalid_function_definition},
        {"42P14", errc::invalid_prepared_statement_definition},
        {"42P15", errc::invalid_schema_definition},
        {"42P16", errc::invalid_table_definition},
        {"42P17", errc::invalid_object_definition},
        {"44000", errc::with_check_option_violation},
        {"53000", errc::insufficient_resources},
        {"53100", errc::disk_full},
        {"53200", errc::out_of_memory},
        {"53300", errc::too_many_connections},
        {"53400", errc::configuration_limit_exceeded},
        {"54000", errc::program_limit_exceeded},
        {"54001", errc::statement_too_complex},
        {"54011", errc::too_many_columns},
        {"54023", errc::too_many_arguments},
        {"55000", errc::object_not_in_prerequisite_state},
        {"55006", errc::object_in_use},
        {"55P02", errc::cant_change_runtime_param},
        {"55P03", errc::lock_not_available},
        {"55P04", errc::unsafe_new_enum_value_usage},
        {"57000", errc::operator_intervention},
        {"57014", errc::query_canceled},
        {"57P01", errc::admin_shutdown},
        {"57P02", errc::crash_shutdown},
        {"57P03", errc::cannot_connect_now},
        {"57P04", errc::database_dropped},
        {"57P05", errc::idle_session_timeout},
        {"58000", errc::system_error},
        {"58030", errc::io_error},
        {"58P01", errc::undefined_file},
        {"58P02", errc::duplicate_file},
        {"72000", errc::snapshot_too_old},
        {"F0000", errc::config_file_error},
        {"F0001", errc::lock_file_exists},
        {"HV000", errc::fdw_error},
        {"HV005", errc::fdw_column_name_not_found},
        {"HV002", errc::fdw_dynamic_parameter_value_needed},
        {"HV010", errc::fdw_function_sequence_error},
        {"HV021", errc::fdw_inconsistent_descriptor_information},
        {"HV024", errc::fdw_invalid_attribute_value},
        {"HV007", errc::fdw_invalid_column_name},
        {"HV008", errc::fdw_invalid_column_number},
        {"HV004", errc::fdw_invalid_data_type},
        {"HV006", errc::fdw_invalid_data_type_descriptors},
        {"HV091", errc::fdw_invalid_descriptor_field_identifier},
        {"HV00B", errc::fdw_invalid_handle},
        {"HV00C", errc::fdw_invalid_option_index},
        {"HV00D", errc::fdw_invalid_option_name},
        {"HV090", errc::fdw_invalid_string_length_or_buffer_length},
        {"HV00A", errc::fdw_invalid_string_format},
        {"HV009", errc::fdw_invalid_use_of_null_pointer},
        {"HV014", errc::fdw_too_many_handles},
        {"HV001", errc::fdw_out_of_memory},
        {"HV00P", errc::fdw_no_schemas},
        {"HV00J", errc::fdw_option_name_not_found},
        {"HV00K", errc::fdw_reply_handle},
        {"HV00Q", errc::fdw_schema_not_found},
        {"HV00R", errc::fdw_table_not_found},
        {"HV00L", errc::fdw_unable_to_create_execution},
        {"HV00M", errc::fdw_unable_to_create_reply},
        {"HV00N", errc::fdw_unable_to_establish_connection},
        {"P0000", errc::plpgsql_error},
        {"P0001", errc::raise_exception},
        {"P0002", errc::no_data_found},
        {"P0003", errc::too_many_rows},
        {"P0004", errc::assert_failure},
        {"XX000", errc::internal_error},
        {"XX001", errc::data_corrupted},
        {"XX002", errc::index_corrupted},
    };
    if (auto ite = sqlstate_map.find(sqlstate); ite != sqlstate_map.end()) {
        return ite->second;
    } else {
        return errc::unknown_sqlstate;
    }
}
}  // namespace detail
}  // namespace chx::sql::postgresql
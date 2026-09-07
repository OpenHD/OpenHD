module;

#include "openhd_util.h"
#include <string>
#include <vector>
#include <optional>

export module ohd_common;

export namespace OHDUtil {
    using OHDUtil::to_uppercase;
    using OHDUtil::endsWith;
    using OHDUtil::startsWith;
    using OHDUtil::contains;
    using OHDUtil::contains_after_uppercase;
    using OHDUtil::str_equal;
    using OHDUtil::equal_after_uppercase;
    using OHDUtil::rtrim;
    using OHDUtil::ltrim;
    using OHDUtil::trim;
    using OHDUtil::create_command_with_args;
    using OHDUtil::join_strings;
    using OHDUtil::run_command;
    using OHDUtil::run_command_out;
    using OHDUtil::keep_alive_until_sigterm;
    using OHDUtil::is_valid_ip;
    using OHDUtil::yes_or_no;
    using OHDUtil::string_in_between;
    using OHDUtil::string_to_int;
    using OHDUtil::string_to_long;
    using OHDUtil::string_to_float;
    using OHDUtil::string_to_long_hex;
    using OHDUtil::get_nth_bit;
    using OHDUtil::split_into_substrings;
    using OHDUtil::check_root;
    using OHDUtil::terminate_if_not_root;
    using OHDUtil::get_ohd_env_variable_bool;
    using OHDUtil::file_exists_and_delete;
    using OHDUtil::split_string_by_newline;
    using OHDUtil::create_string_from_lines;
    using OHDUtil::str_vec_as_string;
    using OHDUtil::bytes_as_string;
    using OHDUtil::map_int_percentage_to_minus1_to_1;
    using OHDUtil::map_int_percentage_0_200_to_minus1_to_1;
    using OHDUtil::calculate_progress_perc;
    using OHDUtil::int_as_string;
    using OHDUtil::password_as_hidden_str;
}

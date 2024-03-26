#ifndef OPENHD_UTIL_H
#define OPENHD_UTIL_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace OHDUtil {

// Converts all letters in the given string to uppercase
std::string to_uppercase(std::string input);

// from
// https://stackoverflow.com/questions/874134/find-out-if-string-ends-with-another-string-in-c
static bool endsWith(const std::string& str, const std::string& suffix) {
  return str.size() >= suffix.size() &&
         0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}
static bool startsWith(const std::string& str, const std::string& prefix) {
  return str.size() >= prefix.size() &&
         0 == str.compare(0, prefix.size(), prefix);
}

// returns true if s1 contains s2
static bool contains(const std::string& s1, const std::string& s2) {
  return s1.find(s2) != std::string::npos;
}

// Converts both strings to uppercase, then checks if
// s1 contains s2 (we can find s2 in s1).
// Returns true if (after uppercase) s1 contains s2, false otherwise
static bool contains_after_uppercase(const std::string& s1,
                                     const std::string& s2) {
  const auto s1_upper = OHDUtil::to_uppercase(s1);
  const auto s2_upper = OHDUtil::to_uppercase(s2);
  return s1_upper.find(s2_upper) != std::string::npos;
}

static bool str_equal(const std::string& s1, const std::string& s2) {
  return s1 == s2;
}

// Converts both strings to uppercase, then checks if
// s1 has exactly the same content as s2
static bool equal_after_uppercase(const std::string& s1,
                                  const std::string& s2) {
  return str_equal(OHDUtil::to_uppercase(s1), OHDUtil::to_uppercase(s2));
}

// From https://stackoverflow.com/questions/216823/how-to-trim-an-stdstring
void rtrim(std::string& s);
void ltrim(std::string& s);
void trim(std::string& s);

// Create a command as a string using the command name (e.g. "run") and a vec of
// arguments (e.g. {"--seconds","10"}
std::string create_command_with_args(const std::string& command,
                                     const std::vector<std::string>& args);

/**
 * Utility to execute a command on the command line.
 * Blocks until the command has been executed, and returns its result.
 * @param command the command to run
 * @param args the args for the command to run
 * @param print_debug print the command executed, this can be usefully for
 * debugging -to replicate, just copy the command from the log message
 * @return the command result
 */
int run_command(const std::string& command,
                const std::vector<std::string>& args, bool print_debug = true);

/**
 * from
 * https://stackoverflow.com/questions/646241/c-run-a-system-command-and-get-output
 * also see https://linux.die.net/man/3/popen
 * Not sure how to describe this - it runs a command and returns its shell
 * output. NOTE: This just returns the shell output, it does not check if the
 * executed command is actually available on the system. If the command is not
 * available on the system, it most likely returns "command not found" as a
 * string.
 * @param command the command and its args to run
 * @return the shell output, or std::nullopt if something went wrong.
 */
std::optional<std::string> run_command_out(const std::string& command,
                                           bool debug = false);

// I use this one during testing a lot, when a module's functionality uses a
// start() / stop() pattern with its own thread. This keeps the current thread
// alive, until a sigterm (CTR+X) happens
void keep_alive_until_sigterm();

/**
 * based on https://man7.org/linux/man-pages/man3/inet_pton.3.html
 * returns true if the given ip is valid, false otherwise
 */
bool is_valid_ip(const std::string& ip);

/**
 * boolean to "Y" or "N"
 */
static std::string yes_or_no(bool yes) { return (yes ? "Y" : "N"); }

// from https://stackoverflow.com/questions/3339200/get-string-between-2-strings
std::string string_in_between(const std::string& start, const std::string& end,
                              const std::string& value, bool debug = false);

/**
 * All these string_to_xxx methods are guaranteed to never throw an exception
 * and return std::nullopt in case parsing fails.
 */
std::optional<int> string_to_int(const std::string& s);
std::optional<long> string_to_long(const std::string& s);
std::optional<float> string_to_float(const std::string& s);
std::optional<long> string_to_long_hex(const std::string& s);

bool get_nth_bit(long number, int position);

// Example: split "hello,world" int "hello" and "world" by ","
std::vector<std::string> split_into_substrings(const std::string& input,
                                               char separator);

// From
// https://stackoverflow.com/questions/3214297/how-can-my-c-c-application-determine-if-the-root-user-is-executing-the-command
// Returns true if the caller is running as root.
bool check_root(bool print_debug = true);

void terminate_if_not_root();

// Example: "export OHD_DISCOVER_CAMERAS_DEBUG=1"
// -> then this method will return true if you query
// "OHD_DISCOVER_CAMERAS_DEBUG"
bool get_ohd_env_variable_bool(const std::string& name);

// For some actions (e.g a reset) a user can create a plain text file, then
// reboot when openhd is started, we check if this file exist, perform the
// appropriate action, and then delete the file. Aka a pattern of: If file
// exists, do action, then do not do the same action again on the next reboot
bool file_exists_and_delete(const char* filename);

template <class T>
static void vec_append(std::vector<T>& dest, const std::vector<T>& src) {
  dest.insert(dest.end(), src.begin(), src.end());
}

// From https://stackoverflow.com/questions/13172158/c-split-string-by-line
// Split the given string into lines
/**
 * Modified, but from
 * https://stackoverflow.com/questions/13172158/c-split-string-by-line Split the
 * given string into lines
 * @param str a string, for example optained by reading a .txt file
 * @return a list of all the lines found, each element ends with a "\n";
 */
std::vector<std::string> split_string_by_newline(
    const std::string& str, bool include_newline_character = true);

std::string create_string_from_lines(const std::vector<std::string>& lines);

template <typename T>
std::string vec_as_string(const std::vector<T>& v);

std::string str_vec_as_string(const std::vector<std::string>& v);

std::string bytes_as_string(const uint8_t* data, int data_len);

// maps [0,100] to [-1.0,1.0] with 50% == 0.0
float map_int_percentage_to_minus1_to_1(int percentage);
// maps [0,200] to [-1.0,1.0] with 100% = 0.0
float map_int_percentage_0_200_to_minus1_to_1(int percentage);

int8_t calculate_progress_perc(int progress, int total_count);

std::string int_as_string(int number);

std::string password_as_hidden_str(const std::string& pw);
}  // namespace OHDUtil

#endif

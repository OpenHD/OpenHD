#ifndef OPENHD_UTIL_H
#define OPENHD_UTIL_H

#include <sstream>
#include <iostream>
#include <cctype>
#include <cstdlib>
#include <vector>
#include <memory>
#include <optional>
#include <string>
#include <regex>

namespace OHDUtil {

static std::string to_uppercase(std::string input) {
  for (char &it: input) {
	it = toupper((unsigned char)it);
  }
  return input;
}

// from https://stackoverflow.com/questions/874134/find-out-if-string-ends-with-another-string-in-c
static bool endsWith(const std::string& str, const std::string& suffix){
  return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}
static bool startsWith(const std::string& str, const std::string& prefix){
  return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

/**
 * Utility to execute a command on the command line.
 * Blocks until the command has been executed, and returns its result.
 * @param command the command to run
 * @param args the args for the command to run
 * @return the command result
 * NOTE: Used to use boost, there were issues with that, I changed it to use c standard library.
 */
static bool run_command(const std::string &command, const std::vector<std::string> &args) {
  std::stringstream ss;
  ss << command;
  for (const auto &arg: args) {
	ss << " " << arg;
  }
  std::cout << "run command begin [" << ss.str() << "]\n";
  // Some weird locale issue ?!
  // https://man7.org/linux/man-pages/man3/system.3.html
  auto ret = system(ss.str().c_str());
  // With boost, there is this locale issue ??!!
  /*boost::process::child c(boost::process::search_path(command), args);
  c.wait();
  std::cout<<"Run command end\n";
  return c.exit_code() == 0;*/
  std::cout << "Run command end\n";
  return ret;
}

/**
 * from https://stackoverflow.com/questions/646241/c-run-a-system-command-and-get-output
 * also see https://linux.die.net/man/3/popen
 * Not sure how to describe this - it runs a command and returns its shell output.
 * NOTE: This just returns the shell output, it does not check if the executed command is actually available on
 * the system. If the command is not available on the system, it most likely returns "command not found" as a string.
 * @param command the command and its args to run
 * @return the shell output, or std::nullopt if something went wrong.
 */
static std::optional<std::string> run_command_out(const char* command){
  std::string raw_value;
  std::array<char, 512> buffer{};
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
  if (!pipe) {
	// if the pipe opening fails, this doesn't mean the command failed (see above)
	// But rather we need to find a different way to implement this functionality on this platform.
	std::stringstream ss;
	ss<<"run_command_out with "<<command<<" cannot open pipe";
	return std::nullopt;
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
	raw_value += buffer.data();
  }
  return raw_value;
}

// Tries to extract a valid ip from the given input string.
// Returns std::nullopt on error, otherwise a always "valid" ip in string from
/*static bool createValidIp(const std::string input){
  // Regex expression for validating IPv4
  std::regex regex_ipv4("(([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])");
  if (regex_match(input, regex_ipv4))
	return true;
  return false;
}*/

}

#endif

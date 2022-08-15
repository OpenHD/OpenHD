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
#include <csignal>
#include <thread>
#include <arpa/inet.h>
#include <regex>
#include <unistd.h>

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
 * @param print_debug print debug to std::cout, really usefully for debugging. true by default.
 * @return the command result
 * NOTE: Used to use boost, there were issues with that, I changed it to use c standard library.
 */
static bool run_command(const std::string &command, const std::vector<std::string> &args,bool print_debug=true) {
  std::stringstream ss;
  ss << command;
  for (const auto &arg: args) {
	ss << " " << arg;
  }
  if(print_debug){
	std::stringstream log;
	log<< "run command begin [" << ss.str() << "]\n";
	std::cout<<log.str();
  }
  // Some weird locale issue ?!
  // https://man7.org/linux/man-pages/man3/system.3.html
  auto ret = system(ss.str().c_str());
  // With boost, there is this locale issue ??!!
  /*boost::process::child c(boost::process::search_path(command), args);
  c.wait();
  std::cout<<"Run command end\n";
  return c.exit_code() == 0;*/
  if(print_debug){
	std::cout << "Run command end\n";
  }
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

// i use this one during testing a lot, when a module's functionality uses a start() / stop()
// pattern with its own thread. This keeps the current thread alive, until a sigterm (CTR+X) happens
static void keep_alive_until_sigterm(){
  static bool quit=false;
  signal(SIGTERM, [](int sig){ quit= true;});
  while (!quit){
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout<<"keep_alive_until_sigterm\n";
  }
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
// based on https://man7.org/linux/man-pages/man3/inet_pton.3.html
static bool is_valid_ip(const std::string& ip){
  unsigned char buf[sizeof(struct in6_addr)];
  auto result= inet_pton(AF_INET,ip.c_str(), buf);
  return result==1;
}

static std::string yes_or_no(bool yes){
  return (yes ? "Y" : "N");
}

// from https://stackoverflow.com/questions/3339200/get-string-between-2-strings
static std::string string_in_between(const std::string& start,const std::string& end,const std::string& value){
  std::regex base_regex(start + "(.*)" + end);
  std::smatch base_match;
  std::string matched;
  if (std::regex_search(value, base_match, base_regex)) {
	// The first sub_match is the whole string; the next
	// sub_match is the first parenthesized expression.
	if (base_match.size() == 2) {
	  matched = base_match[1].str();
	}
  }
  std::stringstream ss;
  ss<<"Given:{"<<value<<"}\n";
  ss<<"Result:{"<<matched<<"}\n";
  std::cout <<ss.str();
  return matched;
}

// From https://stackoverflow.com/questions/3214297/how-can-my-c-c-application-determine-if-the-root-user-is-executing-the-command
// Returns true if the caller is running as root.
static bool check_root(){
  const auto uid=getuid();
  const bool root= uid ? false:true;
  std::stringstream ss;
  ss<<"UID is:["<<uid<<"] root:"<<OHDUtil::yes_or_no(root)<<"\n";
  std::cout<<ss.str();
  return root;
}

static void terminate_if_not_root(){
  if(!check_root()){
	std::cout<<"ERROR not root,terminating. Run OpenHD with root privileges.\n";
	exit(EXIT_FAILURE);
  }
}

}

#endif

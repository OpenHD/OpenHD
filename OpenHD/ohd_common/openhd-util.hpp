#ifndef OPENHD_UTIL_H
#define OPENHD_UTIL_H

#include <arpa/inet.h>
#include <unistd.h>

#include <cctype>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "openhd-spdlog.hpp"
#include "openhd-util-filesystem.hpp"

namespace OHDUtil {

// Converts all letters in the given string to uppercase
static std::string to_uppercase(std::string input) {
  for (char &it: input) {
	// https://cplusplus.com/reference/cctype/toupper/
	it = toupper((unsigned char)it); // NOLINT(cppcoreguidelines-narrowing-conversions)
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

static bool contains(const std::string& s1,const std::string& s2){
  return s1.find(s2)!= std::string::npos;;
}

// Converts both strings to uppercase, then checks if
// s1 contains s2 (we can find s2 in s1).
// Returns true if (after uppercase) s1 contains s2, false otherwise
static bool contains_after_uppercase(const std::string& s1,const std::string& s2){
  const auto s1_upper=OHDUtil::to_uppercase(s1);
  const auto s2_upper=OHDUtil::to_uppercase(s2);
  return s1_upper.find(s2_upper)!= std::string::npos;
}

// Converts both strings to uppercase, then checks if
// s1 has exactly the same content as s2
static bool equal_after_uppercase(const std::string& s1,const std::string& s2){
  const auto s1_upper=OHDUtil::to_uppercase(s1);
  const auto s2_upper=OHDUtil::to_uppercase(s2);
  return s1_upper==s2_upper;
}

// From https://stackoverflow.com/questions/216823/how-to-trim-an-stdstring
static void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
          }).base(), s.end());
}

static std::string create_command_with_args(const std::string& command,const std::vector<std::string> &args){
  std::stringstream ss;
  ss << command;
  for (const auto &arg: args) {
    ss << " " << arg;
  }
  return ss.str();
}

/**
 * Utility to execute a command on the command line.
 * Blocks until the command has been executed, and returns its result.
 * @param command the command to run
 * @param args the args for the command to run
 * @param print_debug print the command executed, this can be usefully for debugging -to replicate, just copy the command
 * from the log message
 * @return the command result
 * NOTE: Do not use boost, it has issues
 */
static int run_command(const std::string &command, const std::vector<std::string> &args,bool print_debug=true) {
  const auto command_with_args= create_command_with_args(command,args);
  if(print_debug){
    openhd::log::get_default()->debug("run command begin [{}]",command_with_args);
  }
  // https://man7.org/linux/man-pages/man3/system.3.html
  const auto ret = std::system(command_with_args.c_str());
  //openhd::log::get_default()->debug("return code:{}",ret);
  if(ret<0){
    openhd::log::get_default()->warn("Invalid command, return code {}",ret);
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
static std::optional<std::string> run_command_out(const std::string &command,const bool debug=false){
  if(debug){
    openhd::log::get_default()->debug("run command out begin [{}]",command);
  }
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
  if (!pipe) {
    // if the pipe opening fails, this doesn't mean the command failed (see above)
    // But rather we need to find a different way to implement this functionality on this platform.
    openhd::log::get_default()->error("Cannot execute command [{}]",command);
    return std::nullopt;
  }
  std::string raw_value;
  std::array<char, 512> buffer{};
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
	raw_value += buffer.data();
  }
  return raw_value;
}

// The above implementation sometimes doesn't work
// Execute a shell command and return its output as a string
/*struct RunShellCommandResult{
  int status;
  std::string status_text;
};
static std::string run_command_out2(const char* command,const bool debug=false){
  const std::string output_filename="/tmp/command_output.txt";
  OHDFilesystemUtil::remove_if_existing(output_filename);
  const std::string command_outputting_to_tmp_file=std::string(command)+" 2>&1 "+output_filename;
  if(debug){
    openhd::log::get_default()->debug("run_command_out2 begin [{}]",command_outputting_to_tmp_file);
  }
  const int status = std::system(command_outputting_to_tmp_file.c_str()); // execute the shell command
  std::string ret=OHDFilesystemUtil::read_file(output_filename);
  openhd::log::get_default()->debug("Done result code: {} text:[{}]",status,ret);
  OHDFilesystemUtil::remove_if_existing(output_filename);
  return ret;
}*/

// I use this one during testing a lot, when a module's functionality uses a start() / stop()
// pattern with its own thread. This keeps the current thread alive, until a sigterm (CTR+X) happens
static void keep_alive_until_sigterm(){
  static bool quit=false;
  signal(SIGTERM, [](int sig){ quit= true;});
  while (!quit){
	std::this_thread::sleep_for(std::chrono::seconds(1));
	openhd::log::get_default()->debug("keep_alive_until_sigterm");
  }
}


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
  ss<<"Given:["<<value<<"] ";
  ss<<"Result:["<<matched<<"]";
  openhd::log::get_default()->debug(ss.str());
  return matched;
}

static std::optional<int> string_to_int(const std::string& s){
  try{
    auto ret=std::stoi(s);
    return ret;
  }catch (...){
    openhd::log::get_default()->warn("Cannot convert ["+s+"] to int");
    return std::nullopt;
  }
}

// Example: split "hello,world" int "hello" and "world" by ","
static std::vector<std::string> split_into_substrings(const std::string& input,const char separator){
  std::vector<std::string> ret;
  std::string buff;
  for(int i=0;i<input.size();i++){
    const auto curr_char=input.at(i);
    if(curr_char==separator){
      if(!buff.empty())ret.push_back(buff);
      buff="";
    } else{
      buff+=curr_char;
    }
  }
  if(!buff.empty())ret.push_back(buff);
  return ret;
}

// From https://stackoverflow.com/questions/3214297/how-can-my-c-c-application-determine-if-the-root-user-is-executing-the-command
// Returns true if the caller is running as root.
static bool check_root(const bool print_debug=true){
  const auto uid=getuid();
  const bool root= uid ? false:true;
  if(print_debug){
    openhd::log::get_default()->debug("UID is:{} root: {}",uid,OHDUtil::yes_or_no(root));
  }
  return root;
}

static void terminate_if_not_root(){
  if(!check_root(false)){
	std::cout<<"ERROR not root,terminating. Run OpenHD with root privileges.\n";
	exit(EXIT_FAILURE);
  }
}

// Example: "export OHD_DISCOVER_CAMERAS_DEBUG=1"
// -> then this method will return true if you query "OHD_DISCOVER_CAMERAS_DEBUG"
static bool get_ohd_env_variable_bool(const char* name){
  if (const char* env_p = std::getenv(name)) {
	if (std::string(env_p) == "1") {
	  return true;
	}
  }
  return false;
}

static bool get_ohd_env_variable_bool(const std::string& name){
  if(OHDUtil::contains_after_uppercase(name,"OHD_")){
	std::cout<<"Please prefix OpenHD env variables with {OHD_}\n";
  }
  return get_ohd_env_variable_bool(name.c_str());
}

// For some actions (e.g a reset) a user can create a plain text file, then reboot
// when openhd is started, we check if this file exist, perform the appropriate action, and then
// delete the file.
// Aka a pattern of: If file exists, do action, then do not do the same action again on the next reboot
static bool file_exists_and_delete(const char* filename){
  bool ret=false;
  if(OHDFilesystemUtil::exists(filename)){
    ret= true;
    openhd::log::get_default()->warn("Got action, deleting {}",filename);
    OHDFilesystemUtil::remove_if_existing(filename);
  }
  return ret;
}


}

#endif

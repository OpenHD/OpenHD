//
// Created by consti10 on 09.02.23.
//

#include "openhd_util.h"

#include <arpa/inet.h>
#include <unistd.h>

#include <cctype>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <regex>
#include <string>
#include <thread>
#include <vector>

#include "openhd_spdlog.h"
#include "openhd_util_filesystem.h"

std::string OHDUtil::to_uppercase(std::string input) {
  for (char& it : input) {
    // https://cplusplus.com/reference/cctype/toupper/
    it = toupper(
        (unsigned char)it);  // NOLINT(cppcoreguidelines-narrowing-conversions)
  }
  return input;
}

int OHDUtil::run_command(const std::string& command,
                         const std::vector<std::string>& args,
                         bool print_debug) {
  const auto command_with_args = create_command_with_args(command, args);
  if (print_debug) {
    openhd::log::get_default()->debug("run command begin [{}]",
                                      command_with_args);
  }
  // https://man7.org/linux/man-pages/man3/system.3.html
  const auto ret = std::system(command_with_args.c_str());
  // openhd::log::get_default()->debug("return code:{}",ret);
  if (ret < 0) {
    openhd::log::get_default()->warn("Invalid command, return code {}", ret);
  }
  return ret;
}

std::optional<std::string> OHDUtil::run_command_out(const std::string& command,
                                                    const bool debug) {
  if (debug) {
    openhd::log::get_default()->debug("run command out begin [{}]", command);
  }
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"),
                                                pclose);
  if (!pipe) {
    // if the pipe opening fails, this doesn't mean the command failed (see above) But rather we need to find a different way to implement this functionality on this platform.
    openhd::log::get_default()->error("Cannot execute command [{}]", command);
    return std::nullopt;
  }
  std::string raw_value;
  std::array<char, 512> buffer{};
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    raw_value += buffer.data();
  }
  return raw_value;
}

void OHDUtil::keep_alive_until_sigterm() {
  static bool quit = false;
  signal(SIGTERM, [](int sig) { quit = true; });
  while (!quit) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    openhd::log::get_default()->debug("keep_alive_until_sigterm");
  }
}

bool OHDUtil::is_valid_ip(const std::string& ip) {
  unsigned char buf[sizeof(struct in6_addr)];
  auto result = inet_pton(AF_INET, ip.c_str(), buf);
  return result == 1;
}

std::string OHDUtil::string_in_between(const std::string& start,
                                       const std::string& end,
                                       const std::string& value, bool debug) {
  const std::regex base_regex(start + "(.*)" + end);
  std::smatch base_match;
  std::string matched;
  if (std::regex_search(value, base_match, base_regex)) {
    // The first sub_match is the whole string; the next
    // sub_match is the first parenthesized expression.
    if (base_match.size() == 2) {
      matched = base_match[1].str();
    }
  }
  if(debug){
    openhd::log::get_default()->debug("Given:[{}] Result:[{}]",value,matched);
  }
  return matched;
}
std::optional<int> OHDUtil::string_to_int(const std::string& s) {
  try {
    auto ret = std::stoi(s);
    return ret;
  } catch (...) {
    openhd::log::get_default()->warn("Cannot convert [{}] to int",s);
    return std::nullopt;
  }
}

std::optional<long> OHDUtil::string_to_long(const std::string& s) {
  try {
    auto ret = std::stol(s);
    return ret;
  } catch (...) {
    openhd::log::get_default()->warn("Cannot convert [{}] to long",s);
    return std::nullopt;
  }
}

std::optional<float> OHDUtil::string_to_float(const std::string& s) {
  try {
    auto ret = std::stof(s);
    return ret;
  } catch (...) {
    openhd::log::get_default()->warn("Cannot convert [{}] to float",s);
    return std::nullopt;
  }
}

std::vector<std::string> OHDUtil::split_into_substrings(
    const std::string& input, const char separator) {
  std::vector<std::string> ret;
  std::string buff;
  for (int i = 0; i < input.size(); i++) {
    const auto curr_char = input.at(i);
    if (curr_char == separator) {
      if (!buff.empty()) ret.push_back(buff);
      buff = "";
    } else {
      buff += curr_char;
    }
  }
  if (!buff.empty()) ret.push_back(buff);
  return ret;
}

bool OHDUtil::file_exists_and_delete(const char* filename) {
  bool ret = false;
  if (OHDFilesystemUtil::exists(filename)) {
    ret = true;
    openhd::log::get_default()->warn("Got action, deleting {}", filename);
    OHDFilesystemUtil::remove_if_existing(filename);
  }
  return ret;
}

std::vector<std::string> OHDUtil::split_string_by_newline(
    const std::string& str, const bool include_newline_character) {
  auto result = std::vector<std::string>{};
  auto ss = std::stringstream{str};

  for (std::string line; std::getline(ss, line, '\n');) {
    const auto tmp=line+(include_newline_character ? "\n" : "");
    result.push_back(tmp);
  }
  return result;
}

std::string OHDUtil::create_string_from_lines(
    const std::vector<std::string>& lines) {
  std::stringstream ss;
  for(const auto& line:lines){
    assert(OHDUtil::endsWith(line,"\n"));
    ss<<line;
  }
  return ss.str();
}

void OHDUtil::rtrim(std::string& s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); })
              .base(),
          s.end());
}

std::string OHDUtil::create_command_with_args(
    const std::string& command, const std::vector<std::string>& args) {
  std::stringstream ss;
  ss << command;
  for (const auto& arg : args) {
    ss << " " << arg;
  }
  return ss.str();
}

bool OHDUtil::check_root(const bool print_debug) {
  const auto uid = getuid();
  const bool root = uid ? false : true;
  if (print_debug) {
    openhd::log::get_default()->debug("UID is:{} root: {}", uid,
                                      OHDUtil::yes_or_no(root));
  }
  return root;
}

void OHDUtil::terminate_if_not_root() {
  if (!check_root(false)) {
    std::cout
        << "ERROR not root,terminating. Run OpenHD with root privileges.\n";
    exit(EXIT_FAILURE);
  }
}

bool OHDUtil::get_ohd_env_variable_bool(const std::string& name) {
  if (OHDUtil::contains_after_uppercase(name, "OHD_")) {
    std::cout << "Please prefix OpenHD env variables with {OHD_}\n";
  }
  if (const char* env_p = std::getenv(name.c_str())) {
    if (std::string(env_p) == "1") {
      return true;
    }
  }
  return false;
}

template <typename T>
std::string OHDUtil::vec_as_string(const std::vector<T>& v) {
  std::stringstream ss;
  ss << "[";
  for(int i=0;i<v.size();i++){
    ss << std::to_string(v[i]);
    //ss<<v[i];
    if(i!=v.size()-1){
      ss<<",";
    }
  }
  ss << "]";
  return ss.str();
}

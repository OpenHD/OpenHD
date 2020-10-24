
#ifndef OPENHD_SETTINGS_H
#define OPENHD_SETTINGS_H

#include <exception> // exception
#include <stdexcept> // runtime_error
#include <string>
#include <utility> // make_pair

#include <boost/regex.hpp>

std::pair<std::string, std::string> parse_kv(std::string kv) {
    boost::smatch result;

    boost::regex r{ "^([\\w\\[\\]]+)\\s*=\\s*(.*)"};
    if (!boost::regex_match(kv, result, r)) {
        throw std::runtime_error("Ignoring invalid setting, check file for errors");
    }

    if (result.size() != 3) {
        throw std::runtime_error("Ignoring invalid setting, check file for errors");
    }

    return std::make_pair<std::string, std::string>(result[1], result[2]);
}


std::map<std::string, std::string> read_config(std::string path) {
    std::ifstream in(path);

    std::map<std::string, std::string> settings;

    std::string str;

    while (std::getline(in, str)) {
        if (str.size() > 0) {
            try {
                auto pair = parse_kv(str);
                settings.insert(pair);
            } catch (std::exception &ex) {
                /* 
                 * Ignore, likely a comment or user error in which case we will use a default.
                 */
            }
        }
    }

    return settings;
}

#endif

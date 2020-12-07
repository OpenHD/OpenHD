
#ifndef OPENHD_SETTINGS_H
#define OPENHD_SETTINGS_H

#include <exception> // exception
#include <stdexcept> // runtime_error
#include <optional>
#include <string>
#include <utility> // make_pair

#include <boost/regex.hpp>

std::optional<std::string> parse_section(std::string line) {
    boost::smatch result;

    boost::regex r{ "^\\[([\\w\\s]+)\\]"};
    if (!boost::regex_match(line, result, r)) {
        return {};
    }

    if (result.size() != 2) {
        return {};
    }

    return result[1];
}

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


std::vector<std::map<std::string, std::string> > read_config(std::string path) {
    std::ifstream in(path);

    std::vector<std::map<std::string, std::string> > settings;

    std::string str;

    std::map<std::string, std::string> section;

    while (std::getline(in, str)) {
        if (str.size() > 0) {
            try {
                auto section_name = parse_section(str);

                if (section_name) {
                    if (section.size() > 0) {
                        settings.push_back(section);
                        section.clear();
                    }

                    continue;
                }
            } catch (std::exception &ex) {
                /* 
                 * Ignore, likely a comment or user error in which case we will use a default.
                 */
            }

            try {
                auto pair = parse_kv(str);
                section.insert(pair);
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

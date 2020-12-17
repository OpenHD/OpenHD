
#ifndef OPENHD_SETTINGS_H
#define OPENHD_SETTINGS_H

#include <exception> // exception
#include <stdexcept> // runtime_error
#include <optional>
#include <string>
#include <fstream>
#include <streambuf>
#include <utility> // make_pair

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>


inline std::string find_settings_path(bool is_air, std::string unit_id) {
    std::string config_path;
    std::string base_path = "/conf/openhd";

    boost::filesystem::create_directory(base_path);

    if (is_air) {
        for (auto & p : boost::filesystem::recursive_directory_iterator(base_path)) {            
            if (p.path() == "/conf/openhd/unit.id") {
                continue;
            }
                
            if (p.path().filename() == "unit.id") {
                std::ifstream f(p.path().string());
                std::string _unit_id((std::istreambuf_iterator<char>(f)),
                                      std::istreambuf_iterator<char>());

                if (_unit_id == unit_id) {
                    config_path = p.path().parent_path().string();
                    break;
                }
            }
        }
    } else {
        config_path = base_path + "/ground";
    }

    if (config_path.size() == 0) {
        throw std::runtime_error("Settings directory missing2");
    }

    return config_path;
}


inline std::string create_settings_path(bool is_air, std::string unit_id) {
    std::string config_path;
    std::string base_path = "/conf/openhd";

    boost::filesystem::create_directory(base_path);

    if (is_air) {
        for (int drone_index = 1; drone_index <= 100; drone_index++) {
            std::string drone_path = base_path + "/vehicle" + std::to_string(drone_index);

            if (boost::filesystem::is_directory(drone_path)) {
                continue;
            }

            config_path = drone_path;
            
            break;
        }
    } else {
        config_path = base_path + "/ground";
    }

    if (config_path.size() == 0) {
        throw std::runtime_error("Settings files missing");
    }

    boost::filesystem::create_directory(config_path);

    std::string unit_id_file = config_path + "/unit.id";

    std::ofstream _f(unit_id_file);
    _f << unit_id;
    _f.close();

    return config_path;
}


inline std::optional<std::string> parse_section(std::string line) {
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


inline std::pair<std::string, std::string> parse_kv(std::string kv) {
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


inline std::vector<std::map<std::string, std::string> > read_config(std::string path) {
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

    /*
     * Push the last section since there aren't any more in the file for the earlier check to be trigged by
     */
    settings.push_back(section);

    return settings;
}

#endif


#ifndef OPENHD_SETTINGS_H
#define OPENHD_SETTINGS_H

#include <exception>
#include <stdexcept>
#include <optional>
#include <string>
#include <fstream>
#include <streambuf>
#include <utility>
#include <optional>
#include <iostream>
#include <fstream>

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

// from https://superuser.com/questions/631859/preferred-place-to-store-configuration-files-that-change-often
// All persistent settings are written into this directory.
//static constexpr auto BASE_PATH="/etc/opt/openhd/";
static constexpr auto BASE_PATH="/home/consti10/openhd/";
// for example, the unique id
static const auto UNIT_ID_FILE=std::string(BASE_PATH)+"unit.id";

/**
 * If the directory does not exist yet,
 * generate the directory where all persistent settings of OpenHD are stored.
 */
static void generateSettingsDirectoryIfNonExists(){
    if(!boost::filesystem::exists(BASE_PATH)){
        std::cout<<"Creating settings directory\n";
        if(!boost::filesystem::create_directory(BASE_PATH)){
            std::cerr<<"Cannot create settings directory\n";
        }
    }
    assert(boost::filesystem::exists(BASE_PATH));
}

/**
 * If no unit id file exists, this is the first boot of this OpenHD image on the platform.
 * In this case, generate a new random unit id, and store it persistently.
 * Then return the unit id.
 * If a unit id file already exists, read and return the unit id.
 * @return the unit id, it doesn't change during reboots of the same system.
 */
static std::string getOrCreateUnitId(){
    std::ifstream unit_id_file(UNIT_ID_FILE);
    std::string unit_id;
    if(!unit_id_file.is_open()){
        //std::cout<<"Generating new unit id\n";
        // generate new unit id
        const boost::uuids::uuid uuid=boost::uuids::random_generator()();
        unit_id=boost::lexical_cast<std::string>(uuid);
        std::cout<<"Created new unit id:["<<unit_id<<"]\n";
        // and write it ot to the right file
        std::ofstream of(UNIT_ID_FILE);
        of << uuid;
        of.close();
    }else{
        //std::cout<<"Unit id exists, reading\n";
        unit_id=std::string((std::istreambuf_iterator<char>(unit_id_file)),
                            std::istreambuf_iterator<char>());
        std::cout<<"Read unit id:["<<unit_id<<"]\n";
        return unit_id;
    }
    assert(!unit_id.empty());
    return unit_id;
}

/**
 * The settings are stored in a directory called air_$unit_id or ground_$unit_id.
 * @return the settings directory, created newly if non existent. As an example, it will return a path like
 * this: BASE_PATH/air_8bfff348-c17e-4833-af66-cef83f90c208/
 */
static std::string findOrCreateSettingsDirectory(bool is_air){
    std::stringstream settingsPath;
    settingsPath<<BASE_PATH;
    settingsPath<<(is_air ? "air_":"ground_");
    const auto unit_id=getOrCreateUnitId();
    settingsPath<<unit_id;
    const auto str=settingsPath.str();
    std::cout<<"SettingsDirectory:["<<str<<"]\n";
    // create the directory if it is non existing
    if(!boost::filesystem::exists(str.c_str())){
        boost::filesystem::create_directory(str.c_str());
    }
    assert(boost::filesystem::exists(str.c_str()));
    return str;
}

// ------------------ Stephen code, undocumented ------------------------------

inline std::optional<std::string> parse_section(const std::string& line) {
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
        throw std::invalid_argument("Ignoring invalid setting, check file for errors");
    }

    if (result.size() != 3) {
        throw std::invalid_argument("Ignoring invalid setting, check file for errors");
    }

    return std::make_pair<std::string, std::string>(result[1], result[2]);
}


inline std::vector<std::map<std::string, std::string> > read_config(const std::string& path) {
    std::ifstream in(path);

    std::vector<std::map<std::string, std::string> > settings;

    std::string str;

    std::map<std::string, std::string> section;

    while (std::getline(in, str)) {
        if (!str.empty()) {
            try {
                auto section_name = parse_section(str);

                if (section_name) {
                    if (!section.empty()) {
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

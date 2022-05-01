#ifndef OPENHD_UTIL_H
#define OPENHD_UTIL_H


#include <boost/process.hpp>

inline std::string to_uppercase(std::string input) {
    for (std::string::iterator it = input.begin(); it != input.end(); ++ it) {
        *it = toupper((unsigned char)*it);
    }

    return input;
}


inline bool run_command(std::string command, std::vector<std::string> args) {    
    boost::process::child c(boost::process::search_path(command), args);

    c.wait();

    return c.exit_code() == 0;
}

// Determine at run time if we are running on air or ground pi
// !!!!! Needs to be used AFTER openhd-system has written all the config stuff !!!!
static bool runs_on_air(){
    bool is_air = false;
    try {
        std::ifstream f("/tmp/profile_manifest");
        nlohmann::json j;
        f >> j;
        is_air = j["is-air"];
    } catch (std::exception &ex) {
        std::cerr << "Profile manifest processing failed: " << ex.what() << std::endl;
        ohd_log(STATUS_LEVEL_EMERGENCY, "Profile manifest processing failed");
        exit(1);
    }
    return is_air;
}


#endif

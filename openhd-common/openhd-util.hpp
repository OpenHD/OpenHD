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


#endif

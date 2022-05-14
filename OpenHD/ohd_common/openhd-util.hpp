#ifndef OPENHD_UTIL_H
#define OPENHD_UTIL_H

#include <sstream>
#include <iostream>
#include <cctype>
#include <cstdlib>
#include <vector>

inline std::string to_uppercase(std::string input) {
    for (char & it : input) {
        it = toupper((unsigned char)it);
    }
    return input;
}


/**
 * Utility to execute a command on the command line.
 * Blocks until the command has been executed, and returns its result.
 * @param command the command to run
 * @param args the args for the command to run
 * @return the command result
 * NOTE: Used to use boost, there were issues with that, I changed it to use c standard library.
 */
inline bool run_command(const std::string& command,const std::vector<std::string>& args) {
    std::stringstream ss;
    ss<<command;
    for(const auto& arg:args){
        ss<<" "<<arg;
    }
    std::cout<<"run command begin ["<<ss.str()<<"]\n";
    // Some weird locale issue ?!
    // https://man7.org/linux/man-pages/man3/system.3.html
    auto ret=system(ss.str().c_str());
    // With boost, there is this locale issue ??!!
    /*boost::process::child c(boost::process::search_path(command), args);
    c.wait();
    std::cout<<"Run command end\n";
    return c.exit_code() == 0;*/
    std::cout<<"Run command end\n";
    return ret;
}


#endif

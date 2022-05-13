#ifndef OPENHD_UTIL_H
#define OPENHD_UTIL_H

#include <boost/process.hpp>
#include <sstream>
#include <iostream>
#include <cctype>

inline std::string to_uppercase(std::string input) {
    for (char & it : input) {
        it = toupper((unsigned char)it);
    }
    return input;
}


inline bool run_command(std::string command, std::vector<std::string> args) {
    std::stringstream ss;
    ss<<"["<<command;
    for(const auto& arg:args){
        ss<<arg<<" ";
    }
    ss<<"]";
    std::cout<<"run command begin "<<ss.str()<<"\n";
    boost::process::child c(boost::process::search_path(command), args);
    c.wait();
    std::cout<<"Run command end\n";
    return c.exit_code() == 0;
}


#endif

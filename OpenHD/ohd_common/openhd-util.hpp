#ifndef OPENHD_UTIL_H
#define OPENHD_UTIL_H

#include <boost/process.hpp>
#include <sstream>
#include <iostream>
#include <cctype>
#include <stdlib.h>

inline std::string to_uppercase(std::string input) {
    for (char & it : input) {
        it = toupper((unsigned char)it);
    }
    return input;
}


inline bool run_command(const std::string& command,const std::vector<std::string>& args) {
    std::stringstream ss;
    ss<<command;
    for(const auto& arg:args){
        ss<<arg<<" ";
    }
    std::cout<<"run command begin ["<<ss.str()<<"]\n";
    // Some weird locale issue ?!
    // https://man7.org/linux/man-pages/man3/system.3.html
    auto ret=system(ss.str().c_str());
    /*boost::process::child c(boost::process::search_path(command), args);
    c.wait();
    std::cout<<"Run command end\n";
    return c.exit_code() == 0;*/
    std::cout<<"Run command end\n";
    return ret;
}


#endif

//
// Created by Constantin on 09.10.2017.
//

#ifndef OSDTESTER_STRINGHELPER_H
#define OSDTESTER_STRINGHELPER_H

#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <cassert>

class StringHelper{
public:
    template<typename T>
    static std::string vectorAsString(const std::vector<T>& v){
        std::stringstream ss;
        ss<<"[";
        for (const auto i:v) {
            ss << (int)i << ",";
        }
        ss<<"]";
        return ss.str();
    }

    template<typename T,std::size_t S>
    static std::string arrayAsString(const std::array<T,S>& a){
        std::stringstream ss;
        ss<<"[";
        for (const auto i:a) {
            ss << (int)i << ",";
        }
        ss<<"]";
        return ss.str();
    }

    static std::string memorySizeReadable(const size_t sizeBytes){
        // more than one MB
        if(sizeBytes>1024*1024){
            float sizeMB=(float)sizeBytes /1024.0 / 1024.0;
            return std::to_string(sizeMB)+"mB";
        }
        // more than one KB
        if(sizeBytes>1024){
            float sizeKB=(float)sizeBytes /1024.0;
            return std::to_string(sizeKB)+"kB";
        }
        return std::to_string(sizeBytes)+"B";
    }
};


#endif //OSDTESTER_STRINGHELPER_H

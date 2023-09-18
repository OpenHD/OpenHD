//
// Created by consti10 on 06.11.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_INCLUDE_JSON_H_
#define OPENHD_OPENHD_OHD_COMMON_INCLUDE_JSON_H_

// NOTE: Please use this one everywhere instead of including the json.hpp directly, since we want to eventually
// switch to using json as a library
#include <nlohmann/json.hpp>

//#include <nlohmann/json_fwd.hpp>

#include <optional>
//#include "openhd_splog.h"

template<class T>
static std::optional<T> openhd_json_parse(const std::string& content){
    try{
        nlohmann::json j{content};
        auto tmp=j.get<T>();
        return tmp;
    }catch(nlohmann::json::exception& ex){
        //openhd::log::get_default()->warn("openhd_json_parse {}",ex.what());
        return std::nullopt;
    }
    return std::nullopt;
};

#endif  // OPENHD_OPENHD_OHD_COMMON_INCLUDE_JSON_H_

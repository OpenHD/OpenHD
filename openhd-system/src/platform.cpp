#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <fstream>

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem.hpp>

#include <boost/regex.hpp>

#include "json.hpp"

#include "platform.h"


constexpr char JETSON_BOARDID_PATH[] = "/proc/device-tree/nvidia,boardids";


Platform::Platform() {}


void Platform::discover() {
    std::cout << "Platform::discover()" << std::endl;

    detect_raspberrypi();
    detect_jetson();
    detect_pc();
}


void Platform::detect_raspberrypi() {
    std::ifstream t("/proc/cpuinfo");
    std::string raw_value((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    
    boost::smatch result;

    // example "Revision	: 2a020d3"
    boost::regex r { "Revision\\t*:\\s*([\\w]+)" };

    if (!boost::regex_search(raw_value, result, r)) {
        std::cerr << "no result" << std::endl;
        return;
    }

    if (result.size() != 2) {
        std::cerr << "result doesnt match" << std::endl;
        return;
    }

    m_platform_type = PlatformTypeRaspberryPi;

    std::string raspberry_identifier = result[1];

    std::set<std::string> pi4b_identifiers = { "a03111", "b03111", "b03112", "c03111", "c03112", "d03114" };
    if (pi4b_identifiers.find(raspberry_identifier) != pi4b_identifiers.end()) {
        m_board_type = BoardTypeRaspberryPi4B;
        m_wifi_hotspot_type = WiFiHotspotTypeInternalDualBand;
    }

    std::set<std::string> pi3b_identifiers = { "2a02082", "2a22082", "2a32082", "2a52082" };
    if (pi3b_identifiers.find(raspberry_identifier) != pi3b_identifiers.end()) {
        m_board_type = BoardTypeRaspberryPi3B;
        m_wifi_hotspot_type = WiFiHotspotTypeInternal2GBand;
    }

    std::set<std::string> pizero_identifiers = { "2900092", "2900093", "2920092", "2920093" };
    if (pizero_identifiers.find(raspberry_identifier) != pizero_identifiers.end()) {
        m_board_type = BoardTypeRaspberryPiZero;
        m_wifi_hotspot_type = WiFiHotspotTypeNone;
    }

    std::set<std::string> pi2b_identifiers = { "2a22042", "2a21041", "2a01041", "2a01040" };
    if (pi2b_identifiers.find(raspberry_identifier) != pi2b_identifiers.end()) {
        m_board_type = BoardTypeRaspberryPi2B;
        m_wifi_hotspot_type = WiFiHotspotTypeNone;
    }

    if (raspberry_identifier == "29020e0") {
        m_board_type = BoardTypeRaspberryPi3APlus;
        m_wifi_hotspot_type = WiFiHotspotTypeInternalDualBand;
    }

    if (raspberry_identifier == "2a020d3") {
        m_board_type = BoardTypeRaspberryPi3BPlus;
        m_wifi_hotspot_type = WiFiHotspotTypeInternalDualBand;
    }

    if (raspberry_identifier == "29000c1") {
        m_board_type = BoardTypeRaspberryPiZeroW;
        m_wifi_hotspot_type = WiFiHotspotTypeInternal2GBand;
    }
}


void Platform::detect_jetson() {
    if (boost::filesystem::exists(JETSON_BOARDID_PATH)) {
        m_platform_type = PlatformTypeJetson;
        m_board_type = BoardTypeJetsonNano;
    }
}


void Platform::detect_pc() {
    std::array<char, 512> buffer;
    std::string raw_value;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("arch", "r"), pclose);
    if (!pipe) {
        return;
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        raw_value += buffer.data();
    }

    boost::smatch result;

    boost::regex r1 { "x86_64" };
    auto res1 = boost::regex_search(raw_value, result, r1);

    boost::regex r2 { "i386" };
    auto res2 = boost::regex_search(raw_value, result, r2);

    if (!res1 && !res2) {
        return;
    }

    m_platform_type = PlatformTypePC;
    m_board_type = BoardTypeGenericPC;
}


std::string Platform::generate_manifest() {
    nlohmann::ordered_json j;
    
    j["platform"] = platform_type_string(m_platform_type);
    j["board"] = board_type_string(m_board_type);
    j["carrier"] = carrier_type_string(m_carrier_type);

    return j.dump(4);
}


std::string Platform::carrier_type_string(CarrierType carrier_type) {
    switch (carrier_type) {
        case CarrierTypeStereoPi: {
            return "stereopi";
        }
        case CarrierTypeComputeModuleIO: {
            return "computemoduleio";
        }
        case CarrierTypeJetsonNanoDevkit: {
            return "jetson-nano-devkit";
        }
        case CarrierTypeJetsonNCB00: {
            return "jetson-ncb00";
        }
        default: {
            return "none";
        }
    }
}


std::string Platform::board_type_string(BoardType board_type) {
    switch (board_type) {
        case BoardTypeRaspberryPiZero: {
            return "pizero";
        }
        case BoardTypeRaspberryPiZeroW: {
            return "pizerow";
        }
        case BoardTypeRaspberryPi2B: {
            return "pi2b";
        }
        case BoardTypeRaspberryPi3A: {
            return "pi3a";
        }
        case BoardTypeRaspberryPi3APlus: {
            return "pi3aplus";
        }
        case BoardTypeRaspberryPi3B: {
            return "pi3b";
        }
        case BoardTypeRaspberryPi3BPlus: {
            return "pi3bplus";
        }
        case BoardTypeRaspberryPiCM: {
            return "picm";
        }
        case BoardTypeRaspberryPiCM3: {
            return "picm3";
        }
        case BoardTypeRaspberryPiCM3Plus: {
            return "picm3plus";
        }
        case BoardTypeRaspberryPiCM4: {
            return "picm4";
        }
        case BoardTypeRaspberryPi4B: {
            return "pi4b";
        }
        case BoardTypeJetsonNano: {
            return "jetson-nano";
        }
        case BoardTypeJetsonTX1: {
            return "jetson-tx1";
        }
        case BoardTypeJetsonTX2: {
            return "jetson-tx2";
        }
        case BoardTypeJetsonNX: {
            return "jetson-nx";
        }
        case BoardTypeJetsonAGX: {
            return "jetson-agx";
        }
        case BoardTypeNanoPiNeo4: {
            return "nanopi-neo4";
        }
        case BoardTypeGenericPC: {
            return "generic-pc";
        }
        default: {
            return "unknown";
        }
    }
}


std::string Platform::platform_type_string(PlatformType platform_type) {
    switch (platform_type) {
        case PlatformTypeJetson: {
            return "jetson";
        }
        case PlatformTypeRaspberryPi: {
            return "raspberrypi";
        }
        case PlatformTypeNanoPi: {
            return "nanopi";
        }
        case PlatformTypePC: {
            return "pc";
        }
        default: {
            return "unknown";
        }
    }
}


#ifndef OPENHD_WIFI_H
#define OPENHD_WIFI_H


#include <string>

#include "openhd-util.hpp"
#include "openhd-log.hpp"

typedef enum WiFiCardType {
    WiFiCardTypeUnknown=0,
    WiFiCardTypeRealtek8812au,
    WiFiCardTypeRealtek8814au,
    WiFiCardTypeRealtek88x2bu,
    WiFiCardTypeRealtek8188eu,
    WiFiCardTypeAtheros9khtc,
    WiFiCardTypeAtheros9k,
    WiFiCardTypeRalink,
    WiFiCardTypeIntel,
    WiFiCardTypeBroadcom,
} WiFiCardType;


typedef enum WiFiHotspotType {
    WiFiHotspotTypeNone=0,
    WiFiHotspotTypeInternal2GBand,
    WiFiHotspotTypeInternal5GBand,
    WiFiHotspotTypeInternalDualBand,
    WiFiHotspotTypeExternal,
} WiFiHotspotType;

typedef enum WifiUseFor{
    WifiUseForUnknown=0, // Not sure what to use this wifi card for ;)
    WifiUseForMonitorMode, //Use for wifibroadcast, aka set to monitor mode
    WifiUseForHotspot, //Use for hotspot, aka start a wifi hotspot with it
}WifiUseFor;


struct WiFiCard {
    WiFiCardType type=WiFiCardTypeUnknown;
    std::string name;
    std::string vendor;
    std::string mac;
    bool supports_5ghz= false;
    bool supports_2ghz= false;
    bool supports_injection= false;
    bool supports_hotspot= false;
    bool supports_rts= false;
    // deterministic info and capabilities part end.
    WifiUseFor use_for=WifiUseForUnknown; // This one needs to be set for the card to then be used for something.
    // Otherwise, it is not used for anything
    // ? Dynamically changed settings ?
    std::string frequency;
    std::string txpower;
    std::string wifi_client_ap_name;
    std::string wifi_client_password;
    std::string hotspot_channel;
    std::string hotspot_password;
    std::string hotspot_band;
};

inline std::string wifi_card_type_to_string(const WiFiCardType& card_type) {
    switch (card_type) {
        case WiFiCardTypeAtheros9k: {
            return "ath9k";
        }
        case WiFiCardTypeAtheros9khtc: {
            return "ath9k_htc";
        }
        case WiFiCardTypeRealtek8812au: {
            return "88xxau";
        }
        case WiFiCardTypeRealtek88x2bu: {
            return "88x2bu";
        }
        case WiFiCardTypeRealtek8188eu: {
            return "8188eu";
        }
        case WiFiCardTypeRalink: {
            return "rt2800usb";
        }
        case WiFiCardTypeIntel: {
            return "iwlwifi";
        }
        case WiFiCardTypeBroadcom: {
            return "brcmfmac";
        }
        default: {
            return "unknown";
        }
    }
}


inline WiFiCardType string_to_wifi_card_type(const std::string& driver_name) {
    if (to_uppercase(driver_name).find(to_uppercase("ath9k_htc")) != std::string::npos) {
        return WiFiCardTypeAtheros9khtc;
    } else if (to_uppercase(driver_name).find(to_uppercase("ath9k")) != std::string::npos) {
        return WiFiCardTypeAtheros9k;
    } else if (to_uppercase(driver_name).find(to_uppercase("rt2800usb")) != std::string::npos) {
        return WiFiCardTypeRalink;
    } else if (to_uppercase(driver_name).find(to_uppercase("iwlwifi")) != std::string::npos) {
        return WiFiCardTypeIntel;
    } else if (to_uppercase(driver_name).find(to_uppercase("brcmfmac")) != std::string::npos) {
        return WiFiCardTypeBroadcom;
    } else if (to_uppercase(driver_name).find(to_uppercase("88xxau")) != std::string::npos) {
        return WiFiCardTypeRealtek8812au;
    } else if (to_uppercase(driver_name).find(to_uppercase("8812au")) != std::string::npos) {
        return WiFiCardTypeRealtek8812au;
    } else if (to_uppercase(driver_name).find(to_uppercase("88x2bu")) != std::string::npos) {
        return WiFiCardTypeRealtek88x2bu;
    } else if (to_uppercase(driver_name).find(to_uppercase("8188eu")) != std::string::npos) {
        return WiFiCardTypeRealtek8188eu;
    }

    return WiFiCardTypeUnknown;
}


inline std::string wifi_hotspot_type_to_string(const WiFiHotspotType& wifi_hotspot_type) {
    switch (wifi_hotspot_type) {
        case WiFiHotspotTypeInternal2GBand: {
            return "internal2g";
        }
        case WiFiHotspotTypeInternal5GBand: {
            return "internal5g";
        }
        case WiFiHotspotTypeInternalDualBand: {
            return "internaldualband";
        }
        case WiFiHotspotTypeExternal: {
            return "external";
        }
        default: {
            return "none";
        }
    }
}


inline WiFiHotspotType string_to_wifi_hotspot_type(const std::string& hotspot_type) {
    if (to_uppercase(hotspot_type).find(to_uppercase("internal2g")) != std::string::npos) {
        return WiFiHotspotTypeInternal2GBand;
    } else if (to_uppercase(hotspot_type).find(to_uppercase("internal5g")) != std::string::npos) {
        return WiFiHotspotTypeInternal5GBand;
    } else if (to_uppercase(hotspot_type).find(to_uppercase("internaldualband")) != std::string::npos) {
        return WiFiHotspotTypeInternalDualBand;
    } else if (to_uppercase(hotspot_type).find(to_uppercase("external")) != std::string::npos) {
        return WiFiHotspotTypeExternal;
    }

    return WiFiHotspotTypeNone;
}

static nlohmann::json wificard_to_json(const WiFiCard& p) {
    auto j=nlohmann::json{ {"type", p.type},
                           {"name", p.name},
                           {"vendor", p.vendor},
                           {"mac", p.mac},
                           {"supports_5ghz", p.supports_5ghz},
                           {"supports_2ghz", p.supports_2ghz},
                           {"supports_injection", p.supports_injection},
                           {"supports_hotspot", p.supports_hotspot},
                           {"supports_rts", p.supports_rts},
                           {"use_for", p.use_for},
    };
    return j;
}

static WiFiCard wificard_from_json(const nlohmann::json& j) {
    WiFiCard p;
    j.at("type").get_to(p.type);
    j.at("name").get_to(p.name);
    j.at("vendor").get_to(p.vendor);
    j.at("mac").get_to(p.mac);
    j.at("supports_5ghz").get_to(p.supports_5ghz);
    j.at("supports_2ghz").get_to(p.supports_2ghz);
    j.at("supports_injection").get_to(p.supports_injection);
    j.at("supports_hotspot").get_to(p.supports_hotspot);
    j.at("supports_rts").get_to(p.supports_rts);
    j.at("use_for").get_to(p.use_for);
    return p;
}

static nlohmann::json wificards_to_json(const std::vector<WiFiCard>& cards){
    nlohmann::json j;
    auto wifi_cards_json = nlohmann::json::array();
    for (auto &_card : cards) {
        auto cardJson = wificard_to_json(_card);
        wifi_cards_json.push_back(cardJson);
    }
    j["cards"] = wifi_cards_json;
    return j;
}

static constexpr auto WIFI_MANIFEST_FILENAME="/tmp/wifi_manifest";

static void write_wificards_manifest(const std::vector<WiFiCard>& cards){
    auto manifest= wificards_to_json(cards);
    std::ofstream _t(WIFI_MANIFEST_FILENAME);
    _t << manifest.dump(4);
    _t.close();
}

static std::vector<WiFiCard> wificards_from_manifest(){
    std::vector<WiFiCard> ret;
    try {
        std::ifstream f(WIFI_MANIFEST_FILENAME);
        nlohmann::json j;
        f >> j;
        for (const auto& _card : j["cards"]) {
            WiFiCard card= wificard_from_json(_card);
            ret.push_back(card);
        }
    } catch (std::exception &ex) {
        ohd_log(STATUS_LEVEL_EMERGENCY, "WiFi manifest processing failed");
        std::cerr << "WiFi::process_manifest: " << ex.what() << std::endl;
        return ret;
    }
    return ret;
}

#endif

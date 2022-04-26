#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

#include <systemd/sd-daemon.h>

#include "sodium.h"

#include "openhd-settings.hpp"
#include "openhd-log.hpp"

#include "json.hpp"

/*
 *
 * These are intentionally static, they need to match across different version of OpenHD without coordinating 
 * anything. We will transition to using plain keypairs once QOpenHD has an initial setup system.
 *
 */
unsigned char air_salt[crypto_pwhash_argon2i_SALTBYTES] = { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 };
unsigned char ground_salt[crypto_pwhash_argon2i_SALTBYTES] = { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 };


int main(int argc, char *argv[]) {


    try {
        bool is_air = false;
        std::string unit_id;

        try {
            std::ifstream f("/tmp/profile_manifest");
            nlohmann::json j;
            f >> j;

            is_air = j["is-air"];
            unit_id = j["unit-id"];
        } catch (std::exception &ex) {
            // don't do anything, but send an error message to the user through the status service
            ohd_log(STATUS_LEVEL_EMERGENCY, "Profile manifest processing failed");
            std::cerr << "EX: " << ex.what() << std::endl;
        }

        std::vector<std::map<std::string, std::string> > settings;

        try {
            std::string settings_path = find_settings_path(is_air, unit_id);
            std::cerr << "settings_path: " << settings_path << std::endl;
            std::string settings_file = settings_path + "/camera.conf";
            std::cerr << "settings_file: " << settings_file << std::endl;
            settings = read_config(settings_file);
        } catch (std::exception &ex) {
            std::cerr << "Camera settings load error: " << ex.what() << std::endl;
        }

        std::string openhd_password;

        for (auto _s : settings) {
            if (_s.count("openhd_password")) {
                openhd_password = _s["openhd_password"];
                boost::trim_right(openhd_password);
            } else {
                openhd_password = "OPENHDLINK";
            }
        }


        int res;
        unsigned char air_key[crypto_box_SEEDBYTES];
        unsigned char ground_key[crypto_box_SEEDBYTES];
        
        res = crypto_pwhash(ground_key,
                            crypto_box_SEEDBYTES,
                            openhd_password.c_str(),
                            openhd_password.length(),
                            ground_salt,
                            3,
                            crypto_pwhash_argon2i_MEMLIMIT_INTERACTIVE,
                            crypto_pwhash_ALG_ARGON2I13);

        if (res < 0) {
            throw std::runtime_error("Ground key generation failed");
        }

        res = crypto_pwhash(air_key,
                            crypto_box_SEEDBYTES,
                            openhd_password.c_str(),
                            openhd_password.length(),
                            air_salt,
                            3,
                            crypto_pwhash_argon2i_MEMLIMIT_INTERACTIVE,
                            crypto_pwhash_ALG_ARGON2I13);

        if (res < 0) {
            throw std::runtime_error("Air key generation failed");
        }

        unsigned char air_pub[crypto_box_PUBLICKEYBYTES];
        unsigned char air_sec[crypto_box_SECRETKEYBYTES];

        unsigned char ground_pub[crypto_box_PUBLICKEYBYTES];
        unsigned char ground_sec[crypto_box_SECRETKEYBYTES];


        res = crypto_box_seed_keypair(air_pub,
                                      air_sec,
                                      air_key);

        if (res < 0) {
            throw std::runtime_error("Air keypair generation failed");
        }


        res = crypto_box_seed_keypair(ground_pub,
                                      ground_sec,
                                      ground_key);

        if (res < 0) {
            throw std::runtime_error("Ground keypair generation failed");
        }

        std::ofstream _d("/tmp/tx.key", std::ios::binary | std::ios::out);
        _d.write(reinterpret_cast<const char*>(air_sec), sizeof(air_sec));
        _d.write(reinterpret_cast<const char*>(ground_pub), sizeof(ground_pub));
        _d.close();

        std::ofstream _u("/tmp/rx.key", std::ios::binary | std::ios::out);
        _u.write(reinterpret_cast<const char*>(ground_sec), sizeof(ground_sec));
        _u.write(reinterpret_cast<const char*>(air_pub), sizeof(air_pub));
        _u.close();

    } catch (std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        exit(1);
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        exit(1);
    }

    sd_notify(0, "READY=1");
    
    return 0;
}

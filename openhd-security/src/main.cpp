#include <fstream>

#include <iostream>
#include <iterator>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/regex.hpp>

#include <exception>

#include "sodium.h"


/*
 *
 * These are intentionally static, they need to match across different version of OpenHD without coordinating 
 * anything. We will transition to using plain keypairs once QOpenHD has an initial setup system.
 *
 */
unsigned char air_salt[crypto_pwhash_argon2i_SALTBYTES] = { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 };
unsigned char ground_salt[crypto_pwhash_argon2i_SALTBYTES] = { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 };


std::map<std::string, std::string> openhd_settings;



std::pair<std::string, std::string> parse_kv(std::string kv) {
    boost::smatch result;

    boost::regex r{ "^([\\w\\[\\]]+)\\s*=\\s*(.*)"};
    if (!boost::regex_match(kv, result, r)) {
        throw std::runtime_error("Ignoring invalid setting, check file for errors");
    }

    if (result.size() != 3) {
        throw std::runtime_error("Ignoring invalid setting, check file for errors");
    }

    return std::make_pair<std::string, std::string>(result[1], result[2]);
}


std::map<std::string, std::string> read_config(std::string path) {
    std::ifstream in(path);

    std::map<std::string, std::string> settings;

    std::string str;

    while (std::getline(in, str)) {
        if (str.size() > 0) {
            try {
                auto pair = parse_kv(str);
                settings.insert(pair);
            } catch (std::exception &ex) {
                /* 
                 * Ignore, likely a comment or user error in which case we will use a default.
                 */
            }
        }
    }

    return settings;
}


int main(int argc, char *argv[]) {


    try {
        openhd_settings = read_config("openhd-settings-1.txt");

        std::string openhd_password;

        {
            auto search = openhd_settings.find("OPENHD_PASSWORD");
            if (search != openhd_settings.end() && (search->second != "0")) {
                openhd_password = search->second;
                boost::trim_right(openhd_password);
            } else {
                std::cout << "OpenHD password missing, using default!!!" << std::endl;
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

    return 0;
}

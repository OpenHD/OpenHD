//
// Created by consti10 on 20.04.22.
//

#include "../src/HelperSources/Helper.hpp"
#include "../src/WBReceiver.h"
#include "../src/Encryption.hpp"

int main(int argc, char *const *argv) {

    WBReceiver wbReceiver(ROptions{}, nullptr);
    Encryptor encryptor{std::nullopt};
    return 0;
}
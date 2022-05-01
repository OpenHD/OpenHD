//
// Created by consti10 on 26.04.22.
// Test the c++-implementation for the reboot / shutdown commands
// Probably needs to be run with sudo
//

#include "../src/ohd_telemetry/RebootUtil.hpp"

int main() {
    RebootUtil::handlePowerCommand(false);
}
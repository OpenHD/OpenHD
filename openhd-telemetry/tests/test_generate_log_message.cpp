//
// Created by consti10 on 30.04.22.
// With telemetry service running, generate a log message that most likely will show up somewhere (lossy)
//

#include "openhd-log.hpp"

int main() {
    ohd_log(STATUS_LEVEL_EMERGENCY,"Test log message\n");
}
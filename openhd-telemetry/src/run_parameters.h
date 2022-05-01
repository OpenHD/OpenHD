//
// Created by consti10 on 24.04.22.
// All the parameters required to start this service.
// NOTE: Please leave these options as non-changing options,
// e.g. anything that needs to change dynamically at run time should not make it into here.
//

#ifndef XMAVLINKSERVICE_RUN_PARAMETERS_H
#define XMAVLINKSERVICE_RUN_PARAMETERS_H

enum Platform{
    RPI,JETSON,RV1126
};

struct OHDTelemetryServiceOptions{
    bool air=false; // true if the service is running on the air pi, false otherwise
    // Some stuff might be platform-dependent, for example how to read the current CPU frequency.
    // However, please be carefully here - Ideally we want as less platform dependency here as possible.
    Platform platform;
};

#endif //XMAVLINKSERVICE_RUN_PARAMETERS_H

//
// Created by consti10 on 27.11.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_DIRTY_IRRECOVERABLE_ERROR_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_DIRTY_IRRECOVERABLE_ERROR_H_

// TODO
// dirty
// OpenHD is written under the assumption that once discovered hardware works and does not disconnect /
// their linux driver(s) doesn't crash.
// Most common camera driver error(s) are handled gracefully, e.g. restarting the pipeline seems to fix most issues
// with the rpi camera drivers.
// For the wifi cards, it is a bit more complicated though.
// Here are the 2 most common issues - they are always fixable by using proper hardware,
// but we have the following checks in place anyways to account for the common mistake of improper wiring.
// 1) completely disconnecting and then reconnecting the wifi card
// -> can be fixed semi-gracefully by just crashing openhd, then letting the service re-start openhd
// 2) crashed / messed up wifi (driver ?!)
// -> requires a complete re-boot of linux

#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_DIRTY_IRRECOVERABLE_ERROR_H_

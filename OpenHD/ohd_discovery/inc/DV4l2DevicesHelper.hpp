//
// Created by consti10 on 16.05.22.
//

#ifndef OPENHD_DV4L2DEVICESHELPER_HPP
#define OPENHD_DV4L2DEVICESHELPER_HPP

#include <linux/videodev2.h>
#include <libv4l2.h>

/**
 * Try and break out some of the stuff from stephen.
 * Even though it mght not be re-used in multiple places, it makes the code more readable in my opinion.
 */
namespace DV4l2DevicesHelper{

    /**
     * For each format
     * enumerate the suported framesizes
     *      enumerate the supported frameintervals.
     */
    static void enumarateSupportedFormats(){

    }
}
#endif //OPENHD_DV4L2DEVICESHELPER_HPP

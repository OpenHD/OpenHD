//
// Created by consti10 on 11.05.22.
//

#ifndef OPENHD_DUMMYGSTREAMERSTREAM_H
#define OPENHD_DUMMYGSTREAMERSTREAM_H

#include "camerastream.h"
#include <gst/gst.h>

/**
 * This is an implementation of CameraStream that emulates a camera using gstreamer.
 * Uses videotestsrc and x264enc internally.
 */
class DummyGstreamerStream : public CameraStream{
public:
    DummyGstreamerStream(PlatformType platform,
                         Camera &camera,
                         uint16_t video_udp_port);
    void setup()override;
    void start()override;
    void stop()override;
private:
    GstElement * gst_pipeline = nullptr;
};


#endif //OPENHD_DUMMYGSTREAMERSTREAM_H

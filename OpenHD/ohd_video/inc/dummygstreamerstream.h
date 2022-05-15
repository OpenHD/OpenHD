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
    explicit DummyGstreamerStream(PlatformType platform,
                         Camera &camera,
                         uint16_t video_udp_port);
    void setup()override;
    void start()override;
    void stop()override;
    std::string debug()override;
private:
    bool supports_bitrate() override;
    void set_bitrate(int bitrate) override;
    // not supported by every encoder, some USB cameras can do it but only with custom commands
    bool supports_cbr() override;
    void set_cbr(bool enable) override;
    // expected to be widthXheight@fps format
    VideoFormat get_format() override;
    void set_format(VideoFormat videoFormat) override;
private:
    GstElement * gst_pipeline = nullptr;
};


#endif //OPENHD_DUMMYGSTREAMERSTREAM_H

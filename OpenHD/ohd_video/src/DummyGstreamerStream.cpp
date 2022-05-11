//
// Created by consti10 on 11.05.22.
//

#include "DummyGstreamerStream.h"

#include <sstream>

#include <gst/gst.h>
#include <fmt/core.h>

DummyGstreamerStream::DummyGstreamerStream(PlatformType platform, Camera &camera, uint16_t video_udp_port)
        : CameraStream(platform, camera, video_udp_port) {
    std::cout<<"DummyGstreamerStream::DummyGstreamerStream\n";
}

void DummyGstreamerStream::setup() {
    GError* error = nullptr;
    if (!gst_init_check(nullptr, nullptr, &error)) {
        std::cerr << "gst_init_check() failed: " << error->message << std::endl;
        g_error_free(error);

        throw std::runtime_error("GStreamer initialization failed");
    }
    std::stringstream pipeline;


    pipeline<<"videotestsrc num-buffers=1000 !";
    pipeline<<"x264enc name=encodectrl bitrate={} tune=zerolatency ! ";
    pipeline << "h264parse config-interval=-1 ! ";
    pipeline << "rtph264pay mtu=1024 ! ";
    pipeline<< fmt::format("udpsink host=127.0.0.1 port={} t. ! ", m_video_udp_port);

    gst_pipeline = gst_parse_launch(pipeline.str().c_str(), &error);
    if (error) {
        std::cerr << "Failed to create pipeline: " << error->message << std::endl;
        return;
    }
}

void DummyGstreamerStream::start() {
    std::cerr<<"DummyGstreamerStream::start()\n";
    gst_element_set_state(gst_pipeline, GST_STATE_PLAYING);
    GstState state;
    GstState pending;
    auto returnValue = gst_element_get_state(gst_pipeline, &state ,&pending, 1000000000);
    std::cerr << "Gst state:" << returnValue << "." << state << "."<< pending << "." << std::endl;
}

void DummyGstreamerStream::stop() {
    std::cerr<<"DummyGstreamerStream::stop()\n";
    gst_element_set_state(gst_pipeline, GST_STATE_PAUSED);
}

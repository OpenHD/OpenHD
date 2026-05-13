#include "../inc/ai_pipeline_helper.hpp"
#include "openhd_spdlog.h"
#include "openhd_spdlog_include.h"
#include <vector>

#include <opencv2/opencv.hpp>

namespace OHDAiHelper {

// Pad probe callback
static GstPadProbeReturn osd_draw_probe_callback(GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
    GstBuffer *buffer = gst_pad_probe_info_get_buffer(info);
    if (!buffer) {
        return GST_PAD_PROBE_OK;
    }

    // Make the buffer writable if it isn't already
    buffer = gst_buffer_make_writable(buffer);
    if (!buffer) {
        return GST_PAD_PROBE_OK;
    }

    // Get width and height from the pad's caps
    GstCaps *caps = gst_pad_get_current_caps(pad);
    if (!caps) {
        return GST_PAD_PROBE_OK;
    }

    GstStructure *structure = gst_caps_get_structure(caps, 0);
    int width = 1920, height = 1080; // defaults
    gst_structure_get_int(structure, "width", &width);
    gst_structure_get_int(structure, "height", &height);
    gst_caps_unref(caps);

    GstMapInfo map;
    if (gst_buffer_map(buffer, &map, GST_MAP_READWRITE)) {
        // Wrap the raw buffer in an OpenCV Mat (BGR format)
        cv::Mat frame(height, width, CV_8UC3, (char*)map.data);

        // --- AI Object Detection & OSD Logic Here ---
        // For demonstration, we draw a simulated bounding box and text
        cv::rectangle(frame, cv::Point(width / 4, height / 4), cv::Point(width * 3 / 4, height * 3 / 4), cv::Scalar(0, 255, 0), 3);
        cv::putText(frame, "AI Object Detected: Drone (98%)", cv::Point(width / 4, height / 4 - 10),
                    cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(0, 255, 0), 2);

        // Add a timestamp or frame counter OSD
        static int frame_count = 0;
        frame_count++;
        cv::putText(frame, "OpenHD AI OSD Active - Frame: " + std::to_string(frame_count),
                    cv::Point(30, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 255), 2);
        // --------------------------------------------

        gst_buffer_unmap(buffer, &map);
    }

    return GST_PAD_PROBE_OK;
}

std::string create_ai_pipeline_if_needed(int camera_index, const CameraSettings& settings, std::string pipeline) {
    // We only want to perform object detection on the secondary camera (index 1)
    if (camera_index != 1) {
        return pipeline;
    }

    auto console = openhd::log::get_default();
    console->info("Secondary camera (index 1) detected. Modifying pipeline for Pad Probe.");

    // We inject "videoconvert ! video/x-raw,format=BGR ! identity name=ai_osd_filter ! videoconvert !"
    // just before the encoder.
    std::string intercept = "videoconvert ! video/x-raw,format=BGR ! identity name=ai_osd_filter ! videoconvert ! ";

    std::vector<std::string> encoders = {
        "v4l2h264enc", "x264enc", "x265enc", "mpph264enc", "mpph265enc",
        "omxh264enc", "nvv4l2h264enc", "qtic2venc", "openh264enc"
    };

    bool injected = false;
    for (const auto& enc : encoders) {
        size_t pos = pipeline.find(enc);
        if (pos != std::string::npos) {
            pipeline.insert(pos, intercept);
            injected = true;
            break;
        }
    }

    if (!injected) {
        console->warn("Could not find a known encoder to inject AI OSD! Pipeline might be fully HW-accelerated like rpicamsrc.");
    } else {
        console->info("Successfully injected AI OSD filter into the pipeline string.");
    }

    return pipeline;
}

void attach_ai_osd_probe(GstElement* pipeline, int camera_index) {
    if (camera_index != 1 || !pipeline) {
        return;
    }

    auto console = openhd::log::get_default();

    // Find our identity element
    GstElement *identity_filter = gst_bin_get_by_name(GST_BIN(pipeline), "ai_osd_filter");
    if (!identity_filter) {
        console->warn("Could not find 'ai_osd_filter' in the pipeline! Cannot attach OSD probe.");
        return;
    }

    GstPad *srcpad = gst_element_get_static_pad(identity_filter, "src");
    if (!srcpad) {
        console->error("Failed to get src pad from ai_osd_filter!");
        gst_object_unref(identity_filter);
        return;
    }

    // Add the probe
    gst_pad_add_probe(srcpad, GST_PAD_PROBE_TYPE_BUFFER,
                      (GstPadProbeCallback)osd_draw_probe_callback,
                      nullptr, nullptr);

    console->info("Successfully attached AI OSD Pad Probe!");

    gst_object_unref(srcpad);
    gst_object_unref(identity_filter);
}

}  // namespace OHDAiHelper

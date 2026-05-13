#ifndef OPENHD_AI_PIPELINE_HELPER_H
#define OPENHD_AI_PIPELINE_HELPER_H

#include <string>
#include <gst/gst.h>
#include "camera_settings.hpp"

namespace OHDAiHelper {

/**
 * @brief Checks if AI processing is needed for the given camera and modifies the pipeline.
 * @param camera_index The index of the camera.
 * @param settings The settings for the camera.
 * @param pipeline The existing GStreamer pipeline string.
 * @return The (potentially modified) GStreamer pipeline string.
 */
std::string create_ai_pipeline_if_needed(int camera_index, const CameraSettings& settings, std::string pipeline);

/**
 * @brief Attaches a pad probe to intercept frames for OSD/AI processing.
 * @param pipeline The fully constructed GStreamer pipeline.
 * @param camera_index The index of the camera.
 */
void attach_ai_osd_probe(GstElement* pipeline, int camera_index);

}  // namespace OHDAiHelper

#endif  // OPENHD_AI_PIPELINE_HELPER_H

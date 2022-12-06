//
// Created by consti10 on 06.12.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_VIDEO_FRAME_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_VIDEO_FRAME_H_

namespace openhd{

// R.n this is the best name i can come up with
// This is not required to be exactly one frame, but should be
// already packetized into rtp fragments
struct FragmentedVideoFrame{
  std::vector<std::shared_ptr<std::vector<uint8_t>>> frame_fragments;
};

}
#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_VIDEO_FRAME_H_

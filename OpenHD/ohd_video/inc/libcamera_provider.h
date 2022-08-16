//
// Created by buldo on 15.08.22.
//

#ifndef OPENHD_LIBCAMERA_PROVIDER_H
#define OPENHD_LIBCAMERA_PROVIDER_H

#ifdef LIBCAMERA_PRESENT
#include <libcamera/libcamera.h>
#endif
#include "openhd-camera.hpp"


class LibcameraProvider {
 public:

  constexpr bool is_libcamera_available() {
    return libcamera_available_;
  }

  std::vector<Camera> get_cameras();

 private:
#ifdef LIBCAMERA_PRESENT
  const bool libcamera_available_ = true;
#else
  const bool libcamera_available_ = false;
#endif
};

#endif  // OPENHD_LIBCAMERA_PROVIDER_H

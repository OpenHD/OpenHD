# bin/bash

mkdir build_usb_cam_disabled
cd build_usb_cam_disabled

cmake .. -DENABLE_USB_CAMERAS=false
make -j4
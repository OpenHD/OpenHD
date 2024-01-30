//
// Created by consti10 on 30.01.24.
//

#ifndef OPENHD_USB_THERMAL_CAM_HELPER_H
#define OPENHD_USB_THERMAL_CAM_HELPER_H


namespace openhd{

static bool is_valid_infiray_custom_control_zoom_absolute_value(int value){
    return value>= 34817 &&  value<=34823;
}

// Apply the setting and hope it works ...
void set_infiray_custom_control_zoom_absolute_async(int value);

}


#endif //OPENHD_USB_THERMAL_CAM_HELPER_H

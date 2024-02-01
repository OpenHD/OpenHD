//
// Created by consti10 on 31.05.22.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_MAVLINKCOMPONENT_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_MAVLINKCOMPONENT_H_

class MavlinkComponent {
 public:
  /**
   * Process a mavlink message.
   * @return true when the message can be handled by this component, false
   * otherwise.
   */
  virtual bool processMavlinkMessage();

 private:
  const int component_id = 0;
};
#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_MAVLINKCOMPONENT_H_

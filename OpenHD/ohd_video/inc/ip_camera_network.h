#ifndef OPENHD_IP_CAMERA_NETWORK_H
#define OPENHD_IP_CAMERA_NETWORK_H

#include <string>

// Ensure a user-configured fixed-address camera is reachable. When its /24 is
// not already present, this adds a secondary address to an existing Ethernet
// interface without replacing the primary address or NetworkManager profile.
bool ensure_ip_camera_route(const std::string& camera_address);

#endif  // OPENHD_IP_CAMERA_NETWORK_H

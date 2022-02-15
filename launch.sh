#!/usr/bin/env bash

systemctl disable osd
systemctl disable openhd_system
systemctl disable openhd_security
systemctl disable openhd_interface
systemctl disable openhd_video
systemctl disable openhd_power
systemctl disable openhd_status
systemctl disable openhd_telemetry@microservice
systemctl disable openhd_telemetry@telemetry
systemctl disable openhd_settings_m

systemctl stop NetworkManager.service \
systemctl stop openhd_system \
systemctl stop openhd_security \
systemctl stop openhd_interface \
systemctl stop openhd_video \
systemctl stop openhd_power \
systemctl stop openhd_status \
systemctl stop openhd_telemetry@microservice \
systemctl stop openhd_telemetry@telemetry \
systemctl stop openhd_settings_m

systemctl start openhd_system
systemctl start openhd_security
systemctl start openhd_interface
systemctl start openhd_video
systemctl start openhd_power
systemctl start openhd_status
systemctl start openhd_telemetry@microservice
systemctl start openhd_telemetry@telemetry
systemctl start openhd_settings_m


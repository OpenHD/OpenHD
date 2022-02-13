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

systemctl stop NetworkManager.service

systemctl start openhd_system
systemctl start openhd_security
systemctl start openhd_interface
systemctl start openhd_video
systemctl start openhd_power
systemctl start openhd_status
systemctl start openhd_telemetry@microservice
systemctl start openhd_telemetry@telemetry


This library serves the following purposes:
1) Hold code that is used commonly throughout openhd 
2) Build and publicly expose spdlog and nlohmann::json to openhd (we use them both
   throughout the project)
3) Since ohd_interface, ohd_telemetry and ohd_video are allowed to depend on ohd_common
    but not on each other, glue them together in case they need to talk to each other
   (for example, when wb_link from ohd_interface wants to tell a camera in ohd_video to change the bitrate)

It is also a bit of a dump for things that don't really have their own submodule -
please take care to not pollute ohd_common with weird dependencies though.

The code for all commands that can be handled by this service itself (commands without side effects)
should be placed here. As well as code generating OHD fire and forget telemetry messages (like CPU Usage).

Example for handling:
reboot / shutdown commands

Example for generating:
Fire-and-forget telemetry message(s) containing (for example):
CPU usage, current in ampere (from sensor or pi itself or ...)

### Channel analysis profiles

`MAV_CMD_OPENHD_INITIATE_CHANNEL_ANALYZE` (11201) uses `param1` for the
channel selection and `param2` for the observation profile: `0` Quick, `1`
Medium, `2` Detailed. Non-integral, non-finite, and out-of-range values are
rejected. Zero selects Quick for clients that omit the profile.
The QOpenHD selector defaults to Medium.

Profiles change observation time, never the selected channel list. Devourer
uses ten CCX readings for Quick and twenty for Medium/Detailed with 20 ms spacing; packet counting
covers the same observation. Detailed visits every channel twice, averages valid per-channel observations,
and normalizes the averaged packet counts in QOpenHD. Legacy primary-radio
observation is 8 seconds for Quick and 16 seconds per visit for Medium/Detailed.
Nexmon uses three rounds for Quick/Medium and six for Detailed, with 2200 ms
Quick or 4400 ms Medium/Detailed observations. Retune and USB overhead are additional. ESP background
results remain cached measurements and are independent of this profile.

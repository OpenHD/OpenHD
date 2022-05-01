
The code for all commands that can be handled by this service itself (commands without side effects)
should be placed here. As well as code generating OHD fire and forget telemetry messages (like CPU Usage).

Example for handling:
reboot / shutdown commands

Example for generating:
Fire-and-forget telemetry message(s) containing (for example):
CPU usage, current in ampere (from sensor or pi itself or ...)
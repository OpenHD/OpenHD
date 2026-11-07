# Luckfox Pico userspace radio startup

The Pico policy uses synchronous Devourer USB register transfers and normal
scheduling for WiFiBroadcast receive/transmit workers. Other platforms retain
register pipelining and their existing scheduling defaults. This is an internal
host policy, not a flash-time setting or a change to WiFiBroadcast defaults.

On a Pico Pro RV1106 with a powered RTL8812EU/RTL8822E USB adapter (0bda:a81a),
pipelined initialization failed with LIBUSB_ERROR_IO and could leave the adapter
unresponsive until its power was removed. Disabling only real-time scheduling
still reproduced that initialization failure. Synchronous initialization alone
started the radio, but management access stopped when video transmission began.
Using synchronous initialization together with normal scheduling kept management
access responsive and restored ground-station video and bidirectional telemetry.
USB reset and reset-settling delays alone did not fix the full application.

Validation on 2026-11-07 included an ARM/uClibc application build, live operation
without diagnostic preload libraries, and automatic startup after reboot. The
native MIS5001 encoder reported about 58 fps at 1280x720; live radio statistics
showed receive traffic and roughly 1000 transmitted packets per second. The user
confirmed video and telemetry at the ground station. Other Pico board variants
require their own hardware validation.

Disabling register pipelining retains UsbTransport batch error accounting:
failed synchronous writes must still fail write_batch_end(). It does not alter
bulk packet transmission or reception.

# X21 USB video diagnostics on Windows

These opt-in bench tools preserve RTP datagrams over USB NCM TCP. They do not
configure the camera or install an SSH server. The camera driver and its MCU
firmware live in `OpenHD/openhd_x21b_kernel`; the matching image profile lives
in `OpenHD/openhd_x21b_base`.

Build `usb-video-relay.c` with the X21 SDK compiler and copy the executable to
`/ohd/x21-diagnostics/usb-video-relay` on the board. Make it executable. The
relay binds UDP 127.0.0.1:5600 and TCP 192.168.7.2:15600 when started by the viewer.
It requests a 4 MiB receive buffer using SO_RCVBUFFORCE as root, bypassing the
image's 208 KiB rmem_max cap, and discards stale queued packets on reconnect.
The printed SO_RCVBUF value includes Linux's doubled socket accounting.

Use an existing authorized SSH debug endpoint on the X21 at 192.168.7.2:2222.
Install its verified host key in a known-hosts file and supply your private key;
no credentials or host keys are included here. Windows needs Python, Paramiko
and its OpenSSH client. Run the following in separate terminals:

```powershell
python windows-usb-telemetry-tunnel.py --identity C:/keys/x21 --known-hosts C:/keys/x21-known-hosts
python windows-usb-ssh-view.py --identity C:/keys/x21 --known-hosts C:/keys/x21-known-hosts
```

Set QOpenHD's manual TCP telemetry address to 127.0.0.1, port 5760. Video arrives
on localhost UDP port 5600. An additional localhost UDP port 5602 copy is provided for
measurement. Each helper discovers the active UsbNcm adapter and its 192.168.7.x
address again after reconnection, including Windows interface renumbering.
`X21_USB_INTERFACE_INDEX` can explicitly select an adapter if necessary.

```powershell
python capture-rtp-bench.py capture720p120 --seconds 30 --port 5602
```

The probe saves Annex-B H.264 and JSON. Check continuity, observed duration,
packet gaps and RTP timestamps together. Active-burst FPS alone can look healthy
after a freeze. Measurements spanning an encoder/mode restart are not steady
FPS proof. Start after the new stream settles. H.264 decoding must begin at the
first SPS and end at a complete access unit; use a decoder to check corruption.
Configured FPS in camera telemetry is not measured capture/output FPS.

The TCP video relay is an unencrypted temporary diagnostic service on the USB
interface. It is not an RF transport or a benchmark of radio performance.
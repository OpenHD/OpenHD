# MAVLink UART Debugger

Interactive utility that reads MAVLink frames from a serial port using the OpenHD dialect and displays them in a curses based table similar to `htop`. The tool tracks the latest payload per message ID, keeps a running count, and can optionally store a textual log on disk.

## Building

```bash
cmake -S . -B build
cmake --build build
```

This will produce the `mavlink_uart_logger` executable inside the `build` directory.

## Usage

```bash
./mavlink_uart_logger --device /dev/ttyUSB0 --baud 115200 \
    --sysid 1 --compid 1 --target-sys 1 --target-comp 1 --output log.txt --raw
```

* `--output` is optional. When set, the decoded messages are appended to the given file.
* `--sysid`/`--compid` specify the IDs used for locally generated messages.
* `--target-sys`/`--target-comp` specify the destination of outgoing commands.
* `--transmit` enables a background mode that periodically emits random OpenHD MAVLink messages (which are also listed in the UI).
* `--loop` enables a loopback test mode that periodically sends MAVLink ping packets and tracks how many are received back on the
  same serial port, making it easy to validate a TX↔RX connection with a jumper.
* `--raw` shows the raw bytes read from the UART while the debugger is in listen mode, allowing troubleshooting of non-MAVLink traffic.

While running, the TUI updates each time a MAVLink message is received. Use the following shortcuts:

| Key | Action |
| --- | ------ |
| `h` | Send a MAVLink heartbeat |
| `p` | Send a MAVLink ping |
| `r` | Send a `MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN` command |
| `t` | Immediately send a random OpenHD telemetry message (or a loop-mode ping when `--loop` is active) |
| `q` | Quit the debugger |

The status line at the top reports the outcome of the most recent command and whether logging is enabled. When `--loop` is active,
an additional counter shows the number of ping packets sent, matched, lost and unexpected, giving a quick view of link health.
All modes, including `--loop`, automatically pick the first `/dev/serialX` device they can find (falling back to `/dev/ttySX`
when needed) and a baud rate of 115200 when those options are not specified explicitly.
When `--raw` is active, an additional panel displays the latest UART bytes in hexadecimal order whenever the tool is receiving.

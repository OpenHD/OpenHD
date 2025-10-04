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
    --sysid 1 --compid 1 --target-sys 1 --target-comp 1 --output log.txt
```

* `--output` is optional. When set, the decoded messages are appended to the given file.
* `--sysid`/`--compid` specify the IDs used for locally generated messages.
* `--target-sys`/`--target-comp` specify the destination of outgoing commands.

While running, the TUI updates each time a MAVLink message is received. Use the following shortcuts:

| Key | Action |
| --- | ------ |
| `h` | Send a MAVLink heartbeat |
| `p` | Send a MAVLink ping |
| `r` | Send a `MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN` command |
| `q` | Quit the debugger |

The status line at the top reports the outcome of the most recent command and whether logging is enabled.

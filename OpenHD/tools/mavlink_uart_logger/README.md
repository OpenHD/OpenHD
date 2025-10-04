# MAVLink UART Logger

Small standalone utility that reads MAVLink frames from a serial port using the OpenHD dialect and stores a textual log on disk.

## Building

```bash
cmake -S . -B build
cmake --build build
```

This will produce the `mavlink_uart_logger` executable inside the `build` directory.

## Usage

```bash
./mavlink_uart_logger --device /dev/ttyUSB0 --baud 115200 --output log.txt
```

The utility prints every decoded message to `stdout` and appends the same information to the provided text file. Press <kbd>Ctrl</kbd>+<kbd>C</kbd> to stop the logger.

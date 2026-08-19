# 07_l4_uart_config UART 配置示例

`l4_uart_config` 用于演示通过 `PRJ_CMD_SET_UART` 和 `PRJ_CMD_GET_UART` 设置、查询设备 UART 配置。

本示例使用 SDK 统一例程参数风格：`-a/-p/-i` 用于连接 daemon 和选择设备，UART id 由 `--get-uart` 或 `--set-uart` 指定。

## 常用命令

只查询 UART 1 的运行态配置：

```sh
./l4_uart_config -g 1
```

设置 UART 1 为 `115200 8N1`，写入 MiniDB：

```sh
./l4_uart_config -U 1 -b 115200 -d 8 -P none -T 1 -r 0
```

设置 UART 1 为 `9600 8N1`，并请求立即应用到运行态：

```sh
./l4_uart_config -U 1 -b 9600 -d 8 -P none -T 1 -A
```

通过 remote ioctl slot 0 查询 UART 1：

```sh
./l4_uart_config -s 0 -g 1
```

## 参数说明

公共参数：

| 参数 | 长参数 | 作用 | 默认值 |
| --- | --- | --- | --- |
| `-h` | `--help` | 打印帮助 | 无 |
| `-a <addr>` | `--addr <addr>` | daemon 地址 | `127.0.0.1` |
| `-p <port>` | `--port <port>` | daemon 端口 | `BB_PORT_DEFAULT` |
| `-i <index>` | `--index <index>` | 设备序号 | `0` |

查询参数：

| 短参数 | 长参数 | 作用 |
| --- | --- | --- |
| `-g <id>` | `--get-uart <id>` | 查询指定 UART 的 running 配置 |

设置参数：

| 短参数 | 长参数 | 作用 | 默认值 |
| --- | --- | --- | --- |
| `-U <id>` | `--set-uart <id>` | 设置指定 UART | 无 |
| `-b <rate>` | `--baudrate <rate>` | UART 波特率 | `115200` |
| `-d <5-8>` | `--data-bit <5-8>` | UART 数据位 | `8` |
| `-P <parity>` | `--parity <parity>` | UART 校验位，支持 `none/even/odd/0/1/2` | `none` |
| `-T <1-3>` | `--stop-bit <1-3>` | UART 停止位协议值，`1:1bit / 2:1.5bits / 3:2bits` | `1` |
| `-r <n>` | `--rx-buf-size <n>` | UART RX buffer 大小，`0` 表示设备默认值 | `0` |
| `-A` | `--apply` | 设置后请求立即应用到运行态 | 不执行 |

其它参数：

| 短参数 | 长参数 | 作用 |
| --- | --- | --- |
| `-s <slot>` | `--slot <slot>` | 通过 remote ioctl slot 执行命令 |

每次运行只允许一个主动作：`--get-uart` 或 `--set-uart` 二选一。设置动作不会自动查询，设置后如需确认，请再次执行 `--get-uart`。

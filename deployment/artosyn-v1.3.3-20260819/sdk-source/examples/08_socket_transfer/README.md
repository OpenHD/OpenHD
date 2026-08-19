# 08_l4_socket_transfer Socket 收发示例

`l4_socket_transfer` 用于演示打开设备的 socket port，并通过 `bb_socket_write()` 发送指定内容，或通过 `bb_socket_read()` 持续接收数据。

本示例只覆盖 socket API 的最小收发流程，不负责配对、链路配置、吞吐压测或文件发送。运行前请先启动 `l4_daemon`，并确认 AP/DEV 已完成连接。

普通 socket 与旧版协议兼容。加密 socket 要求客户端、daemon 和 8030 固件均支持新版加密协议，并且收发两端使用相同的加密模式和密钥。

## 常用命令

接收 socket port 1 上的数据，按 `Ctrl-C` 退出：

```sh
./l4_socket_transfer -s 0 -P 1 --recv
```

发送一次文本内容，发送完成后等待 1 秒关闭 socket：

```sh
./l4_socket_transfer -s 0 -P 1 --text "hello l4"
```

进入文本输入模式，每输入一行并按回车发送一次，按 `Ctrl-C` 关闭：

```sh
./l4_socket_transfer -s 0 -P 1 --text-input
```

发送一次十六进制字节，发送完成后等待 1 秒关闭 socket：

```sh
./l4_socket_transfer -s 0 -P 1 --hex "01 02 0a ff"
```

使用 AES128 加密发送文本（接收端需使用相同的模式和密钥）：

```sh
./l4_socket_transfer -s 0 -P 1 --encrypt-mode aes128 \
  --encrypt-key 0102030405060708090a0b0c0d0e0f10 \
  --text "hello l4"
```

进入十六进制输入模式，每输入一行并按回车发送一次，按 `Ctrl-C` 关闭：

```sh
./l4_socket_transfer -s 0 -P 1 --hex-input
```

## 参数说明

公共参数：

| 参数 | 长参数 | 作用 | 默认值 |
| --- | --- | --- | --- |
| `-h` | `--help` | 打印帮助 | 无 |
| `-a <addr>` | `--addr <addr>` | daemon 地址 | `127.0.0.1` |
| `-p <port>` | `--port <port>` | daemon 端口 | `BB_PORT_DEFAULT` |
| `-i <index>` | `--index <index>` | 设备序号 | `0` |
| `-s <slot>` | `--slot <slot>` | 目标 slot，DEV 侧 `0` 表示 AP | `0` |
| `-P <port>` | `--socket-port <port>` | socket 逻辑端口 | `1` |
| 无 | `--encrypt-mode <mode>` | 启用加密，可选 `default`、`aes128`、`aes256` | 不启用 |
| 无 | `--encrypt-key <hex>` | AES 密钥；AES128 为 32 个十六进制字符，AES256 为 64 个 | 无 |

主动作：

| 参数 | 长参数 | 作用 |
| --- | --- | --- |
| `-t <text>` | `--text <text>` | 发送文本字节一次，发送完成后等待 1 秒关闭 socket |
| `-T` | `--text-input` | 文本输入模式，持续读取用户输入的文本行并发送 |
| `-x <hex>` | `--hex <hex>` | 发送十六进制字节一次，发送完成后等待 1 秒关闭 socket |
| `-X` | `--hex-input` | 十六进制输入模式，持续读取用户输入的十六进制行并发送 |
| `-r` | `--recv` | 持续接收数据直到 `Ctrl-C` |

每次运行必须且只能指定一个主动作。`--hex` 和 `--hex-input` 要求每个字节使用两个十六进制字符，例如 `0a`，也可以写成 `01020aff` 或 `01:02:0a:ff`。输入模式期间空行会被跳过；文本输入模式按原始文本字节发送，十六进制输入模式按十六进制字节发送。

`default` 加密模式不使用 `--encrypt-key`；`aes128` 和 `aes256` 必须提供长度完全匹配的 key。示例不会打印密钥内容，也不会负责运行中动态更新加密参数。

如果命令行出现 `dquote>`，表示 shell 检测到双引号没有闭合，程序尚未启动。请补齐结尾双引号，或按 `Ctrl-C` 取消后重新输入，例如：

```sh
./l4_socket_transfer -s 1 -P 1 -t "test"
```

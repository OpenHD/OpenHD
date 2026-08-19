# 06_l4_minidb_config MiniDB 配置示例

`l4_minidb_config` 用于演示通过 `BB_SET_PRJ_DISPATCH` 和 `BB_GET_PRJ_DISPATCH` 读写设备 MiniDB 持久化配置。增加 `--remote` 后，查询、写入、重置和重启会通过 `BB_REMOTE_IOCTL_REQ` 在对端执行，默认 remote slot 为 0。

MiniDB 是持久化配置，不等同于当前运行态配置。写入后是否立即生效由设备侧决定，通常建议重启设备后再次查询确认。

## 常用命令

查询全部 MiniDB 信息：

```sh
./l4_minidb_config -A
```

查询 AP MAC：

```sh
./l4_minidb_config -m
```

查询 slot 0 MAC：

```sh
./l4_minidb_config -s
```

查询频点列表：

```sh
./l4_minidb_config -f
```

查询扩展功率配置：

```sh
./l4_minidb_config --get-pwr-ex
```

查询 UART2 波特率：

```sh
./l4_minidb_config -u
```

设置角色：

```sh
./l4_minidb_config -R ap
./l4_minidb_config -R dev
```

设置 AP MAC：

```sh
./l4_minidb_config -M 11:22:33:44
```

设置 slot MAC：

```sh
./l4_minidb_config -S 11:22:33:44
```

设置频段：

```sh
./l4_minidb_config -B auto
./l4_minidb_config -B 2g
./l4_minidb_config -B 5g
```

设置固定功率：

```sh
./l4_minidb_config -W 20
```

设置功率自适应区间：

```sh
./l4_minidb_config -W 10,27
```

设置频点列表：

```sh
./l4_minidb_config -F 2400,2411,2422
```

设置 UART2 波特率：

```sh
./l4_minidb_config -U 115200
```

重置 MiniDB：

```sh
./l4_minidb_config -D
```

设置成功后重启设备：

```sh
./l4_minidb_config -B auto -H
```

单独重启设备：

```sh
./l4_minidb_config -H
```

远程查询对端全部 MiniDB 信息：

```sh
./l4_minidb_config --remote -A
```

远程写入对端 MiniDB role：

```sh
./l4_minidb_config --remote -R ap
```

远程写入对端 slot MAC：

```sh
./l4_minidb_config --remote -S 1,11:22:33:44
```

远程重置对端 MiniDB：

```sh
./l4_minidb_config --remote -D
```

远程重启对端设备：

```sh
./l4_minidb_config --remote -H
```

远程操作指定 remote slot：

```sh
./l4_minidb_config --remote --remote-slot 1 -B auto -H
```

## 参数说明

公共参数：

| 参数 | 作用 | 默认值 |
| --- | --- | --- |
| `-a <addr>` | daemon 地址 | `127.0.0.1` |
| `-p <port>` | daemon 端口 | `BB_PORT_DEFAULT` |
| `-i <index>` | 设备序号 | `0` |
| `--remote` | 通过远程 ioctl 操作对端设备 | 本机操作 |
| `--remote-slot <slot>` | 指定 `--remote` 使用的 remote slot | `0` |
| `-h` | 打印帮助 | 无 |

查询参数：

| 短参数 | 长参数 | 作用 |
| --- | --- | --- |
| `-A` | `--get-all` | 查询 role、AP MAC、slot 0 MAC、band、power、power ex、freq list |
| `-r` | `--get-role` | 查询 role |
| `-m` | `--get-ap-mac` | 查询 AP MAC |
| `-s [slot]` | `--get-slot-mac[=slot]` | 查询 slot MAC，默认 slot 0 |
| `-b` | `--get-band` | 查询 band |
| `-w` | `--get-pwr` | 查询 power |
| 无 | `--get-pwr-ex` | 查询扩展功率配置 |
| `-f` | `--get-freq-list` | 查询频点列表 |
| `-u` | `--get-uart-baudrate` | 查询 UART2 波特率 |

设置参数：

| 短参数 | 长参数 | 作用 |
| --- | --- | --- |
| `-R <ap|dev|0|1>` | `--set-role <...>` | 设置 role |
| `-M <mac>` | `--set-ap-mac <mac>` | 设置 AP MAC |
| `-S <mac|slot,mac>` | `--set-slot-mac <mac|slot,mac>` | 设置 slot MAC，默认 slot 0 |
| `-B <auto|2g|5g|0x07>` | `--set-band <...>` | 设置 band bitmap |
| `-W <pwr|min,max>` | `--set-pwr <pwr|min,max>` | 设置固定功率或功率自适应区间 |
| `-F <mhz,...>` | `--set-freq-list <mhz,...>` | 设置频点列表，输入单位 MHz |
| `-U <baudrate>` | `--set-uart-baudrate <baudrate>` | 设置 UART2 波特率 |
| `-D` | `--reset` | 重置 MiniDB |

其它参数：

| 参数 | 作用 |
| --- | --- |
| `-H` / `--reboot` | 单独重启设备，或设置/重置成功后重启设备 |

## 输入格式

MAC 支持两种格式。当前 SDK 中 `BB_MAC_LEN = 4`，因此 AP MAC 和 slot MAC 都按 4 字节输入：

```text
11:22:33:44
11223344
```

功率设置约定：

| 输入 | 行为 |
| --- | --- |
| `-W 20` | 设置固定功率，并同步写入扩展功率的手动初始化和重置功率 |
| `-W 10,27` | 设置自适应功率区间，写入 `pwr_auto=1`、`pwr_min=10`、`pwr_max=27` |

固定功率和功率区间的合法范围都是 `10-27`。`-W` 只能选择一种格式：单个值表示固定功率，`min,max` 表示功率范围。

固定功率模式下，`-W 20` 会先通过 `PRJ_CMD_SET_PWR` 写入 `pwr_init=20`，再通过 `PRJ_CMD_SET_PWR_EX` 写入 `link_auto=0`、`auto_reset=0`、`manu_init=20`、`manu_reset=20`、`csma_offset=0`。自适应区间格式 `-W min,max` 只更新基础功率配置，不修改扩展功率配置。扩展功率可通过 `--get-pwr-ex` 查询；设备禁用或从未配置时显示 `disabled/unset`。

频点列表设置约定：

| 输入 | 行为 |
| --- | --- |
| `-F 2400,2411,2422` | 按 MHz 输入，写入协议时转换为 `2400000,2411000,2422000` KHz |

频点数量必须为 `1-32`，即不超过 `BB_CONFIG_MAX_CHAN_NUM`。当前只接受正整数 MHz，保留输入顺序，不自动排序或去重。

UART 波特率配置固定操作 UART2。`-U <baudrate>` 只修改 MiniDB 中 UART2 的波特率，保留已有数据位、校验位、停止位和 RX buffer 配置；如果 UART2 尚未配置，则使用 `8N1` 和默认 RX buffer 创建配置。

band bitmap 约定：

| 输入 | bitmap |
| --- | --- |
| `auto` | `0x07` |
| `2g` | `0x02` |
| `5g` | `0x04` |

每次运行只允许一个主动作，例如不能同时指定 `-A` 和 `-B auto`。`-W` 的固定功率和功率范围格式互斥。`-s` 和 `-S <mac>` 默认使用 MiniDB slot 0；非 0 MiniDB slot 可用 `-s 1`、`--get-slot-mac=1` 或 `-S 1,11:22:33:44`。`--remote` 只改变命令实际执行端，`--remote-slot` 只能和 `--remote` 一起使用；不新增远程短参数，因为 `-R` 已是 `--set-role`，`-S` 已是 `--set-slot-mac`。`-H/--reboot` 可以单独作为重启动作，也可以附加在设置或重置动作后，例如 `-F 2400,2411 -H`；带 `--remote` 时 reboot 也在对端执行。

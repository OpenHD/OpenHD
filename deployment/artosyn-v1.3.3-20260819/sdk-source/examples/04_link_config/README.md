# 04_link_config 图传链路配置示例

`l4_link_config` 用于设置图传链路的频段、信道、频宽、MCS、功率和帧结构配置。它和 `03_link_monitor` 配套使用：`03_link_monitor` 负责读取链路状态，`04_link_config` 负责下发链路配置。

这个示例当前实现十一个设置命令，均支持通过 `-R` 封装成 `BB_REMOTE_IOCTL_REQ` 远程设置对端。`BB_SET_FRAME_CHANGE` 只能在 AP 端执行；带 `-R` 时，程序会先远程读取对端状态，确认对端是 AP 且处于 `BB_MODE_SINGLE_USER` 后再下发。

| 命令字 | 作用 |
| --- | --- |
| `BB_SET_BAND_MODE` | 设置频段自动/手动模式 |
| `BB_SET_BAND` | 手动设置目标工作频段 |
| `BB_SET_CHAN_MODE` | 设置信道自动/手动模式 |
| `BB_SET_CHAN` | 手动设置目标信道索引 |
| `BB_SET_BANDWIDTH_MODE` | 设置频宽自动/手动模式 |
| `BB_SET_BANDWIDTH` | 手动设置目标频宽 |
| `BB_SET_MCS_MODE` | 设置 MCS 自动/手动模式 |
| `BB_SET_MCS` | 手动设置目标 MCS |
| `BB_SET_POWER_AUTO` | 设置功率自适应开关 |
| `BB_SET_POWER` | 设置固定发射功率 |
| `BB_SET_FRAME_CHANGE` | 运行中改变帧结构，仅 AP 端 SINGLE_USER 模式 |

## 常用命令

### 频段和信道都设置为自动

```sh
./l4_link_config -B 1 -C 1
```

这里 `-B 1` 表示频段自动，`-C 1` 表示信道自动。

### 手动设置目标频段

```sh
./l4_link_config -B 0 -b 2g
```

这里先把频段模式设置为手动，再把目标频段设置为 2G。

### 手动设置目标信道

```sh
./l4_link_config -C 0 -d rx -c 3
```

这里先把信道模式设置为手动，再把 RX 方向的目标信道索引设置为 `3`。

### 手动设置目标频宽

```sh
./l4_link_config -s 0 -W 0 -d rx -w 4
```

这里先把 slot 0 的频宽模式设置为手动，再把 RX 方向的目标频宽设置为 `BB_BW_20M`。

### 手动设置目标 MCS

```sh
./l4_link_config -s 0 -M 0 -m 6
```

这里先把 slot 0 的 MCS 模式设置为手动，再把目标 MCS 设置为 SDK 枚举值 `6`。

### 设置功率自适应

```sh
./l4_link_config -O 1
```

这里 `-O 1` 表示开启功率自适应，`-O 0` 表示关闭功率自适应。

### 手动设置固定功率

```sh
./l4_link_config -O 0 -P 20
```

这里先关闭功率自适应，再把默认物理用户的发射功率设置为 `20 dBm`。如果需要指定物理用户，可增加 `-u <user>`。

### 远程设置对端配置

```sh
./l4_link_config -R -s 0 -B 0 -b 2g -C 0 -d rx -c 3 -W 0 -w 4 -M 0 -m 6
```

`-R` 会把设置命令封装成 `BB_REMOTE_IOCTL_REQ`，通过 `-s <slot>` 指定的链路位置发给对端设备。远程模式支持 `-B`、`-b`、`-C`、`-c`、`-W`、`-w`、`-M`、`-m`、`-O`、`-P`、`-F`。其中 `-F` 只能让实际执行端为 AP；如果远程对端不是 AP，程序会报错退出。

远程设置对端固定功率示例：

```sh
./l4_link_config -R -s 0 -O 0 -P 20
```

### 设置帧结构切换

```sh
./l4_link_config -F 1
```

这里 `-F 1` 表示执行帧结构切换，`-F 0` 表示恢复原始帧结构。程序会先读取 `BB_GET_STATUS`，只有实际执行端是 AP 且当前模式是 `BB_MODE_SINGLE_USER` 时才会调用 `BB_SET_FRAME_CHANGE`。

### 同时设置频段、信道、频宽、MCS 和功率

```sh
./l4_link_config -B 0 -b 2g -C 0 -d rx -c 3 -s 0 -W 0 -w 4 -M 0 -m 6
```

程序会按固定顺序执行：频段模式、频段、信道模式、信道、频宽模式、频宽、MCS 模式、MCS、功率自适应、固定功率。如果指定 `-F`，会在功率设置后执行帧结构切换。一次命令行包含多个配置动作时，相邻两个配置命令之间会等待 200ms。

## 参数说明

| 参数 | 作用 |
| --- | --- |
| `-a <addr>` | daemon 地址，默认 `127.0.0.1` |
| `-p <port>` | daemon 端口，默认 `BB_PORT_DEFAULT` |
| `-i <index>` | 设备序号，默认 `0` |
| `-s <slot>` | slot 编号，默认 `0`；用于频宽和 MCS 配置，带 `-R` 时也是远程对端 slot |
| `-u <user>` | `BB_SET_POWER` 使用的物理用户；未指定时 AP 默认 `BB_USER_BR_CS`，DEV 默认 `BB_USER_0` |
| `-R` | 通过远程 ioctl 设置 `-s <slot>` 指定的对端；支持 `-B`、`-b`、`-C`、`-c`、`-W`、`-w`、`-M`、`-m`、`-O`、`-P`、`-F` |
| `-B <0|1>` | 调用 `BB_SET_BAND_MODE`，`1` 自动，`0` 手动 |
| `-b <band>` | 调用 `BB_SET_BAND`，支持 `1g`、`2g`、`5g` 或 `0`、`1`、`2` |
| `-C <0|1>` | 调用 `BB_SET_CHAN_MODE`，`1` 自动，`0` 手动 |
| `-c <index>` | 调用 `BB_SET_CHAN`，设置信道索引，范围 `0-255` |
| `-d <dir>` | `-c` 和 `-w` 使用的方向，支持 `tx`、`rx` 或 `0`、`1`，默认 `rx` |
| `-W <0|1>` | 调用 `BB_SET_BANDWIDTH_MODE`，`1` 自动，`0` 手动 |
| `-w <bandwidth>` | 调用 `BB_SET_BANDWIDTH`，范围 `0-5` |
| `-M <0|1>` | 调用 `BB_SET_MCS_MODE`，`1` 自动，`0` 手动 |
| `-m <mcs>` | 调用 `BB_SET_MCS`，范围 `0-24` |
| `-O <0|1>` | 调用 `BB_SET_POWER_AUTO`，`1` 开启功率自适应，`0` 关闭功率自适应 |
| `-P <power>` | 调用 `BB_SET_POWER`，固定发射功率范围 `0-31 dBm` |
| `-F <0|1>` | 调用 `BB_SET_FRAME_CHANGE`，`1` 执行帧结构切换，`0` 恢复原始帧结构，仅 AP 端且 `BB_MODE_SINGLE_USER` 支持；带 `-R` 时检查远端角色和模式 |
| `-h` | 打印帮助信息 |

如果没有指定任何设置动作，程序只打印帮助并返回错误，避免误操作。`-R` 远程模式会复用 `-s <slot>` 选择对端链路位置；对 `-F`，程序会额外检查实际执行端必须为 AP。

## 输出日志格式

每个成功执行的命令会先打印命令字标题，再打印参数详情，格式和 `03_link_monitor` 保持一致：

```text
[BB_SET_BAND_MODE]
auto_mode=0

[BB_SET_BAND]
target_band=5G(2)
```

任意一步 `bb_ioctl()` 返回非 0 时，程序会打印 `BB_SET_xxx failed, ret=<ret>`，停止后续设置，关闭连接并返回错误。一次执行多个配置动作时，只有前一个配置动作成功后才会在下一个配置动作前等待 200ms；带 `-R` 时同样生效，成功标题会显示 `remote slot=<slot>`。

## 参数和结构体字段的对应关系

### `BB_SET_BAND_MODE`

```c
bb_set_band_mode_t input;
input.auto_mode = auto_mode;
bb_ioctl(handle, BB_SET_BAND_MODE, &input, NULL);
```

| 参数 | 字段 | 含义 |
| --- | --- | --- |
| `-B 1` | `auto_mode = 1` | 频段自动 |
| `-B 0` | `auto_mode = 0` | 频段手动 |

### `BB_SET_BAND`

```c
bb_set_band_t input;
input.target_band = target_band;
bb_ioctl(handle, BB_SET_BAND, &input, NULL);
```

| 参数 | 字段值 | 含义 |
| --- | --- | --- |
| `-b 1g` 或 `-b 0` | `BB_BAND_1G` | 1G 频段 |
| `-b 2g` 或 `-b 1` | `BB_BAND_2G` | 2G 频段 |
| `-b 5g` 或 `-b 2` | `BB_BAND_5G` | 5G 频段 |

### `BB_SET_CHAN_MODE`

```c
bb_set_chan_mode_t input;
input.auto_mode = auto_mode;
bb_ioctl(handle, BB_SET_CHAN_MODE, &input, NULL);
```

| 参数 | 字段 | 含义 |
| --- | --- | --- |
| `-C 1` | `auto_mode = 1` | 信道自适应 |
| `-C 0` | `auto_mode = 0` | 信道手动 |

### `BB_SET_CHAN`

```c
bb_set_chan_t input;
input.chan_dir = chan_dir;
input.chan_index = chan_index;
bb_ioctl(handle, BB_SET_CHAN, &input, NULL);
```

| 参数 | 字段 | 含义 |
| --- | --- | --- |
| `-d tx` 或 `-d 0` | `chan_dir = BB_DIR_TX` | TX 方向 |
| `-d rx` 或 `-d 1` | `chan_dir = BB_DIR_RX` | RX 方向 |
| `-c <index>` | `chan_index = index` | 目标信道索引 |

`chan_index` 是否是设备支持的预置信道，由设备侧判断。示例程序只检查它是否在 `0-255` 范围内。

### `BB_SET_BANDWIDTH_MODE`

```c
bb_set_bandwidth_mode_t input;
input.slot = slot;
input.mode = mode;
bb_ioctl(handle, BB_SET_BANDWIDTH_MODE, &input, NULL);
```

| 参数 | 字段 | 含义 |
| --- | --- | --- |
| `-s <slot>` | `slot = slot` | 目标 slot |
| `-W 1` | `mode = 1` | 频宽自动 |
| `-W 0` | `mode = 0` | 频宽手动 |

### `BB_SET_BANDWIDTH`

```c
bb_set_bandwidth_t input;
input.slot = slot;
input.dir = dir;
input.bandwidth = bandwidth;
bb_ioctl(handle, BB_SET_BANDWIDTH, &input, NULL);
```

| 参数 | 字段 | 含义 |
| --- | --- | --- |
| `-s <slot>` | `slot = slot` | 目标 slot |
| `-d tx` 或 `-d 0` | `dir = BB_DIR_TX` | TX 方向 |
| `-d rx` 或 `-d 1` | `dir = BB_DIR_RX` | RX 方向 |
| `-w 0` | `bandwidth = BB_BW_1_25M` | 1.25M |
| `-w 1` | `bandwidth = BB_BW_2_5M` | 2.5M |
| `-w 2` | `bandwidth = BB_BW_5M` | 5M |
| `-w 3` | `bandwidth = BB_BW_10M` | 10M |
| `-w 4` | `bandwidth = BB_BW_20M` | 20M |
| `-w 5` | `bandwidth = BB_BW_40M` | 40M |

### `BB_SET_MCS_MODE`

```c
bb_set_mcs_mode_t input;
input.slot = slot;
input.auto_mode = auto_mode;
bb_ioctl(handle, BB_SET_MCS_MODE, &input, NULL);
```

| 参数 | 字段 | 含义 |
| --- | --- | --- |
| `-s <slot>` | `slot = slot` | 目标 slot |
| `-M 1` | `auto_mode = 1` | MCS 自适应 |
| `-M 0` | `auto_mode = 0` | MCS 手动 |

### `BB_SET_MCS`

```c
bb_set_mcs_t input;
input.slot = slot;
input.mcs = mcs;
bb_ioctl(handle, BB_SET_MCS, &input, NULL);
```

| 参数 | 字段 | 含义 |
| --- | --- | --- |
| `-s <slot>` | `slot = slot` | 目标 slot |
| `-m <mcs>` | `mcs = mcs` | SDK MCS 枚举值 |

注意：`-m` 使用的是 `bb_phy_mcs_e` 的枚举值，不是直接显示的调制档位名字。比如 `-m 0` 表示 `BB_PHY_MCS_NEG_2`，`-m 2` 表示 `BB_PHY_MCS_0`。

### `BB_SET_POWER_AUTO`

```c
bb_set_pwr_auto_in_t input;
input.pwr_auto = pwr_auto;
link_config_ioctl(handle, BB_SET_POWER_AUTO, &input, sizeof(input),
                  NULL, 0, remote_slot);
```

| 参数 | 字段 | 含义 |
| --- | --- | --- |
| `-O 1` | `pwr_auto = 1` | 开启功率自适应 |
| `-O 0` | `pwr_auto = 0` | 关闭功率自适应 |

### `BB_SET_POWER`

```c
bb_set_pwr_in_t input;
input.usr = user;
input.pwr = power;
link_config_ioctl(handle, BB_SET_POWER, &input, sizeof(input),
                  NULL, 0, remote_slot);
```

| 参数 | 字段 | 含义 |
| --- | --- | --- |
| `-u <user>` | `usr = user` | 物理用户 |
| `-P <power>` | `pwr = power` | 固定发射功率，范围 `0-31 dBm` |

未指定 `-u` 时，程序会读取实际执行端的 `BB_GET_STATUS`，AP 默认使用 `BB_USER_BR_CS`，DEV 默认使用 `BB_USER_0`。带 `-R` 时读取远端状态并在远端执行 `BB_SET_POWER`。

### `BB_SET_FRAME_CHANGE`

```c
bb_get_status_in_t status_input;
bb_get_status_out_t status;
bb_set_frame_change_t input;

link_config_ioctl(handle, BB_GET_STATUS, &status_input, sizeof(status_input),
                  &status, sizeof(status), remote_slot);
if (status.role != BB_ROLE_AP || status.mode != BB_MODE_SINGLE_USER) {
    return -1;
}

input.mode = mode;
link_config_ioctl(handle, BB_SET_FRAME_CHANGE, &input, sizeof(input),
                  NULL, 0, remote_slot);
```

| 参数 | 字段 | 含义 |
| --- | --- | --- |
| `-F 1` | `mode = 1` | 执行帧结构切换 |
| `-F 0` | `mode = 0` | 恢复原始帧结构 |

`BB_SET_FRAME_CHANGE` 仅支持 AP 端的 `BB_MODE_SINGLE_USER`。示例程序会在调用前读取 `BB_GET_STATUS`；带 `-R` 时读取远端状态。如果实际执行端不是 AP，或模式不是 `SINGLE_USER`，会直接报错退出，不下发 `BB_SET_FRAME_CHANGE`。

## 执行流程

执行：

```sh
./l4_link_config -B 0 -b 2g -C 0 -d rx -c 3 -s 0 -W 0 -w 4 -M 0 -m 6
```

程序内部大致按下面流程运行：

```text
解析命令行参数
    |
检查参数范围
    |
连接 daemon 和指定设备
    |
BB_SET_BAND_MODE
    |
BB_SET_BAND
    |
BB_SET_CHAN_MODE
    |
BB_SET_CHAN
    |
BB_SET_BANDWIDTH_MODE
    |
BB_SET_BANDWIDTH
    |
BB_SET_MCS_MODE
    |
BB_SET_MCS
    |
BB_SET_POWER_AUTO
    |
BB_SET_POWER
    |
BB_SET_FRAME_CHANGE（仅指定 -F 时，且实际执行端必须是 AP + SINGLE_USER；支持 -R）
    |
关闭设备连接
```

任意一步 `bb_ioctl()` 返回非 0 时，程序会停止后续设置，关闭连接并返回错误。相邻两个实际配置命令之间固定等待 200ms；带 `-R` 时，支持的命令会通过 `BB_REMOTE_IOCTL_REQ` 下发到 `-s <slot>` 指定的对端。

## 和 `03_link_monitor` 配合使用

设置完成后，可以用 `03_link_monitor` 读取当前链路信息：

```sh
./l4_link_monitor -C -M -T
```

它会读取信道信息、MCS 和吞吐信息，用于辅助确认配置效果。

## 新手最容易混淆的点

### 自动模式和手动设置不是一回事

`-B 1`、`-C 1`、`-W 1`、`-M 1`、`-O 1` 只是把设备切到自动模式。手动指定频段、信道、频宽、MCS 或功率时，通常要先设置对应模式为 `0`。

### 程序不会自动补发模式切换

如果只执行 `-w 4`，程序只调用 `BB_SET_BANDWIDTH`，不会自动调用 `BB_SET_BANDWIDTH_MODE 0`。同理，只执行 `-P 20` 时程序只调用 `BB_SET_POWER`，不会自动调用 `BB_SET_POWER_AUTO 0`。需要手动模式时，请显式写成 `-W 0 -w 4` 或 `-O 0 -P 20`。

### 固定频点配置的控制关系

固定频点流程：
固定AP频点：首先配置AP的频段模式为手动，然后设置AP的频段到2G/5G,然后设置信道模式为手动，设置信道。
固定DEV频点，也是首先配置AP的频段模式为手动。然后设置DEV的频段到2G/5G,然后设置信道模式为手动，设置信道。

命令流程：
在AP端固定AP频点到2400: l4_link_config -B 0 -b 1 -C 0 -c 0

在AP端固定DEV频点到2400： l4_link_config -B 0、l4_link_config -b 1 -C 0 -c 0 -R

### `-d` 同时服务信道和频宽

`-d` 会用于 `BB_SET_CHAN` 的 `chan_dir`，也会用于 `BB_SET_BANDWIDTH` 的 `dir`。如果同一条命令里同时设置 `-c` 和 `-w`，二者会使用同一个方向。

### `-R` 是远程 ioctl，不是 `BB_SET_REMOTE`

`-R` 会把 `BB_SET_*` 命令封装成 `BB_REMOTE_IOCTL_REQ` 发给对端执行。它不会额外调用 `BB_SET_REMOTE`。`-W`、`-w`、`-O`、`-P`、`-F` 也走同一条远程 ioctl 路径；其中 `-F` 会先确认实际执行端是 AP。

## 一句话总结

`l4_link_config` 做的事情就是：连接 daemon，打开指定 8030 设备，然后按参数下发频段、信道、频宽、MCS、功率和帧结构相关配置；带 `-R` 时，配置会远程下发给 `-s <slot>` 指定的对端，其中 `-F` 只允许实际执行端为 AP。

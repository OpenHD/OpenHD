# 03_link_monitor 对频后链路信息读取示例

`l4_link_monitor` 用于读取链路状态、信号质量、MCS、MCS 模式、频宽模式、功率、信道、吞吐和测距结果。它适合用作图传链路体检工具。

## 常用命令

```sh
./l4_link_monitor
```

默认执行全量查询，等同于 `-A`。

```sh
./l4_link_monitor -s 0 -T
```

只查询 slot 0 的 TX/RX 实时吞吐。

```sh
./l4_link_monitor -s 0 -D
```

只查询 slot 0 的测距结果。

```sh
./l4_link_monitor -s 0 -M
```

只查询 slot 0 的 TX/RX MCS 和理论吞吐。

```sh
./l4_link_monitor -m -W
```

只查询 slot 0 的 MCS 控制模式和频宽控制模式。`auto_mode=1` 表示自动，`auto_mode=0` 表示手动。

```sh
./l4_link_monitor -u 0 -Q -P
```

查询用户质量和当前发射功率；未指定 `-u` 时，`-Q` 默认 AP=0、DEV=8，`-P` 默认 AP=8、DEV=0。

```sh
./l4_link_monitor -B
```

只查询当前频段控制模式和工作频段。`band_mode=0` 表示手动控制，`band_mode=1` 表示频段自适应。

```sh
./l4_link_monitor -C -R -s 0
```

通过 slot 0 远程查询对端设备的信道信息；也可以和 `-S/-Q/-q/-M/-m/-W/-P/-C/-B/-T/-D/-V` 任意组合。不带 `-R` 时仍查询本机信息。

```sh
./l4_link_monitor -V
```

查询 1V1 模式 self/peer 链路信息，只输出 SNR、LDPC 错误比例、A/B 路 gain、TX MCS、TX channel、TX power 和 TX 频点。

## 参数说明

| 参数 | 作用 |
| --- | --- |
| `-a <addr>` | daemon 地址，默认 `127.0.0.1` |
| `-p <port>` | daemon 端口 |
| `-i <index>` | 设备序号，默认 `0` |
| `-s <slot>` | 目标 slot，默认 `0`；DEV 侧 slot 0 表示 AP |
| `-u <user>` | 指定 `-Q`/`-P` 的物理用户；未指定时 `-Q` 默认 AP=0、DEV=8，`-P` 默认 AP=8、DEV=0 |
| `-A` | 查询全部链路信息 |
| `-S` | 查询 `BB_GET_STATUS` |
| `-Q` | 查询 `BB_GET_USER_QUALITY` |
| `-q` | 查询 `BB_GET_PEER_QUALITY` |
| `-M` | 查询 `BB_GET_MCS` |
| `-m` | 查询 `BB_GET_MCS_MODE` |
| `-W` | 查询 `BB_GET_BANDWIDTH_MODE` |
| `-P` | 查询 `BB_GET_CUR_POWER` |
| `-C` | 查询 `BB_GET_CHAN_INFO` |
| `-R` | 通过远程 ioctl 查询 `-s <slot>` 指定的对端；支持 `-A/-S/-Q/-q/-M/-m/-W/-P/-C/-B/-T/-D/-V` |
| `-B` | 查询 `BB_GET_BAND_INFO` |
| `-T` | 查询 `BB_GET_THROUGHPUT` |
| `-D` | 查询 `BB_GET_DISTC_RESULT` |
| `-V` | 查询 `BB_GET_1V1_INFO` |

## 读取的信息

| API | 主要输出 |
| --- | --- |
| `BB_GET_STATUS` | 角色、模式、本机 MAC、slot 链路状态、peer MAC、映射后的 user phy status |
| `BB_GET_USER_QUALITY` | 指定 user 的 SNR 原始值、换算 dB、LDPC、A/B 路 gain |
| `BB_GET_PEER_QUALITY` | 指定 slot 对端数据通道质量，SNR 按 `10log10(snr/36)` 换算 dB |
| `BB_GET_MCS` | 指定 slot 的 TX/RX MCS 原始值、真实值 `raw-2` 和理论吞吐 |
| `BB_GET_MCS_MODE` | 指定 slot 的 MCS 控制模式：`0=手动`、`1=自动` |
| `BB_GET_BANDWIDTH_MODE` | 指定 slot 的频宽控制模式：`0=手动`、`1=自动` |
| `BB_GET_CUR_POWER` | 指定 user 当前发射功率 |
| `BB_GET_CHAN_INFO` | 信道数量、自适应模式、ACS 信道、工作信道、频点和扫频能量 |
| `BB_GET_BAND_INFO` | 频段控制模式：`0=手动`、`1=自适应`，以及当前工作频段 |
| `BB_GET_THROUGHPUT` | 指定 slot 的 TX/RX 物理吞吐和实际承载吞吐 |
| `BB_GET_DISTC_RESULT` | 指定 slot 的测距结果，`-1` 表示当前无测距结果 |
| `BB_GET_1V1_INFO` | 1V1 模式 self/peer 链路信息：SNR、LDPC 错误比例、gain、TX MCS、TX channel、TX power、TX 频点 |



## 读取前置条件

`BB_GET_USER_QUALITY`、`BB_GET_PEER_QUALITY`、`BB_GET_MCS`、`BB_GET_CUR_POWER`、`BB_GET_THROUGHPUT`、`BB_GET_DISTC_RESULT` 这些链路细节只有在图传已经对频/连接后才读取。程序会先读取实际查询端的 `BB_GET_STATUS`，带 `-R` 时读取远端状态；确认指定 slot 满足 `pair_state=1` 或 `state=CONNECT` 后才继续，否则会打印当前状态并跳过链路细节查询。

`BB_GET_STATUS`、`BB_GET_MCS_MODE`、`BB_GET_BANDWIDTH_MODE`、`BB_GET_CHAN_INFO`、`BB_GET_BAND_INFO` 和 `BB_GET_1V1_INFO` 可作为基础状态信息单独读取，不受配对/连接状态检查限制；带 `-R` 时同样会通过 `BB_REMOTE_IOCTL_REQ` 在对端执行。

## 数值换算

- SNR 输出 `snr_raw` 和 `snr_db`；`snr_db` 使用公式 `10log10(snr_raw/36)` 换算。
- MCS 输出 `mcs_raw` 和 `mcs_real`；真实 MCS 为 `mcs_raw - 2`。
- `slot link status` 中的 `rx_mcs_raw` 也按 `rx_mcs_raw - 2` 输出真实值 `rx_mcs_real`。
- `user phy status` 会按角色映射逻辑 RX/TX：AP RX=`BB_USER_0.rx`、AP TX=`BB_USER_BR_CS.tx`、DEV RX=`BB_USER_BR_CS.rx`、DEV TX=`BB_USER_0.tx`。RX 只输出 RX 对象，不使用 RX 端物理 MCS 字段。
- `bw_mode` 和 `major_dir` 只根据 RX 行的 `tintlv_len/tintlv_num` 解释，并单独一行输出。`tintlv_len=3,tintlv_num=1` 显示为 `Y24X2`，大带宽方向 `DEV->AP`；`tintlv_len=2,tintlv_num=0` 显示为 `Y12X1`，大带宽方向 `AP->DEV`。
- 测距结果直接输出 SDK 返回的 `distance[slot]`；`-1` 表示当前没有测距结果。

## slot 和 user

AP 侧通常用 `-s <slot>` 指定要查看哪个 DEV 所在的 slot。DEV 侧使用 `-s 0` 查看 AP 方向，因为 SDK 中 `BB_SLOT_AP` 等于 0。

`-u <user>` 用于覆盖 `BB_GET_USER_QUALITY` 和 `BB_GET_CUR_POWER` 的物理用户。未指定 `-u` 时，程序会先读取实际查询端角色：`-Q` 在 AP 默认 user 0、DEV 默认 user 8；`-P` 在 AP 默认 user 8、DEV 默认 user 0。

## 远程查询

`-R` 会把查询命令封装成 `BB_REMOTE_IOCTL_REQ`，由 `-s <slot>` 指定的对端执行。远程查询支持 `-A` 和所有单项查询参数，包括 `-S`、`-Q`、`-q`、`-M`、`-m`、`-W`、`-P`、`-C`、`-B`、`-T`、`-D`、`-V`。输出标题会显示 `remote slot=<slot>`，例如 `[BB_GET_STATUS remote slot=0]`。

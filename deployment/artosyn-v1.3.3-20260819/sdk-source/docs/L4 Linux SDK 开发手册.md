# L4 Linux SDK 开发手册


本文档面向 Linux SDK 使用者，说明快速接入、常用工具和 API 示例的使用方法。`README.md` 只保留项目入口、最小编译和最小验证；本文档作为完整开发、部署和排错说明。

## 一、SDK 编译使用

### 快速了解

SDK 最小运行链路如下图所示：

```text
+--------------------------------------------------+
| Linux user program                               |
| l4_basic_info / customer application             |
+-------------------------+------------------------+
                          | Call SDK APIs
                          v
+--------------------------------------------------+
| libar8030_client.so                              |
| Client library, wraps RPC communication          |
+-------------------------+------------------------+
                          | Connect to local daemon
                          v
+--------------------------------------------------+
| l4_daemon                                        |
| Device access, message forwarding, hotplug       |
+-------------------------+------------------------+
                          | USB 
                          v
+--------------------------------------------------+
| L4-BOX / 8030 device                             |
| Baseband control, status query, data transfer    |
+--------------------------------------------------+
```
 
常用文件如下：

| 文件 | 说明 |
|-|-|
| `script/cmk-local.sh` | x86_64 本机编译脚本。 |
| `script/cmk-arm.sh` | arm64 交叉编译脚本。 |
| `install/<arch>/bin/l4_daemon` | 后台通信服务。 |
| `install/<arch>/bin/l4_basic_info` | 基础信息查询和最小验证程序。 |
| `install/<arch>/lib/libar8030_client.so` | 客户端动态库。 |
| `com/bb_api.h` | SDK 核心 API 头文件。 |
| `com/prj_rpc.h` | `BB_GET_PRJ_DISPATCH` / `BB_SET_PRJ_DISPATCH` 二级命令结构体。 |
| `app/ar8030/ar_net_api.h` | netdev 相关 API 头文件。 |

当前精简 SDK 不包含原厂 `driver/linux/` 内核模块源码；该目录用于构建 `artosyn_drv.ko`，如需交付内核驱动，应单独迁移并按目标内核版本验证。

`<arch>` 根据运行平台选择：

| 运行平台 | 架构目录 |
|-|-|
| Ubuntu PC / Ubuntu 虚拟机 | `x86_64` |
| arm64 Linux 开发板 | `arm64` |

Linux 环境至少需要：

- CMake。
- GCC / G++。
- Shell 执行环境。
- 可访问 L4-BOX 的 USB 权限，必要时使用 `sudo`。

连接 L4-BOX 后，可先执行：

```sh
lsusb
```

正常输出示例：

```text
ID 1d6b:8030 Linux Foundation Artosyn in HS Mode
```

### 编译 SDK

进入 SDK 目录：

```sh
cd L4_Linux_SDK
```

如果之前编译过，可先清理历史产物：

```sh
./script/clean.sh all
```

#### 本机 x86_64 编译

```sh
./script/cmk-local.sh
```

编译和安装产物输出到：

```text
L4_Linux_SDK/install/x86_64/
```

常用可执行程序位于 `install/x86_64/bin/` 目录，动态库位于 `install/x86_64/lib/` 目录。

#### ARM64 交叉编译

##### 准备交叉编译工具链

当前 SDK 的 `compiler.arm.cmake` 默认期望工具链位于：

```text
L4_Linux_SDK/toolchain/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu
```

其中编译器前缀为：

```text
bin/aarch64-none-linux-gnu-
```

如果工具链放在其他目录，需要先修改 `compiler.arm.cmake` 中的 `GCC_PATH`。

##### 执行交叉编译

```sh
./script/cmk-arm.sh
```

编译和安装产物输出到：

```text
L4_Linux_SDK/install/arm64/
```

常用可执行程序位于 `install/arm64/bin/` 目录，动态库位于 `install/arm64/lib/` 目录。

两个编译脚本默认构建并安装 `ar8030_client`、`l4_daemon`、`l4_cmd_dbg`、`l4_tuntap`、`l4_ota_upgrade` 和 `examples/` 下全部业务示例。需要只构建部分 target 或调整 CMake 选项时，请参考“高级构建”章节。

### x86_64 最快验证

适用于 Ubuntu PC 或 Ubuntu 虚拟机。目标是先在本机完成最小链路验证。

1. 启动 `l4_daemon`：

```sh
cd L4_Linux_SDK/install/x86_64/bin
sudo ./l4_daemon
```

2. 另开终端读取版本：

```sh
./l4_basic_info -V
```

正常输出示例：

```text
[BB_GET_SYS_INFO]
uptime       : 51232 ms
compile_time : 01/28/2026 19:57:27
soft_ver     : 1.3.02-21
hardware_ver : Unknown_4.0
firmware_ver : KT-2458-G-V1.1.8-U
```

### arm64 最快验证

适用于 arm64 Linux 开发板。PC 上编译出的 `x86_64` 程序不能直接运行在开发板上，需要先交叉编译 `arm64` 产物，再部署到开发板。

1. 在 PC 侧编译 arm64 产物：

```sh
cd L4_Linux_SDK
./script/cmk-arm.sh
```

2. 在开发板上准备目录：

```sh
mkdir -p ~/l4box/bin ~/l4box/lib
```

3. 将 `install/arm64/bin/l4_daemon`、`install/arm64/bin/l4_basic_info` 和 `install/arm64/lib/libar8030_client.so` 复制到对应目录。
4. 在开发板上运行：

```sh
cd ~/l4box/bin
chmod +x l4_daemon l4_basic_info
sudo ./l4_daemon
```

5. 另开终端读取版本：

```sh
cd ~/l4box/bin
export LD_LIBRARY_PATH=../lib:$LD_LIBRARY_PATH
./l4_basic_info -V
```

### 启动 daemon

SDK 应用程序通常先连接 `l4_daemon`，再通过 daemon 访问 8030 设备。运行工具或示例前，先在设备连接正常的环境中启动 daemon：

```sh
cd L4_Linux_SDK/install/x86_64/bin
./l4_daemon
```

如果是 ARM64 设备，请进入 `install/arm64/bin` 后执行同样命令。

### 运行工具

以下命令示例默认在 `install/<arch>/bin` 目录执行：

```sh
cd L4_Linux_SDK/install/x86_64/bin
```

如需在其它目录运行，请确保能正确找到 `libar8030_client.so` 等 SDK 动态库。

### 从最小验证到开发

最小验证通过后，常见调用顺序如下：

```text
bb_host_connect("127.0.0.1", BB_PORT_DEFAULT)
  -> bb_dev_getlist(...)
  -> bb_dev_open(...)
  -> bb_ioctl(..., BB_GET_SYS_INFO, ...)
  -> bb_dev_close(...)
  -> bb_dev_freelist(...)
  -> bb_host_disconnect(...)
```

建议先参考 `examples/01_basic_info` 的基础信息查询流程，再根据需求阅读 `examples/02_pair_manager` 到 `examples/07_uart_config`。

### 通用参数说明

本文档中的应用工具和 API 示例程序统一支持以下通用参数，后续各章节只说明各程序自身的业务参数。


| 参数           | 说明                 | 默认值               |
| ------------ | ------------------ | ----------------- |
| `-h, --help` | 打印帮助信息             | 无                 |
| `-a <addr>`  | daemon 地址          | `127.0.0.1`       |
| `-p <port>`  | daemon 端口          | `BB_PORT_DEFAULT` |
| `-i <index>` | 选择 daemon 枚举到的设备序号 | `0`               |


查看帮助信息：

```sh
./l4_basic_info -h
```

连接指定 daemon：

```sh
./l4_basic_info -a 192.168.1.10 -p 9000
```

选择第 1 个设备：

```sh
./l4_basic_info -i 1
```

如果不确定要选择哪个 index，可先用 `l4_basic_info` 列出 daemon 当前设备：

```sh
./l4_basic_info -l
```

这些通用参数可与各程序自身业务参数组合使用，例如：

```sh
./l4_link_monitor -a 192.168.1.10 -p 9000 -i 1 -S
```

## 二、应用工具

### 1、固件升级工具

`l4_ota_upgrade` 用于对 8030 设备执行固件升级。当前手册先只介绍通过 `-f` 指定升级镜像的基础用法。

#### 1.1 程序参数说明


| 参数                  | 说明             | 默认值 |
| ------------------- | -------------- | --- |
| `-f, --file <file>` | 指定待升级的固件镜像文件路径 | `无` |


`-f` 后面应填写 `.img` 固件文件路径。路径可以是相对路径，也可以是绝对路径。

#### 1.2 标准使用方法

1. 确认 `l4_daemon` 已启动。
2. 确认 8030 设备连接正常。
3. 准备待升级固件镜像，例如 `VT4-KT-2458-G-V1.1.8-U.img`。
4. 执行 `l4_ota_upgrade -f <固件镜像路径>`。
5. 等待升级进度到 100%，程序返回成功后再进行断电、重启或后续验证。

升级过程中不要断开设备连接，也不要关闭 daemon。

#### 1.3 示例

使用相对路径升级：

```sh
./l4_ota_upgrade -f ../../firmware/KT-2458-G-V1.1.8-U/VT4-KT-2458-G-V1.1.8-U.img
```

使用绝对路径升级：

```sh
./l4_ota_upgrade -f /opt/firmware/VT4-KT-2458-G-V1.1.8-U.img
```

正常输出示例：

```text
./l4_ota_upgrade compiled at: Jun  6 2026 14:15:56
upgrade file: ../../../../firmware/KT-2458-G-V1.1.8-U/VT4-KT-2458-G-V1.1.8-U.img
file size = 898916

====================upgrade bin infomation====================
magic:            0x4152544f
img size:         0xdb544
partitions:       0x5
segments:         0x2
...
====================upgrade bin infomation done====================

Verify upgrade image...
Verify upgrade image success
update romcode data=0x796e2ca8d230, len=0x8000
ar8030_upgrade_partition addr 0 len 32768 crc b53913ab
update process 100%
firmware_ver: KT-2458-G-V1.1.8-U
running sys is 0
...
update process 100%
upgrade done
```

看到 `update process 100%` 和 `upgrade done`，表示升级流程正常结束。

### 2、网络透传工具

`l4_tuntap` 用于通过 TAP 设备实现基带数据透传能力。它会把 Linux 网络侧数据封装后写入基带 socket，也会把基带收到的数据还原后写入 TAP 设备，从而实现网络透传功能。

`l4_tuntap` 仅适用于网络通过 USB 透传的设备，也就是图传固件版本以 `-U` 结尾的配置。固件版本以 `-E` 结尾时，网络通过设备网口通信，不需要使用 `l4_tuntap`。

#### 2.1 程序参数说明


| 参数                     | 说明                  | 默认值     |
| ---------------------- | ------------------- | ------- |
| `-u, --user <user>`    | 指定 baseband user id | `0`     |
| `-P, --transport <id>` | 指定透传 transport id   | `3`     |
| `-I, --tap-ip <ip>`    | 指定 TAP 设备 IP，必填     | 无       |
| `-d, --dev <dev>`      | 指定 TAP 设备名          | `tap0`  |
| `-v, --debug`          | 打开网络透传调试打印          | 关闭      |
| `-k, --force-close-on-open-fail` | socket 打开失败时强制关闭同 slot/transport 的 socket 并重试一次 | 关闭 |
| `-r, --rx_buf <len>`   | 指定 RX buffer 长度     | `40000` |
| `-t, --tx_buf <len>`   | 指定 TX buffer 长度     | `60000` |


`-I` 是网络透传工具的关键参数，用于给 TAP 虚拟网卡配置 IP。两端设备应配置在同一网段，并避免 IP 冲突。

如果异常退出后再次启动提示基带 socket 创建失败，可增加 `-k`。该参数会在首次打开当前 `-u` 和 `-P` 指定的 socket 失败时调用 `BB_FORCE_CLS_SOCKET`，等待 200ms 后自动重试一次。

`l4_tuntap` 启动时使用 `-i` 选择设备，并记录该设备的 MAC/序列号。运行期间如果设备断开或 socket 失效，程序会关闭旧 socket 和设备句柄，每 2 秒重新扫描一次设备；每次失败都会打印尝试序号、失败阶段和下次等待时间。同一 MAC/序列号的设备重新上线后，程序会使用设备最新的 working ID 重新打开 socket，并打印成功所在的尝试序号，无需重启 `l4_tuntap`。断线期间从 TAP 读取的报文会被丢弃，不会在恢复后补发。

自动重连同样遵循 `-k` 设置：未指定 `-k` 时只持续尝试正常打开 socket；指定 `-k` 后，每次设备重新打开但 socket 创建失败时，会强制关闭当前 slot/transport 上的残留 socket 并重试一次。

#### 2.2 标准使用方法

1. AP 和 DEV 侧均启动 `l4_daemon`。
2. 确认 AP 和 DEV 已完成配对并建立链路。
3. AP 侧和 DEV 侧分别运行 `l4_tuntap`，并配置不同的 TAP IP。
4. 使用 `ping`、`iperf` 或业务程序通过 TAP IP 验证网络透传。

如果 TAP 设备创建或配置失败，通常需要使用 root 权限运行。

#### 2.3 示例

AP 侧配置 TAP IP：

```sh
sudo ./l4_tuntap -I 192.168.144.55 -d tap0
```

DEV 侧配置 TAP IP：

```sh
sudo ./l4_tuntap -I 192.168.144.66 -d tap0
```

`l4_tuntap` 正常启动输出示例：

```text
l4_tuntap args: -u 0 -P 3 -I 192.168.144.55 -d tap0 -r 40000 -t 60000 -k 0
dev = tap0,ip = 192.168.144.55,mtu = 4000 , tun_fd = 4
lgeng - 0  ar_bb_socket_fd = 0
```

看到 TAP 设备名、IP、`tun_fd` 和 `ar_bb_socket_fd` 都正常打印，表示 TAP 设备和基带 socket 已创建成功。

`ifconfig` 查看tap0 网卡详情

```
tap0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 4000
        inet 192.168.144.55  netmask 255.255.255.0  broadcast 192.168.144.255
        inet6 fe80::3088:49ff:fe90:c09c  prefixlen 64  scopeid 0x20<link>
        ether f2:a0:f6:85:63:4d  txqueuelen 1000  (以太网)
        RX packets 168  bytes 18353 (18.3 KB)
        RX errors 0  dropped 2  overruns 0  frame 0
        TX packets 50  bytes 6073 (6.0 KB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
```

随后 `ping` 能收到回复，表示网络透传链路正常。

```
ping 192.168.144.66
PING 192.168.144.66 (192.168.144.66) 56(84) bytes of data.
64 bytes from 192.168.144.66: icmp_seq=1 ttl=64 time=31.9 ms
64 bytes from 192.168.144.66: icmp_seq=2 ttl=64 time=28.0 ms
64 bytes from 192.168.144.66: icmp_seq=3 ttl=64 time=34.2 ms
64 bytes from 192.168.144.66: icmp_seq=4 ttl=64 time=31.4 ms
64 bytes from 192.168.144.66: icmp_seq=5 ttl=64 time=27.7 ms
^C
--- 192.168.144.66 ping statistics ---
5 packets transmitted, 5 received, 0% packet loss, time 4005ms
rtt min/avg/max/mdev = 27.678/30.619/34.187/2.471 ms
```

### 3、命令行调试工具

`l4_cmd_dbg` 用于建立设备调试会话。默认模式从标准输入逐行读取命令并发送到设备，同时将设备返回的调试数据原样输出到终端。

#### 3.1 程序参数说明

除通用的 daemon 地址、端口和设备序号参数外，`l4_cmd_dbg` 还支持以下参数：

| 参数 | 说明 | 默认值 |
| --- | --- | --- |
| `-m, --mac <mac>` | 按 MAC 地址选择设备，不能与 `-i/--index` 同时使用 | 无 |
| `-l, --list` | 列出 daemon 当前设备及 MAC 地址后退出 | 关闭 |
| `-o, --output` | 仅接收设备调试输出，不读取和发送标准输入 | 关闭 |

#### 3.2 标准使用方法

1. 启动 `l4_daemon` 并确认设备已连接。
2. 多设备环境可先执行 `l4_cmd_dbg -l` 查看设备序号和 MAC 地址。
3. 使用默认设备、`-i <index>` 或 `-m <mac>` 建立调试会话。
4. 在交互模式下输入设备固件支持的调试命令；设备离线或标准输入结束后程序退出。

#### 3.3 示例

连接默认设备并进入交互模式：

```sh
./l4_cmd_dbg
```

列出设备：

```sh
./l4_cmd_dbg -l
```

按设备序号或 MAC 地址选择设备：

```sh
./l4_cmd_dbg -i 1
./l4_cmd_dbg -m 00:11:22:33:44:55
```

只查看设备调试输出：

```sh
./l4_cmd_dbg -o
```


## 三、API 示例

SDK 在 `examples/` 下提供 8 个业务 API 示例，另有 `examples/00_common` 公共连接库。业务示例都复用 `examples/00_common` 中的公共连接流程：

```text
bb_host_connect()
    |
bb_dev_getlist()
    |
bb_dev_open()
    |
bb_ioctl() / bb_socket_open()
    |
bb_dev_close() / bb_dev_freelist() / bb_host_disconnect()
```

示例程序默认连接本机 daemon，并打开第 0 个设备。详细 API 结构体和命令字说明可参考 `com/bb_api.h`、`com/prj_rpc.h` 和 `app/ar8030/ar_net_api.h`。

> 2026-06-22 同步说明：`bb_net_dev_create()`、`bb_net_dev_destroy()`、`bb_net_dev_buf_resize()` 已跟随新原厂 SDK 改为接收 `bb_dev_handle_t *`，由 SDK 内部打开和关闭 netdev 设备；旧版直接传入 netdev fd 的调用方式不再使用。

> 2026-07-10 同步说明：SDK 已同步原厂 `ar8030-coolsky-sdk-1.3.02-28-release-20260708` 的 Socket 加密、MCS 模式查询和频宽模式查询接口。`bb_sock_opt_t`、`bb_auto_band_para_t`、`ar_netif_t` 等公共结构发生变化，客户端库、daemon 和应用应作为同一版本整体清理重编。普通 Socket 保持旧版 12 字节创建协议兼容；加密 Socket 要求新版客户端、daemon 和固件配套使用。

### 1、信息查询示例

`l4_basic_info` 用于查询设备基础信息，包括系统版本、角色、模式、MAC、slot 链路状态和 user phy 状态。

#### 1.1 程序参数说明


| 参数   | 说明                             | 默认值  |
| ---- | ------------------------------ | ---- |
| `-S` | 查询 `BB_GET_STATUS`，打印设备状态和链路状态 | 无    |
| `-V` | 查询 `BB_GET_SYS_INFO`，打印系统和版本信息 | 无    |
| `-A` | 查询全部基础信息                       | 默认动作 |
| `-l` | 查看 daemon 当前设备列表，不打开设备          | 无    |


如果不指定 `-S`、`-V` 或 `-A`，程序默认执行全量查询。

#### 1.2 标准使用方法

1. 启动 `l4_daemon`。
2. 如有多个设备，可先执行 `l4_basic_info -l` 查看设备列表，再用 `-i <index>` 选择目标设备。
3. 执行 `l4_basic_info` 查询基础状态。
4. 根据输出确认设备角色、工作模式、版本和链路状态。

#### 1.3 示例

##### 1.3.1 查看当前设备列表：

```sh
./l4_basic_info -l
```

`-l` 只连接 daemon 并打印 `device[index]` 和 MAC，不会打开设备，也不会执行 `BB_GET_SYS_INFO` 或 `BB_GET_STATUS`。

##### 1.3.2 查询全部基础信息：

```sh
./l4_basic_info
```

##### 1.3.3 只查询系统版本：

```sh
./l4_basic_info -V
```

正常输出示例:

```
[BB_GET_SYS_INFO]
uptime       : 176706959 ms
compile_time : 01/28/2026 19:57:27
soft_ver     : 1.3.02-21
hardware_ver : Unknown_4.0
firmware_ver : KT-2458-G-V1.1.8-U
```

##### 1.3.4 只查询设备状态：

```sh
./l4_basic_info -S
```

正常输出示例:

```
[BB_GET_STATUS]
role        : AP (0)
mode        : SINGLE_USER (0)
sync_mode   : 0
sync_master : 1
cfg_sbmp    : 0x01
rt_sbmp     : 0x01
local_mac   : a5:58:21:c2

slot link status:
  slot[0]: state=CONNECT(2), rx_mcs_raw=11, rx_mcs_real=9, pair=0, peer_mac=64:2d:80:9f

user phy status:
  RX: user=0 source=rx freq=2477000KHz bw=10MHz(3) tintlv_enable=1 tintlv_len=3 tintlv_num=1
  TX: user=8 source=tx freq=2395625KHz mcs_raw=2 mcs_real=0 bw=2.5MHz(1) tintlv_enable=1 tintlv_len=3 tintlv_num=0
  bw_mode=Y24X2 major_dir=DEV->AP
```

### 2、配对管理示例

`l4_pair_manager` 用于启动配对、停止配对、查询配对结果，以及手动设置对端 MAC。

#### 2.1 程序参数说明


| 参数          | 说明             | 默认值   |
| ----------- | -------------- | ----- |
| `-P`        | 启动配对，并等待配对结果事件 | 无     |
| `-X`        | 停止当前配对         | 无     |
| `-m`        | 查询配对结果和对端 MAC  | 无     |
| `-M <mac>`  | 按当前角色设置对端 MAC  | 无     |
| `-s <slot>` | AP 侧开放的 slot   | `0`   |
| `-t <sec>`  | 设备侧配对超时时间      | `100` |


AP 侧启动配对时，`-s` 表示开放哪个 slot 给 DEV 接入。DEV 侧启动配对时通常不需要指定 slot。

#### 2.2 标准使用方法

1. AP 和 DEV 侧分别启动 `l4_daemon`。
2. AP 侧执行 `l4_pair_manager -P -s <slot>`，开放目标 slot。
3. DEV 侧执行 `l4_pair_manager -P`，进入配对流程。
4. 等待 `BB_EVENT_PAIR_RESULT` 事件返回。
5. 配对成功后执行 `l4_pair_manager -m` 查询对端 MAC。

配对是异步动作，程序会先订阅 `BB_EVENT_PAIR_RESULT`，再下发配对命令，并持续等待设备返回事件；对频成功或设备侧超时都会通过事件回调返回。

#### 2.3 示例

##### 2.3.1 启动对频

AP 侧开放 slot 0 配对：

```sh
./l4_pair_manager -P
```

正常输出示例：

```text
device role: AP (0)
pair callback registered
start pair: role=AP slot=0 bitmap=0x01 timeout=100 asyn=1
waiting for pair result...
waiting for pair result...

[BB_EVENT_PAIR_RESULT]
pair ret=0 slot=0
pair ok
device role: AP (0)

[BB_GET_CANDIDATES]
slot: 0 mac_num: 1
dev_mac[0]: 65:a6:78:c7
```

DEV 侧启动配对：

```sh
./l4_pair_manager -P
```

正常输出示例：

```text
device role: DEV (1)
pair callback registered
start pair: role=DEV slot=0 bitmap=0x00 timeout=100 asyn=1
waiting for pair result...
waiting for pair result...

[BB_EVENT_PAIR_RESULT]
pair ret=0 slot=0
pair ok
device role: DEV (1)

[BB_GET_AP_MAC]
ap_mac: a5:58:21:c2
```

##### 2.3.2 查询配对结果

配对完成后，可查询当前设备已记录的对端 MAC。AP 侧会查询候选 DEV MAC，DEV 侧会查询 AP MAC。

```sh
./l4_pair_manager -m
```

输出中应出现 `[BB_GET_CANDIDATES]` 或 `[BB_GET_AP_MAC]`，用于确认当前角色对应的配对结果。

##### 2.3.3 手动设置配对

假设 AP MAC 地址为 11:22:33:44，DEV MAC 地址为 AA:BB:CC:DD。

AP 侧手动设置 slot 0 的 DEV MAC：

```sh
./l4_pair_manager -M AA:BB:CC:DD
```

DEV 侧手动设置 AP MAC：

```sh
./l4_pair_manager -M 11:22:33:44
```

设置完成后，两边设备会自动对频。

##### 2.3.4 停止配对

如果设备正在配对，可停止当前配对流程：

```sh
./l4_pair_manager -X
```

命令成功后，设备侧会退出当前配对等待状态。

##### 2.3.5 指定配对超时时间

通过 `-t <sec>` 指定设备侧配对超时时间。下面示例将超时时间设置为 60 秒：

```sh
./l4_pair_manager -P -t 60
```

输出中的 `timeout=60` 表示该超时时间已随配对命令下发。

### 3、链路监控示例

`l4_link_monitor` 用于读取链路状态、信号质量、MCS、MCS 模式、频宽模式、功率、信道、频段、吞吐和测距结果。

#### 3.1 程序参数说明


| 参数          | 说明                         | 默认值  |
| ----------- | -------------------------- | ---- |
| `-A`        | 查询全部链路信息                   | 默认动作 |
| `-S`        | 查询 `BB_GET_STATUS`         | 无    |
| `-Q`        | 查询 `BB_GET_USER_QUALITY`   | 无    |
| `-q`        | 查询 `BB_GET_PEER_QUALITY`   | 无    |
| `-M`        | 查询 `BB_GET_MCS`            | 无    |
| `-m`        | 查询 `BB_GET_MCS_MODE`       | 无    |
| `-W`        | 查询 `BB_GET_BANDWIDTH_MODE` | 无    |
| `-P`        | 查询 `BB_GET_CUR_POWER`      | 无    |
| `-C`        | 查询 `BB_GET_CHAN_INFO`      | 无    |
| `-R`        | 通过远程 ioctl 查询 `-s <slot>` 指定的对端，支持全量和单项查询 | 无    |
| `-B`        | 查询 `BB_GET_BAND_INFO`      | 无    |
| `-T`        | 查询 `BB_GET_THROUGHPUT`     | 无    |
| `-D`        | 查询 `BB_GET_DISTC_RESULT`   | 无    |
| `-V`        | 查询 `BB_GET_1V1_INFO`       | 无    |
| `-s <slot>` | 目标 slot；DEV 侧 slot 0 表示 AP | `0`  |
| `-u <user>` | 指定 `-Q`/`-P` 的物理用户；未指定时 `-Q` 默认 AP=0、DEV=8，`-P` 默认 AP=8、DEV=0 | 见说明 |


链路质量、实时 MCS、功率、吞吐和测距结果等细节需要在配对完成、链路连接后读取。程序会先查询 `BB_GET_STATUS` 判断目标 slot 是否可用。MCS 模式和频宽模式属于配置状态，不受配对/连接状态检查限制。未指定 `-u` 时会按实际查询端角色选择默认 user：`-Q` 在 AP 为 0、DEV 为 8；`-P` 在 AP 为 8、DEV 为 0。

#### 3.2 标准使用方法

1. 启动 `l4_daemon`。
2. 使用 `l4_pair_manager` 完成 AP 和 DEV 配对。
3. 执行 `l4_link_monitor` 查询全量链路信息。
4. 根据需要通过 `-S/-Q/-M/-m/-W/-T/-V` 等参数查询指定信息。
5. AP 侧通过 `-s` 指定目标 DEV 所在 slot；DEV 侧通常使用 `-s 0` 查看 AP 方向。
6. 默认查询本机信息；需要查询对端时增加 `-R -s <slot>`，可配合 `-A/-S/-Q/-q/-M/-m/-W/-P/-C/-B/-T/-D/-V` 使用。

#### 3.3 示例

##### 3.3.1 查询全部链路信息

```sh
./l4_link_monitor
```

##### 3.3.2 查询链路基础信息

```sh
./l4_link_monitor -V
```

正常输出示例：

```text
[BB_GET_1V1_INFO]
  self: snr_raw=4013 snr_db=20.47 dB ldpc_tlv_err_ratio=0 ldpc_num_err_ratio=0 gain_a=33 gain_b=25 tx_mcs=2 tx_chan=10 tx_power=15 tx_freq_khz=5590625
  peer: snr_raw=1010 snr_db=14.48 dB ldpc_tlv_err_ratio=0 ldpc_num_err_ratio=0 gain_a=33 gain_b=37 tx_mcs=11 tx_chan=8 tx_power=15 tx_freq_khz=5110000
```

##### 3.3.3 查询链路信号质量

AP 端：

```sh
./l4_link_monitor -Q -q
```

正常输出示例：

```text
[BB_GET_USER_QUALITY]
user[0]: snr_raw=3900 snr_db=20.35 dB ldpc=0/96 gain_a=34 gain_b=26

[BB_GET_PEER_QUALITY]
slot[0]: snr_raw=1215 snr_db=15.28 dB ldpc=0/2 gain_a=33 gain_b=37
```

DEV 端：

```sh
./l4_link_monitor -Q -q
```

正常输出示例：

```text
[BB_GET_USER_QUALITY]
user[8]: snr_raw=1949 snr_db=17.34 dB ldpc=0/2 gain_a=32 gain_b=40

[BB_GET_PEER_QUALITY]
slot[0]: snr_raw=2716 snr_db=18.78 dB ldpc=0/96 gain_a=36 gain_b=34
```

##### 3.3.4 查询 MCS 和控制模式

查询实时 MCS：

```sh
./l4_link_monitor -M
```

正常输出示例：

```text
[BB_GET_MCS] slot=0
  TX: mcs_raw=2 mcs_real=0 theory_throughput=236 kbps
  RX: mcs_raw=11 mcs_real=9 theory_throughput=11493 kbps
```

查询 MCS 控制模式和频宽控制模式：

```sh
./l4_link_monitor -m -W
```

正常输出示例：

```text
[BB_GET_MCS_MODE]
slot=0 auto_mode=AUTO(1)

[BB_GET_BANDWIDTH_MODE]
slot=0 auto_mode=MANUAL(0)
```

##### 3.3.5 查询天线发射功率

AP 端：

```sh
./l4_link_monitor -P -u 8
```

正常输出示例：

```text
[BB_GET_CUR_POWER]
user[8]: power=15
```

DEV 端：

```sh
./l4_link_monitor -P
```

正常输出示例：

```text
[BB_GET_CUR_POWER]
user[0]: power=15
```

##### 3.3.6 查询信道信息

查询本机信道信息：

```sh
./l4_link_monitor -C
```

正常输出示例：

```text
[BB_GET_CHAN_INFO]
chan_num=32 auto_mode=1 acs_chan=26 work_chan=30
  chan[0]: freq=2400000 KHz power=-92 dbm
  chan[1]: freq=2411000 KHz power=-79 dbm
  chan[2]: freq=2422000 KHz power=-73 dbm
  chan[3]: freq=2433000 KHz power=-85 dbm
  chan[4]: freq=2444000 KHz power=-79 dbm
  chan[5]: freq=2455000 KHz power=-78 dbm
  chan[6]: freq=2466000 KHz power=-71 dbm
  chan[7]: freq=2477000 KHz power=-88 dbm
  chan[8]: freq=5110000 KHz power=-95 dbm
  chan[9]: freq=5155000 KHz power=-98 dbm
  chan[10]: freq=5180000 KHz power=-98 dbm
  chan[11]: freq=5225000 KHz power=-80 dbm
  chan[12]: freq=5250000 KHz power=-80 dbm
  chan[13]: freq=5270000 KHz power=-101 dbm
  chan[14]: freq=5295000 KHz power=-100 dbm
  chan[15]: freq=5315000 KHz power=-101 dbm
  chan[16]: freq=5350000 KHz power=-100 dbm
  chan[17]: freq=5380000 KHz power=-98 dbm
  chan[18]: freq=5410000 KHz power=-96 dbm
  chan[19]: freq=5480000 KHz power=-99 dbm
  chan[20]: freq=5495000 KHz power=-99 dbm
  chan[21]: freq=5515000 KHz power=-100 dbm
  chan[22]: freq=5530000 KHz power=-99 dbm
  chan[23]: freq=5555000 KHz power=-98 dbm
  chan[24]: freq=5575000 KHz power=-94 dbm
  chan[25]: freq=5595000 KHz power=-95 dbm
  chan[26]: freq=5730000 KHz power=-100 dbm
  chan[27]: freq=5750000 KHz power=-100 dbm
  chan[28]: freq=5780000 KHz power=-100 dbm
  chan[29]: freq=5800000 KHz power=-100 dbm
  chan[30]: freq=5820000 KHz power=-100 dbm
  chan[31]: freq=5840000 KHz power=-100 dbm
```

查询对端信息时，增加 `-R`，并通过 `-s <slot>` 指定对端所在 slot。例如 AP 侧查询 slot 0 的 DEV 信道信息：

```sh
./l4_link_monitor -C -R -s 0
```

输出标题会显示远程 slot：

```text
[BB_GET_CHAN_INFO remote slot=0]
chan_num=32 auto_mode=1 acs_chan=26 work_chan=30
  chan[0]: freq=2400000 KHz power=-92 dbm
  ...
```

`-R` 会将查询命令封装进 `BB_REMOTE_IOCTL_REQ`，由 `-s` 指定的对端 slot 执行查询。不带 `-R` 时仍查询本机信息。`-R` 支持 `-A` 和所有单项查询参数，包括 `-S`、`-Q`、`-q`、`-M`、`-m`、`-W`、`-P`、`-C`、`-B`、`-T`、`-D`、`-V`。链路细节查询会先读取实际查询端的 `BB_GET_STATUS` 做 pair/connect 检查；MCS 模式和频宽模式查询不受该检查限制。

##### 3.3.7 查询频段信息

```sh
./l4_link_monitor -B
```

正常输出示例：

```text
[BB_GET_BAND_INFO]
band_mode=AUTO(1) work_band=5G(2)
```

##### 3.3.8 查询传输带宽

```sh
./l4_link_monitor -T
```

正常输出示例：

```text
[BB_GET_THROUGHPUT]
slot=0
  TX: phy=232960 real=26112
  RX: phy=11471040 real=53664
```

##### 3.3.9 查询测距结果

```sh
./l4_link_monitor -D -s 0
```

正常输出示例：

```text
[BB_GET_DISTC_RESULT]
slot=0
  slot[0]: distance=123
```

`-D` 会调用 `BB_GET_DISTC_RESULT`，通过 `-s <slot>` 指定读取的 slot，并输出 SDK 返回的 `distance[slot]`。当 `distance=-1` 时，表示当前没有测距结果。

### 4、链路设置示例

`l4_link_config` 用于设置图传链路的频段、信道、频宽、MCS、功率和帧结构。它通常与 `l4_link_monitor` 配套使用：先读取当前链路状态，再下发配置。

#### 4.1 程序参数说明


| 参数               | 说明                                               | 默认值  |
| ---------------- | ------------------------------------------------ | ---- |
| `-B <0/1>`       | 调用 `BB_SET_BAND_MODE`，`1` 表示自动，`0` 表示手动          | 不执行  |
| `-b <band>`      | 调用 `BB_SET_BAND`，支持 `1g`、`2g`、`5g` 或 `0`、`1`、`2` | 不执行  |
| `-C <0/1>`       | 调用 `BB_SET_CHAN_MODE`，`1` 表示自动，`0` 表示手动          | 不执行  |
| `-c <index>`     | 调用 `BB_SET_CHAN`，设置信道索引                          | 不执行  |
| `-d <dir>`       | `-c` 和 `-w` 使用的方向，支持 `tx`、`rx` 或 `0`、`1`         | `rx` |
| `-W <0/1>`       | 调用 `BB_SET_BANDWIDTH_MODE`，`1` 表示自动，`0` 表示手动     | 不执行  |
| `-w <bandwidth>` | 调用 `BB_SET_BANDWIDTH`，范围 `0-5`                   | 不执行  |
| `-M <0/1>`       | 调用 `BB_SET_MCS_MODE`，`1` 表示自动，`0` 表示手动           | 不执行  |
| `-m <mcs>`       | 调用 `BB_SET_MCS`，范围 `0-24`                        | 不执行  |
| `-O <0/1>`       | 调用 `BB_SET_POWER_AUTO`，`1` 表示自适应，`0` 表示手动       | 不执行  |
| `-P <power>`     | 调用 `BB_SET_POWER`，固定发射功率范围 `0-31 dBm`          | 不执行  |
| `-u <user>`      | `BB_SET_POWER` 使用的物理用户                              | 按角色默认 |
| `-F <0/1>`       | 调用 `BB_SET_FRAME_CHANGE`，仅 `SINGLE_USER` 模式支持    | 不执行  |
| `-s <slot>`      | slot 编号，用于频宽和 MCS 配置；带 `-R` 时也是远程对端 slot      | `0`  |


如果没有指定任何设置动作，程序只打印帮助并返回错误，避免误操作。

#### 4.2 标准使用方法

1. 启动 `l4_daemon`。
2. 使用 `l4_link_monitor` 查看当前角色、模式、slot 和链路状态。
3. 根据需求设置自动或手动模式。
4. 下发目标频段、信道、频宽或 MCS。
5. 设置后再次使用 `l4_link_monitor` 验证结果。

频段、信道、频宽、MCS 和功率的设置命令可组合使用。程序会按固定顺序执行：频段模式、频段、信道模式、信道、频宽模式、频宽、MCS 模式、MCS、功率自适应、固定功率、帧结构。

成功执行的命令会按 `l4_link_monitor` 风格打印命令字标题和参数详情，例如：

```text
[BB_SET_BAND_MODE]
auto_mode=0

[BB_SET_BAND]
target_band=5G(2)
```

如果 `bb_ioctl()` 返回非 0，会打印 `BB_SET_xxx failed, ret=<ret>` 并停止后续设置。

#### 4.3 示例

##### 4.3.1 手动设置频段为 2G

```sh
./l4_link_config -B 0 -b 2g
```

正常输出示例：

```text
[BB_SET_BAND_MODE]
auto_mode=0

[BB_SET_BAND]
target_band=2G(1)
```

设置完成后，可查询当前频段确认结果：

```sh
./l4_link_monitor -B
```

正常输出示例：

```text
[BB_GET_BAND_INFO]
band_mode=MANUAL(0) work_band=2G(1)
```

##### 4.3.2 手动设置信道索引为 0（2400 MHz）

手动设置信道前，应先将频段固定到目标频段。本例前一步已将频段设置为 2G，因此信道索引 0 对应信道信息中的 `2400000 KHz`，也就是 2400 MHz。

```sh
./l4_link_config -C 0 -c 0
```

正常输出示例：

```text
[BB_SET_CHAN_MODE]
auto_mode=0

[BB_SET_CHAN]
dir=RX(1) chan_index=0
```

设置完成后，可查询信道信息确认 `work_chan` 和频点映射：

```sh
./l4_link_monitor -C
```

正常输出示例：

```text
[BB_GET_CHAN_INFO]
chan_num=32 auto_mode=0 acs_chan=26 work_chan=0
  chan[0]: freq=2400000 KHz power=-74 dbm
  chan[1]: freq=2411000 KHz power=-71 dbm
  chan[2]: freq=2422000 KHz power=-78 dbm
  ...
  chan[8]: freq=5110000 KHz power=-99 dbm
  ...
```

其中 `work_chan=0` 表示当前工作信道索引为 0，`chan[0]` 对应 `2400000 KHz`。完整信道索引和频点映射可参考 `3.3.6 查询信道信息` 的 `BB_GET_CHAN_INFO` 输出。

切换到 5G 频点时，也需要先将频段设置为 5G，例如要使用 `5110000 KHz`，应先固定到 5G 频段，再选择对应的信道索引。

##### 4.3.3 设置自动跳频

该命令同时打开频段自动模式和信道自动模式，由设备自动选择工作频段和信道。

```sh
./l4_link_config -B 1 -C 1
```

正常输出示例：

```text
[BB_SET_BAND_MODE]
auto_mode=1

[BB_SET_CHAN_MODE]
auto_mode=1
```

设置完成后，可查询频段和信道信息确认当前自动模式：

```sh
./l4_link_monitor -B
./l4_link_monitor -C
```

##### 4.3.4 设置 RX 方向频宽为 20 MHz

`-W 0` 表示使用手动频宽模式，`-w 4` 对应 `BB_BW_20M`，即 20 MHz。未指定 `-s` 时默认设置 slot 0。

```sh
./l4_link_config -W 0 -d rx -w 4
```

正常输出示例：

```text
[BB_SET_BANDWIDTH_MODE]
slot=0 mode=0

[BB_SET_BANDWIDTH]
slot=0 dir=RX(1) bandwidth=20M(4)
```

设置完成后，可查询设备状态，确认 `user phy status` 中 RX 方向的 `bw=20MHz(4)`：

```sh
./l4_link_monitor -S
```

```text
[BB_GET_STATUS]
...

user phy status:
  RX: user=0 source=rx freq=2400000KHz bw=20MHz(4) tintlv_enable=1 tintlv_len=3 tintlv_num=1
  TX: user=8 source=tx freq=5105625KHz mcs_raw=2 mcs_real=0 bw=10MHz(3) tintlv_enable=1 tintlv_len=3 tintlv_num=0
  bw_mode=Y24X2 major_dir=DEV->AP
```

##### 4.3.5 设置图传调制方式 MCS

`-M 0` 表示使用手动 MCS 模式，`-m 0` 表示将目标 MCS 设置为 0。未指定 `-s` 时默认设置 slot 0。

```sh
./l4_link_config -M 0 -m 0
```

正常输出示例：

```text
[BB_SET_MCS_MODE]
slot=0 auto_mode=0

[BB_SET_MCS]
slot=0 mcs=0
```

设置完成后，可查询设备状态，确认 `slot link status` 中 `rx_mcs_raw=0`：

```sh
./l4_link_monitor -S
```

```text
[BB_GET_STATUS]
...

slot link status:
  slot[0]: state=CONNECT(2), rx_mcs_raw=0, rx_mcs_real=-2, pair=0, peer_mac=65:a6:78:c7

...
```

##### 4.3.6 设置发射功率

开启功率自适应：

```sh
./l4_link_config -O 1
```

设置固定功率前，建议显式关闭功率自适应。本例将默认物理用户的发射功率设置为 `20 dBm`：

```sh
./l4_link_config -O 0 -P 20
```

指定物理用户时使用 `-u <user>`。带 `-R` 时，`-O` 和 `-P` 都通过远程 ioctl 在对端执行：

```sh
./l4_link_config -R -s 0 -O 0 -P 20
```

##### 4.3.7 帧结构切换

默认大带宽方向为 `DEV->AP`。如果需要从 AP 向 DEV 发送大流量数据，例如 AP 上传文件到 DEV，可执行帧结构切换，将大带宽方向切到 `AP->DEV`。

`BB_SET_FRAME_CHANGE` 仅支持 `SINGLE_USER` 模式。示例程序会先读取 `BB_GET_STATUS`，如果当前不是 `SINGLE_USER` 模式，则不会下发帧结构切换命令。

执行帧结构切换：

```sh
./l4_link_config -F 1
```

正常输出示例：

```text
[BB_SET_FRAME_CHANGE]
mode=1
```

设置完成后，可查询设备状态，确认 `major_dir=AP->DEV`：

```sh
./l4_link_monitor -S
```

```text
[BB_GET_STATUS]
...

  bw_mode=Y12X1 major_dir=AP->DEV
```

恢复原始帧结构：

```sh
./l4_link_config -F 0
```

正常输出示例：

```text
[BB_SET_FRAME_CHANGE]
mode=0
```

设置完成后，可查询设备状态，确认 `major_dir=DEV->AP`：

```sh
./l4_link_monitor -S
```

```text
[BB_GET_STATUS]
...

  bw_mode=Y24X2 major_dir=DEV->AP
```

### 5、配置文件管理示例

`l4_config_file` 用于导出、写入和恢复设备配置文件。该示例演示 `BB_GET_CFG`、`BB_SET_CFG` 和 `BB_RESET_CFG` 的分片调用流程，并支持通过 `BB_REMOTE_IOCTL_REQ` 操作对端设备，远程模式默认 slot 为 0。

#### 5.1 程序参数说明

| 参数 | 说明 | 默认值 |
| --- | --- | --- |
| `-g <file>` | 调用 `BB_GET_CFG`，导出配置到本地文件 | 不执行 |
| `-s <file>` | 调用 `BB_SET_CFG`，把本地文件写入设备 | 不执行 |
| `-r` | 调用 `BB_RESET_CFG`，恢复设备配置 | 不执行 |
| `-R` | 通过远程 ioctl 操作对端设备 | 本机操作 |
| `-S <slot>` | 指定 `-R` 使用的 remote slot | `0` |
| `-m <mode>` | `BB_GET_CFG` 读取模式，支持 `auto`、`memory`、`flash` 或 `0`、`1`、`2` | `auto` |

`-g`、`-s`、`-r` 三个动作必须且只能指定一个，避免一次命令同时执行多个配置文件操作。`-R` 只改变执行端，`-S` 只能和 `-R` 一起使用。

#### 5.2 示例

导出配置文件：

```sh
./l4_config_file -g cfg.json
```

从 flash 导出配置文件：

```sh
./l4_config_file -m flash -g cfg_flash.json
```

写入配置文件：

```sh
./l4_config_file -s cfg.json
```

恢复设备配置文件：

```sh
./l4_config_file -r
```

远程导出对端 slot 0 配置文件：

```sh
./l4_config_file -R -g peer_cfg.json
```

远程写入对端 slot 0 配置文件：

```sh
./l4_config_file -R -s cfg.json
```

远程恢复对端 slot 0 配置文件：

```sh
./l4_config_file -R -r
```

远程导出指定 slot 的 flash 配置文件：

```sh
./l4_config_file -R -S 1 -m flash -g peer_slot1_flash_cfg.json
```

导出时，程序会校验设备返回的 `total_length` 和 `total_crc16`；写入时，程序会先计算本地文件 CRC，再按 `bb_set_cfg_t.data` 最大长度分片下发。远程执行时标题会显示 `remote slot=<slot>`。


### 6、MiniDB 配置示例

`l4_minidb_config` 用于演示通过 `BB_SET_PRJ_DISPATCH` 和 `BB_GET_PRJ_DISPATCH` 读写设备 MiniDB 持久化配置，包括角色、AP MAC、slot MAC、频段、功率和扩展功率等配置项。增加 `--remote` 后，查询、写入、重置和重启会通过 `BB_REMOTE_IOCTL_REQ` 在对端执行，默认 remote slot 为 0。

MiniDB 修改的是持久化配置，不等同于当前运行态配置。修改 MiniDB 后，配置需要重启设备后才生效；建议设置或重置后使用 `-H` 重启设备，重启完成后再查询确认。

#### 6.1 程序参数说明

| 参数 | 长参数 | 说明 | 默认值 |
| --- | --- | --- | --- |
| `-A` | `--get-all` | 查询 role、AP MAC、slot 0 MAC、band、power、power ex 和 freq list | 不执行 |
| `-r` | `--get-role` | 查询 MiniDB 中保存的 role | 不执行 |
| `-m` | `--get-ap-mac` | 查询 MiniDB 中保存的 AP MAC | 不执行 |
| `-s [slot]` | `--get-slot-mac[=slot]` | 查询 MiniDB 中保存的 slot MAC | slot 0 |
| `-b` | `--get-band` | 查询 MiniDB 中保存的 band bitmap | 不执行 |
| `-w` | `--get-pwr` | 查询 MiniDB 中保存的 power 配置 | 不执行 |
| 无 | `--get-pwr-ex` | 查询 MiniDB 中保存的扩展功率配置 | 不执行 |
| `-u` | `--get-uart-baudrate` | 查询 MiniDB 中保存的 UART2 波特率 | 不执行 |
| `-R <ap|dev|0|1>` | `--set-role <...>` | 设置 MiniDB role | 不执行 |
| `-M <mac>` | `--set-ap-mac <mac>` | 设置 MiniDB AP MAC | 不执行 |
| `-S <mac|slot,mac>` | `--set-slot-mac <mac|slot,mac>` | 设置 MiniDB slot MAC | slot 0 |
| `-B <auto|2g|5g|bitmap>` | `--set-band <...>` | 设置 MiniDB band bitmap | 不执行 |
| `-W <pwr|min,max>` | `--set-pwr <...>` | 设置 MiniDB 固定功率或功率自适应区间 | 不执行 |
| `-U <baudrate>` | `--set-uart-baudrate <baudrate>` | 设置 MiniDB UART2 波特率 | 不执行 |
| `-D` | `--reset` | 重置 MiniDB | 不执行 |
| `-H` | `--reboot` | 单独重启设备，或设置/重置成功后重启设备 | 不执行 |
| 无 | `--remote` | 通过远程 ioctl 操作对端设备 | 本机操作 |
| 无 | `--remote-slot <slot>` | 指定 `--remote` 使用的 remote slot | `0` |

每次运行只允许一个主动作，例如不能同时指定 `-A` 和 `-B auto`。`--remote` 只改变命令实际执行端，`--remote-slot` 只能和 `--remote` 一起使用；不新增远程短参数，因为 `-R` 已是 `--set-role`，`-S` 已是 `--set-slot-mac`。`-H` 可以单独作为重启动作，也可以附加在设置或重置动作后；`-H` 不能和查询动作组合。带 `--remote` 时，查询、写入、重置和重启都在对端执行。

#### 6.2 输入格式

MAC 支持冒号分隔或连续十六进制两种格式。当前 SDK 中 `BB_MAC_LEN = 4`，因此 AP MAC 和 slot MAC 都按 4 字节输入：

```text
11:22:33:44
11223344
```

band bitmap 支持名称和数值：

| 输入 | bitmap | 说明 |
| --- | --- | --- |
| `auto` | `0x07` | 自动频段配置 |
| `2g` | `0x02` | 仅 2G |
| `5g` | `0x04` | 仅 5G |
| `0x07`、`0x02`、`0x04` 等数值 | 输入值 | 自定义 bitmap |

功率设置范围为 `10-27`：

| 输入 | 行为 |
| --- | --- |
| `-W 20` | 设置固定功率，并同步写入扩展功率的手动初始化和重置功率 |
| `-W 10,27` | 设置自适应功率区间，写入 `pwr_auto=1`、`pwr_min=10`、`pwr_max=27` |

固定功率模式下，`-W 20` 会先通过 `PRJ_CMD_SET_PWR` 写入 `pwr_init=20`，再通过 `PRJ_CMD_SET_PWR_EX` 写入 `link_auto=0`、`auto_reset=0`、`manu_init=20`、`manu_reset=20`、`csma_offset=0`。自适应区间格式 `-W min,max` 只更新基础功率配置，不修改扩展功率配置。扩展功率可通过 `--get-pwr-ex` 查询；设备禁用或从未配置时显示 `disabled/unset`。

UART 波特率配置固定操作 UART2。`-U <baudrate>` 只修改 MiniDB 中 UART2 的波特率，保留已有数据位、校验位、停止位和 RX buffer 配置；如果 UART2 尚未配置，则使用 `8N1` 和默认 RX buffer 创建配置。

#### 6.3 示例

以下示例按“先设置，再查询确认”的顺序组织。由于程序每次运行只允许一个主动作，设置和查询需要分两次执行；如果需要确认设备运行态是否生效，设置后先使用 `-H` 重启设备，重启完成后再查询。远程操作时在命令中增加 `--remote`，非默认对端使用 `--remote-slot <slot>`。

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

远程操作指定 remote slot，并在写入成功后重启对端：

```sh
./l4_minidb_config --remote --remote-slot 1 -B auto -H
```

##### 6.3.1 设置 role 为 AP 后查询

将 MiniDB role 设置为 AP：

```sh
./l4_minidb_config -R ap
```

正常调用示例：

```text
[PRJ_CMD_SET_ROLE]
role=AP(0)
set minidb role ok
```

查询确认 MiniDB 中保存的设备角色：

```sh
./l4_minidb_config -r
```

正常调用示例：

```text
[PRJ_CMD_GET_ROLE]
role=AP(0)
```

##### 6.3.2 设置 role 为 DEV 后查询

将 MiniDB role 设置为 DEV：

```sh
./l4_minidb_config -R dev
```

正常调用示例：

```text
[PRJ_CMD_SET_ROLE]
role=DEV(1)
set minidb role ok
```

查询确认 MiniDB 中保存的设备角色：

```sh
./l4_minidb_config -r
```

正常调用示例：

```text
[PRJ_CMD_GET_ROLE]
role=DEV(1)
```

##### 6.3.3 设置 AP MAC 后查询

写入 MiniDB AP MAC：

```sh
./l4_minidb_config -M 11:22:33:44
```

正常调用示例：

```text
[PRJ_CMD_SET_AP_MAC]
ap_mac=11:22:33:44
set minidb AP MAC ok
```

查询确认 MiniDB 中保存的 AP MAC：

```sh
./l4_minidb_config -m
```

正常调用示例：

```text
[PRJ_CMD_GET_AP_MAC]
ap_mac=11:22:33:44
```

##### 6.3.4 设置默认 slot MAC 后查询

不指定 slot 时，默认写入 slot 0 MAC：

```sh
./l4_minidb_config -S 11:22:33:44
```

正常调用示例：

```text
[PRJ_CMD_SET_SLOT_MAC]
slot=0 mac=11:22:33:44
set minidb slot MAC ok
```

查询确认 slot 0 MAC：

```sh
./l4_minidb_config -s
```

正常调用示例：

```text
[PRJ_CMD_GET_SLOT_MAC]
slot=0 mac=11:22:33:44
```

##### 6.3.5 设置指定 slot MAC 后查询

写入 slot 1 MAC：

```sh
./l4_minidb_config -S 1,11:22:33:44
```

正常调用示例：

```text
[PRJ_CMD_SET_SLOT_MAC]
slot=1 mac=11:22:33:44
set minidb slot MAC ok
```

查询确认 slot 1 MAC：

```sh
./l4_minidb_config -s 1
```

正常调用示例：

```text
[PRJ_CMD_GET_SLOT_MAC]
slot=1 mac=11:22:33:44
```

##### 6.3.6 设置 band 为自动后查询

将 MiniDB band bitmap 设置为 `auto(0x07)`：

```sh
./l4_minidb_config -B auto
```

正常调用示例：

```text
[PRJ_CMD_SET_BAND]
band=auto(0x07)
set minidb band ok
```

查询确认 MiniDB 中保存的频段 bitmap：

```sh
./l4_minidb_config -b
```

正常调用示例：

```text
[PRJ_CMD_GET_BAND]
band=auto(0x07)
```

##### 6.3.7 设置 band 为 2G 后查询

将 MiniDB band bitmap 设置为 `2g(0x02)`：

```sh
./l4_minidb_config -B 2g
```

正常调用示例：

```text
[PRJ_CMD_SET_BAND]
band=2g(0x02)
set minidb band ok
```

查询确认 MiniDB 中保存的频段 bitmap：

```sh
./l4_minidb_config -b
```

正常调用示例：

```text
[PRJ_CMD_GET_BAND]
band=2g(0x02)
```

##### 6.3.8 设置 band 为 5G 后查询

将 MiniDB band bitmap 设置为 `5g(0x04)`：

```sh
./l4_minidb_config -B 5g
```

正常调用示例：

```text
[PRJ_CMD_SET_BAND]
band=5g(0x04)
set minidb band ok
```

查询确认 MiniDB 中保存的频段 bitmap：

```sh
./l4_minidb_config -b
```

正常调用示例：

```text
[PRJ_CMD_GET_BAND]
band=5g(0x04)
```

##### 6.3.9 设置固定功率后查询

将 MiniDB power 设置为固定功率 20 dBm：

```sh
./l4_minidb_config -W 20
```

正常调用示例：

```text
[PRJ_CMD_SET_PWR]
pwr_auto=off(0)
pwr_init=20 dBm
set minidb power ok
[PRJ_CMD_SET_PWR_EX]
link_auto=0
auto_reset=0 dBm
manu_init=20 dBm
manu_reset=20 dBm
csma_offset=0
set minidb extended power configuration ok
```

查询确认 MiniDB 中保存的功率配置：

```sh
./l4_minidb_config -w
```

正常调用示例：

```text
[PRJ_CMD_GET_PWR]
pwr_auto=off(0)
pwr_init=20 dBm
```

查询确认同步写入的扩展功率配置：

```sh
./l4_minidb_config --get-pwr-ex
```

正常调用示例：

```text
[PRJ_CMD_GET_PWR_EX]
link_auto=0
auto_reset=0 dBm
manu_init=20 dBm
manu_reset=20 dBm
csma_offset=0
```

##### 6.3.10 设置功率自适应区间后查询

将 MiniDB power 设置为自适应功率区间 10-27 dBm：

```sh
./l4_minidb_config -W 10,27
```

正常调用示例：

```text
[PRJ_CMD_SET_PWR]
pwr_auto=on(1)
pwr_min=10 dBm
pwr_max=27 dBm
set minidb power ok
```

查询确认 MiniDB 中保存的功率配置：

```sh
./l4_minidb_config -w
```

正常调用示例：

```text
[PRJ_CMD_GET_PWR]
pwr_auto=on(1)
pwr_min=10 dBm
pwr_max=27 dBm
```

##### 6.3.11 重置 MiniDB 后查询

清除 MiniDB 持久化配置：

```sh
./l4_minidb_config -D
```

正常调用示例：

```text
[PRJ_CMD_SET_RESET_DB]
reset minidb ok
```

查询确认 role、AP MAC、slot 0 MAC、band、power、扩展功率和频点列表：

```sh
./l4_minidb_config -A
```

正常调用示例：

```text
[MiniDB]
role     : unset
ap_mac   : unset
slot_mac0: unset
band     : unset
power    : unset
power_ex : disabled/unset
freq_list: unset
```

##### 6.3.12 设置后查询全部 MiniDB 信息

完成所需设置后，可查询 role、AP MAC、slot 0 MAC、band、power、扩展功率和频点列表：

```sh
./l4_minidb_config -A
```

正常调用示例：

```text
[MiniDB]
role     : AP(0)
ap_mac   : 11:22:33:44
slot_mac0: 11:22:33:44
band     : auto(0x07)
power    : fixed init=20 dBm
power_ex : disabled/unset
freq_list: unset
```

##### 6.3.13 设置后自动重启设备并查询

`-H` 可以附加在设置动作后。下面示例先设置 band 为自动，再请求设备重启，使 MiniDB 修改生效：

```sh
./l4_minidb_config -B auto -H
```

正常调用示例：

```text
[PRJ_CMD_SET_BAND]
band=auto(0x07)
set minidb band ok
[BB_SET_SYS_REBOOT]
reboot requested
```

设备重启完成后，再查询确认 MiniDB 中保存的频段 bitmap：

```sh
./l4_minidb_config -b
```

正常调用示例：

```text
[PRJ_CMD_GET_BAND]
band=auto(0x07)
```

##### 6.3.14 单独重启设备

不修改 MiniDB，只请求设备重启：

```sh
./l4_minidb_config -H
```

正常调用示例：

```text
[BB_SET_SYS_REBOOT]
reboot requested
```

##### 6.3.15 设置 UART2 波特率后查询

将 MiniDB 中 UART2 波特率设置为 `115200`：

```sh
./l4_minidb_config -U 115200
```

正常调用示例：

```text
[PRJ_CMD_SET_UART]
id=2
uart2_baudrate=115200
set minidb UART2 baudrate ok
```

查询确认 MiniDB 中保存的 UART2 波特率：

```sh
./l4_minidb_config -u
```

正常调用示例：

```text
[PRJ_CMD_GET_UART]
id=2
uart2_baudrate=115200
```

### 7、UART 配置示例

`l4_uart_config` 用于演示通过 `PRJ_CMD_SET_UART` 和 `PRJ_CMD_GET_UART` 设置、查询设备 UART 配置。该示例使用 SDK 统一例程参数风格，`-a/-p/-i` 用于连接 daemon 和选择设备，UART id 由 `--get-uart` 或 `--set-uart` 指定。

#### 7.1 程序参数说明

公共参数：

| 参数 | 长参数 | 说明 | 默认值 |
| --- | --- | --- | --- |
| `-h` | `--help` | 打印帮助 | 无 |
| `-a <addr>` | `--addr <addr>` | daemon 地址 | `127.0.0.1` |
| `-p <port>` | `--port <port>` | daemon 端口 | `BB_PORT_DEFAULT` |
| `-i <index>` | `--index <index>` | 设备序号 | `0` |

查询参数：

| 参数 | 长参数 | 说明 |
| --- | --- | --- |
| `-g <id>` | `--get-uart <id>` | 查询指定 UART 的 running 配置 |

设置参数：

| 参数 | 长参数 | 说明 | 默认值 |
| --- | --- | --- | --- |
| `-U <id>` | `--set-uart <id>` | 设置指定 UART | 无 |
| `-b <rate>` | `--baudrate <rate>` | UART 波特率 | `115200` |
| `-d <5-8>` | `--data-bit <5-8>` | UART 数据位 | `8` |
| `-P <parity>` | `--parity <parity>` | UART 校验位，支持 `none/even/odd/0/1/2` | `none` |
| `-T <1-3>` | `--stop-bit <1-3>` | UART 停止位协议值，`1:1bit / 2:1.5bits / 3:2bits` | `1` |
| `-r <n>` | `--rx-buf-size <n>` | UART RX buffer 大小，`0` 表示设备默认值 | `0` |
| `-A` | `--apply` | 设置后请求立即应用到运行态 | 不执行 |

其它参数：

| 参数 | 长参数 | 说明 |
| --- | --- | --- |
| `-s <slot>` | `--slot <slot>` | 通过 remote ioctl slot 执行命令 |

每次运行只允许一个主动作：`--get-uart` 或 `--set-uart` 二选一。设置动作不会自动查询，设置后如需确认，请再次执行 `--get-uart`。

#### 7.2 示例

只查询 UART 1：

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

### 8、Socket 收发示例

`l4_socket_transfer` 用于演示打开设备的 socket port，并发送指定文本或十六进制字节，也可以持续接收 socket 数据直到 `Ctrl-C` 退出。运行前请先启动 `l4_daemon`，并确认 AP/DEV 已完成连接。

示例支持 `BB_SOCK_ENCRYPT_MODE_DEFAULT`、AES128 和 AES256。收发两端必须使用相同模式和密钥，密钥不会打印到日志。运行中动态更新加密参数可使用 `bb_socket_ioctl_enc_set()` / `bb_socket_ioctl_enc_get()`，或者通过 `BB_SET_FORCE_UPD_SOCKET_ENC` 调用 `bb_ioctl()`；本示例只演示创建 Socket 时设置加密参数。

#### 8.1 程序参数说明

公共参数：

| 参数 | 长参数 | 说明 | 默认值 |
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

| 参数 | 长参数 | 说明 |
| --- | --- | --- |
| `-t <text>` | `--text <text>` | 发送文本字节一次，发送完成后等待 1 秒关闭 socket |
| `-T` | `--text-input` | 文本输入模式，持续读取用户输入的文本行并发送 |
| `-x <hex>` | `--hex <hex>` | 发送十六进制字节一次，发送完成后等待 1 秒关闭 socket |
| `-X` | `--hex-input` | 十六进制输入模式，持续读取用户输入的十六进制行并发送 |
| `-r` | `--recv` | 持续接收数据直到 `Ctrl-C` |

每次运行必须且只能指定一个主动作。`--hex` 和 `--hex-input` 要求每个字节使用两个十六进制字符，例如 `0a`，支持 `01020aff`、`01:02:0a:ff` 或 `01 02 0a ff`。输入模式期间空行会被跳过；文本输入模式按原始文本字节发送，十六进制输入模式按十六进制字节发送。

`default` 模式不使用 `--encrypt-key`。`aes128` 和 `aes256` 必须分别提供 16 字节和 32 字节密钥；仅指定 key 而未指定 `--encrypt-mode` 会被拒绝。

如果命令行出现 `dquote>`，表示 shell 检测到双引号没有闭合，程序尚未启动。请补齐结尾双引号，或按 `Ctrl-C` 取消后重新输入，例如：

```sh
./l4_socket_transfer -s 1 -P 1 -t "test"
```

#### 8.2 示例

接收 socket port 1 上的数据：

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

使用 AES128 加密发送文本：

```sh
./l4_socket_transfer -s 0 -P 1 --encrypt-mode aes128 \
  --encrypt-key 0102030405060708090a0b0c0d0e0f10 \
  --text "hello l4"
```

进入十六进制输入模式，每输入一行并按回车发送一次，按 `Ctrl-C` 关闭：

```sh
./l4_socket_transfer -s 0 -P 1 --hex-input
```

## 四、高级构建

通常优先使用 `script/` 下的脚本。只有需要手动控制 CMake 参数、只构建部分 target、或排查交叉编译配置时，才建议直接执行 CMake 命令。

### 1、常用 CMake 选项

| 选项 | 默认值 | 说明 |
|-|-|-|
| `CMAKE_INSTALL_PREFIX` | 脚本按架构设置 | 安装输出目录。 |
| `CMAKE_BUILD_TYPE` | `RelWithDebInfo` | 构建类型。 |
| `CMAKE_TOOLCHAIN_FILE` | arm64 构建使用 `compiler.arm.cmake` | 交叉编译工具链文件。 |
| `APP_STATIC_LIB` | `OFF` | `OFF` 时生成 `libar8030_client.so`。 |
| `USING_8030USB` | `ON` | 启用 USB 后端。 |
| `USING_8030UART` | `ON` | 启用 UART 后端。 |
| `USING_8030SDIO` | `OFF` | SDIO 后端当前默认关闭。 |
| `USING_XDS_HDR` | `ON` | 启用 XDS 包头处理。 |

CMake target 名为 `ar8030_client`，默认动态库产物为 `libar8030_client.so`。

### 2、x86_64 手动构建

```sh
cmake -S L4_Linux_SDK -B L4_Linux_SDK/build/x86_64 \
  -DCMAKE_INSTALL_PREFIX=L4_Linux_SDK/install/x86_64 \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DAPP_STATIC_LIB=OFF \
  -DUSING_8030USB=ON \
  -DUSING_8030UART=ON \
  -DUSING_8030SDIO=OFF \
  -DUSING_XDS_HDR=ON

cmake --build L4_Linux_SDK/build/x86_64 --target \
  ar8030_client \
  l4_cmd_dbg \
  l4_tuntap \
  l4_ota_upgrade \
  l4_basic_info \
  l4_pair_manager \
  l4_link_monitor \
  l4_link_config \
  l4_config_file \
  l4_minidb_config \
  l4_uart_config \
  l4_socket_transfer \
  l4_daemon \
  -j

cmake --install L4_Linux_SDK/build/x86_64
```

### 3、arm64 手动构建

```sh
cmake -S L4_Linux_SDK -B L4_Linux_SDK/build/arm64 \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_INSTALL_PREFIX=L4_Linux_SDK/install/arm64 \
  -DCMAKE_TOOLCHAIN_FILE=L4_Linux_SDK/compiler.arm.cmake \
  -DAPP_STATIC_LIB=OFF \
  -DUSING_8030USB=ON \
  -DUSING_8030UART=ON \
  -DUSING_8030SDIO=OFF \
  -DUSING_XDS_HDR=ON

cmake --build L4_Linux_SDK/build/arm64 --target \
  ar8030_client \
  l4_cmd_dbg \
  l4_tuntap \
  l4_ota_upgrade \
  l4_basic_info \
  l4_pair_manager \
  l4_link_monitor \
  l4_link_config \
  l4_config_file \
  l4_minidb_config \
  l4_uart_config \
  l4_socket_transfer \
  l4_daemon \
  -j

cmake --install L4_Linux_SDK/build/arm64
```

## 五、常见问题

### 1、程序提示没有设备

检查以下项目：

- `l4_daemon` 是否已启动。
- 8030 设备是否已通过 USB、UART 或其它接口连接。
- 当前用户是否有访问设备节点的权限。
- daemon 日志中是否有枚举失败或接口初始化失败信息。

### 2、连接 daemon 失败

确认 daemon 正在运行，并且示例程序连接的地址和端口与 daemon 一致。本机默认使用 `127.0.0.1` 和 `BB_PORT_DEFAULT`。

### 3、链路质量、MCS 或吞吐没有输出

这些信息依赖 AP 和 DEV 已完成配对并建立链路。先使用：

```sh
./l4_pair_manager -m
./l4_link_monitor -S
```

确认目标 slot 已经处于连接状态。

### 4、链路设置命令失败

检查以下项目：

- 当前设备角色是否支持该设置。
- `-s` 指定的 slot 是否存在有效链路。
- `-d` 指定的方向是否符合当前场景。
- `-F` 是否只在 `SINGLE_USER` 模式下使用。
- 设置后是否需要重新查询状态确认设备侧实际生效结果。

### 5、需要查看完整 API 结构体

本文档只说明常用工具和示例的使用方式。完整 API 命令字、输入输出结构体和字段说明请参考：

```text
L4_Linux_SDK/com/bb_api.h
L4_Linux_SDK/com/prj_rpc.h
L4_Linux_SDK/app/ar8030/ar_net_api.h
```

### 6、常见问题速查

| 现象 | 优先检查 |
|-|-|
| `lsusb` 看不到设备 | L4-BOX 是否上电，USB 线是否正常，虚拟机是否已接管 USB，开发板是否处于 USB Host 模式。 |
| `l4_daemon` 启动失败 | 是否使用 `sudo`，文件是否有执行权限，设备是否被其他程序占用。 |
| `l4_basic_info` 提示找不到 `.so` | 是否执行 `export LD_LIBRARY_PATH=../lib:$LD_LIBRARY_PATH`，`libar8030_client.so` 是否与程序架构一致。 |
| 连接 daemon 失败 | `l4_daemon` 是否仍在运行，端口是否被占用，客户端是否连接默认地址 `127.0.0.1` 和默认端口。 |
| 提示没有设备 | `lsusb` 是否能看到 L4-BOX，`l4_daemon` 是否有设备访问权限。 |
| arm64 程序无法运行 | 是否误用了 `install/x86_64` 产物，开发板上应使用 `install/arm64` 产物。 |

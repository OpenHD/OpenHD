# 01_basic_info 基础信息查询说明

这份文档解释 `l4_basic_info` 这个示例程序做了什么，以及它是怎么通过 daemon 查询 8030 设备基础信息的。目标是让刚接触这个工程的人也能看懂。

## 1. 先理解几个概念

可以把 `l4_basic_info` 理解成一个最小的“连接设备并读取状态”的示例。

| 名词 | 可以这样理解 | 在代码里的含义 |
| --- | --- | --- |
| daemon | 后台服务程序 | PC 侧应用先连接 daemon，再通过 daemon 访问设备 |
| device | 8030 设备对象 | daemon 枚举到的可操作设备 |
| index | 设备序号 | 当 daemon 管理多个设备时，用 `-i` 选择其中一个 |
| sys info | 系统版本信息 | 通过 `BB_GET_SYS_INFO` 查询 |
| status | 当前运行状态 | 通过 `BB_GET_STATUS` 查询 |
| slot | 链路位置 | AP 侧可以有多个 slot，每个 slot 对应一个 DEV 链路 |
| user phy | 用户物理层状态 | 每个数据用户的收发频点、带宽、MCS 等状态 |

程序启动后会先连接 daemon，枚举当前设备列表，打开指定 index 的设备，然后按参数查询系统信息和运行状态。

## 2. 这个程序能做什么

`basic_info.c` 编译后生成可执行程序 `l4_basic_info`，它主要支持三类查询：

| 参数 | 作用 |
| --- | --- |
| `-S` | 查询 `BB_GET_STATUS`，打印角色、模式、MAC、slot 链路状态和 user phy 状态 |
| `-V` | 查询 `BB_GET_SYS_INFO`，打印运行时间、编译时间和版本信息 |
| `-A` | 查询全部基础信息，也就是同时执行 `-S` 和 `-V` |
| `-R` | 通过远程 ioctl 查询 `-s <slot>` 指定对端的基础信息；可配合 `-S`、`-V` 或 `-A` 使用 |

如果不带 `-S`、`-V` 或 `-A`，程序默认执行 `-A`。

其它常用参数：

| 参数 | 作用 | 默认值 |
| --- | --- | --- |
| `-a <addr>` | daemon 地址 | `127.0.0.1` |
| `-p <port>` | daemon 端口 | `BB_PORT_DEFAULT` |
| `-i <index>` | 选择第几个设备 | `0` |
| `-l` | 查看当前设备列表，不打开设备 | 无 |
| `-s <slot>` | `-R` 远程查询使用的对端 slot；DEV 侧 slot 0 表示 AP | `0` |
| `-h` | 打印帮助信息 | 无 |

## 3. 最常用的操作方式

### 查询全部基础信息

```sh
./l4_basic_info
```

这等价于：

```sh
./l4_basic_info -A
```

程序会先查询系统版本信息，再查询当前设备状态。

### 只查询设备状态

```sh
./l4_basic_info -S
```

这会调用 `BB_GET_STATUS`，用于查看当前设备是 AP 还是 DEV、当前工作模式、本机 MAC、slot 链路和 user phy 状态。

### 只查询系统版本信息

```sh
./l4_basic_info -V
```

这会调用 `BB_GET_SYS_INFO`，用于查看设备运行时间、编译时间、软件版本、硬件版本和固件版本。

### 远程查询对端基础信息

```sh
./l4_basic_info -R -s 0
```

`-R` 会把 `BB_GET_SYS_INFO` 和 `BB_GET_STATUS` 封装成 `BB_REMOTE_IOCTL_REQ`，通过 `-s <slot>` 指定的链路位置发给对端设备。不带 `-S`、`-V` 或 `-A` 时仍按默认动作查询全部基础信息。

也可以只查询对端某一类信息：

```sh
./l4_basic_info -R -s 0 -S
./l4_basic_info -R -s 0 -V
```

### 指定 daemon 地址和端口

```sh
./l4_basic_info -a 192.168.1.10 -p 9000
```

默认情况下程序连接 `127.0.0.1:BB_PORT_DEFAULT`。如果 daemon 跑在其它机器或其它端口，可以用 `-a` 和 `-p` 指定。

### 查看当前设备列表

```sh
./l4_basic_info -l
```

`-l` 只连接 daemon 并打印当前枚举到的设备列表，不会打开设备，也不会查询系统信息或状态。看到列表后，可以根据 `device[index]` 选择要打开的设备：

```sh
./l4_basic_info -i 1
```

### 指定设备 index

```sh
./l4_basic_info -i 1
```

如果 daemon 枚举到多个设备，可以用 `-i` 选择要打开的设备。默认打开 `device[0]`。

## 4. 程序内部做了什么

执行：

```sh
./l4_basic_info
```

程序内部大致按下面流程运行：

```text
解析命令行参数
    |
连接 daemon
    |
获取设备列表并打印每个设备的 MAC
    |
打开指定 index 的设备
    |
查询 BB_GET_SYS_INFO（本端；带 -R 时查询对端）
    |
查询 BB_GET_STATUS（本端；带 -R 时查询对端）
    |
关闭设备连接和 daemon 连接
```

如果执行 `./l4_basic_info -l`，流程会在打印设备列表后结束，不会进入打开设备和 ioctl 查询步骤。

### 第一步：连接 daemon 和打开设备

`main()` 里会调用：

```c
bb_demo_open(&ctx, addr, port, dev_index);
```

这个公共函数会做几件事：

```text
bb_host_connect()
    |
bb_dev_getlist()
    |
bb_dev_getinfo()
    |
bb_dev_open()
```

它会先连接 daemon，然后获取 daemon 当前管理的设备列表，打印每个设备的 MAC，最后打开指定 index 的设备。后续所有 `bb_ioctl()` 都通过这个设备句柄发给设备。

如果 `-i` 指定的 index 超出范围，程序会打印合法范围并退出。

### 第二步：查询系统信息

当执行默认查询、`-A` 或 `-V` 时，程序会进入 `query_sys_info()`：

```c
bb_ioctl(handle, BB_GET_SYS_INFO, NULL, &info);
```

输出示例：

```text
[BB_GET_SYS_INFO]
uptime       : 123456 ms
compile_time : 2026-01-01 12:00:00
soft_ver     : x.x.x
hardware_ver : x.x.x
firmware_ver : x.x.x
```

这些字段可以用来确认设备当前运行的固件和软件版本。

| 字段 | 含义 |
| --- | --- |
| `uptime` | 设备当前运行时间，单位毫秒 |
| `compile_time` | 固件或软件编译时间 |
| `soft_ver` | 软件版本 |
| `hardware_ver` | 硬件版本 |
| `firmware_ver` | 固件版本 |

### 第三步：查询设备状态

当执行默认查询、`-A` 或 `-S` 时，程序会进入 `query_status()`：

```c
input.user_bmp = 0xffff;
bb_ioctl(handle, BB_GET_STATUS, &input, &status);
```

这里 `user_bmp = 0xffff` 表示尽量查询所有用户状态。

输出内容主要分成三部分：

| 输出段 | 说明 |
| --- | --- |
| 基本状态 | 角色、模式、同步状态、slot bitmap、本机 MAC |
| slot link status | 每个非空 slot 的链路状态、MCS、对频状态和对端 MAC |
| user phy status | 按 AP/DEV 角色映射后的 RX/TX 频点、带宽、TX MCS 和时隙配比 |

## 5. 状态输出怎么看

### 角色和模式

状态查询会打印：

```text
role        : AP (1)
mode        : SINGLE_USER (0)
```

`role` 表示当前设备角色：

| 角色 | 含义 |
| --- | --- |
| `AP` | 基站、主机、接收端 |
| `DEV` | 终端、从机、发射端 |
| `UNKNOWN` | 示例代码暂时没有识别出的角色值 |

`mode` 表示当前工作模式：

| 模式 | 含义 |
| --- | --- |
| `SINGLE_USER` | 单用户模式 |
| `MULTI_USER` | 多用户模式 |
| `RELAY` | 中继模式 |
| `DIRECTOR` | director 模式 |
| `UNKNOWN` | 示例代码暂时没有识别出的模式值 |

### slot link status

AP 或 DEV 存在链路状态时，会打印类似：

```text
slot link status:
  slot[0]: state=CONNECT(2), rx_mcs_raw=4, rx_mcs_real=2, pair=1, peer_mac=xx:xx:xx:xx:xx:xx
```

常见链路状态：

| 状态 | 含义 |
| --- | --- |
| `IDLE` | 空闲 |
| `LOCK` | 已锁定 |
| `CONNECT` | 已连接 |
| `UNKNOWN` | 示例代码暂时没有识别出的状态值 |

程序会按当前工作模式打印 slot：`SINGLE_USER` 只打印 slot 0，其它模式遍历全部 slot；状态值不是 `IDLE`、`LOCK` 或 `CONNECT` 的 slot 会被跳过。
`rx_mcs_raw` 是 SDK 原始值，真实 MCS 按 `rx_mcs_raw - 2` 输出为 `rx_mcs_real`。

### user phy status

程序会按当前设备角色把物理 user 重新映射成逻辑 RX/TX 方向，只读取对应方向的对象：AP 的 RX 使用 `BB_USER_0`，TX 使用 `BB_USER_BR_CS`；DEV 的 RX 使用 `BB_USER_BR_CS`，TX 使用 `BB_USER_0`。

```text
user phy status:
  RX: user=0 source=rx freq=1400000KHz bw=20MHz(4) tintlv_enable=1 tintlv_len=3 tintlv_num=1
  TX: user=8 source=tx freq=1400000KHz mcs_raw=4 mcs_real=2 bw=20MHz(4) tintlv_enable=1 tintlv_len=3 tintlv_num=1
  bw_mode=Y24X2 major_dir=DEV->AP
```

查看 TX 状态时只参考 `tx` 对象，并输出 `mcs_raw` 和修正后的 `mcs_real=mcs_raw-2`。查看 RX 状态时只参考 `rx` 对象，RX 端物理 MCS 字段不作为有效 MCS 输出。`bw_mode` 和 `major_dir` 只根据 RX 行的 `tintlv_len/tintlv_num` 解释，并单独一行输出。

## 6. 新手最容易混淆的点

### `l4_basic_info` 不直接访问 USB 或 UART

这个示例程序连接的是 daemon。真正的 USB、UART 等设备通信由 daemon 和底层 SDK 负责。

### 默认执行的是完整查询

不带参数时，程序会同时执行 `BB_GET_SYS_INFO` 和 `BB_GET_STATUS`。如果只想看某一类信息，使用 `-V` 或 `-S`。

### `device index` 不是 slot

`-i <index>` 选择的是 daemon 枚举到的第几个物理设备。`-s <slot>` 只在 `-R` 远程查询时使用，表示通过哪个链路位置访问对端。两者不是一回事。

### 没有打印 slot 或 user 不一定是错误

示例代码会跳过空闲 slot 和没有频点信息的 user。设备未连接、未对频或当前没有数据链路时，相关列表可能为空。

## 7. 和源码函数的对应关系

| 函数 | 做的事情 |
| --- | --- |
| `main()` | 解析参数、打开设备、按参数执行系统信息和状态查询 |
| `query_sys_info()` | 调用 `BB_GET_SYS_INFO` 并打印版本信息；带 `-R` 时通过远程 ioctl 查询对端 |
| `query_status()` | 调用 `BB_GET_STATUS` 并打印角色、模式、链路和物理层状态；带 `-R` 时通过远程 ioctl 查询对端 |
| `basic_info_ioctl()` | 根据是否启用 `-R` 选择本地 `bb_ioctl()` 或 `BB_REMOTE_IOCTL_REQ` |
| `role_name()` | 把角色枚举值转换成可读字符串 |
| `mode_name()` | 把模式枚举值转换成可读字符串 |
| `link_state_name()` | 把链路状态枚举值转换成可读字符串 |
| `print_mac()` | 按 `xx:xx:xx:xx:xx:xx` 格式打印 MAC |
| `list_devices()` | 连接 daemon、获取设备列表并打印设备信息，不打开设备 |
| `bb_demo_open()` | 连接 daemon、枚举设备、打开指定设备 |
| `bb_demo_close()` | 关闭设备、释放设备列表、断开 daemon |

## 8. 一句话总结

`l4_basic_info` 做的事情就是：连接 daemon，打开指定 8030 设备，然后用 `BB_GET_SYS_INFO` 查询版本信息，用 `BB_GET_STATUS` 查询当前角色、模式、链路和物理层状态；带 `-R` 时通过指定 slot 远程查询对端的这些基础信息。

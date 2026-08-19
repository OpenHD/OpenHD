# L4 Linux SDK

`L4_Linux_SDK` 是面向 Linux 的精简 SDK，用于构建和使用 8030/L4 设备的后台服务、客户端库、应用工具和 API 示例。

本仓库当前维护的是整理后的 Linux SDK 内容。`build/` 和 `install/` 是构建、安装输出，不应作为源码维护。

> ## [L4 Linux SDK 开发手册](docs/L4%20Linux%20SDK%20开发手册.md)
> README 用于快速了解和最小验证；完整编译、部署、工具参数、API 示例和排错说明请阅读开发手册。

## 运行链路

SDK 的最小运行链路如下：

```text
+-----------------------------+
| 用户程序 / SDK 示例程序      |
| l4_basic_info 等            |
+--------------+--------------+
               |
               | 调用 SDK API
               v
+-----------------------------+
| libar8030_client.so         |
| 封装客户端接口和 RPC 通信    |
+--------------+--------------+
               |
               | 连接本机或远端 daemon
               v
+-----------------------------+
| l4_daemon                   |
| 设备访问、消息转发、热插拔   |
+--------------+--------------+
               |
               | USB / UART
               v
+-----------------------------+
| 8030 / L4 设备              |
+-----------------------------+
```

通常先启动 `l4_daemon`，再运行示例程序或用户应用。应用侧通过 `libar8030_client.so` 访问 daemon，由 daemon 负责和设备通信。

## 目录结构

```text
L4_Linux_SDK/
  app/              应用侧客户端库和工具
  com/              公共头文件、RPC、日志、缓冲区等基础代码
  daemon/           后台服务和 8030 设备后端
  examples/         API 示例程序
  docs/             开发手册
  script/           编译和清理脚本
  third_package/    第三方依赖源码包或补丁
  toolchain/        ARM64 交叉编译工具链目录
  build/            CMake 构建缓存，生成物
  install/          安装输出，生成物
```

构建和安装输出按架构分目录保存：

```text
build/x86_64/
build/arm64/
install/x86_64/
install/arm64/
```

## 主要产物

| 类型 | 产物 | 说明 |
| --- | --- | --- |
| 客户端库 | `libar8030_client.so` | 用户程序链接的 SDK 动态库 |
| 后台服务 | `l4_daemon` | 访问设备并为客户端程序提供 RPC 服务 |
| 应用工具 | `l4_ota_upgrade` | 固件升级工具 |
| 应用工具 | `l4_tuntap` | 网络透传工具 |
| 应用工具 | `l4_cmd_dbg` | 设备命令行调试工具 |
| 示例程序 | `l4_basic_info` | 查询设备基础信息和版本信息 |
| 示例程序 | `l4_pair_manager` | 图传对频和配对管理 |
| 示例程序 | `l4_link_monitor` | 链路状态、质量、吞吐、测距查询 |
| 示例程序 | `l4_link_config` | 链路频段、信道、频宽、MCS 配置 |
| 示例程序 | `l4_config_file` | 配置文件导入、导出和恢复 |
| 示例程序 | `l4_minidb_config` | MiniDB 持久化配置读写 |
| 示例程序 | `l4_uart_config` | UART 配置读写 |
| 示例程序 | `l4_socket_transfer` | 普通或加密 Socket 数据收发 |

`examples/00_common` 是示例公共库，封装连接 daemon、枚举设备和打开设备的公共流程，不是可单独运行的示例程序。

## 快速编译

脚本默认构建当前 SDK 的核心库、daemon、应用工具和全部示例程序，并按架构安装到 `install/<arch>/`。

### 本机 x86_64

在 Ubuntu PC 或 Ubuntu 虚拟机上执行：

```bash
cd L4_Linux_SDK
./script/cmk-local.sh
```

输出目录：

```text
build/x86_64/
install/x86_64/
```

### ARM64 交叉编译

在 PC 上为 arm64 Linux 开发板交叉编译：

```bash
cd L4_Linux_SDK
./script/cmk-arm.sh
```

输出目录：

```text
build/arm64/
install/arm64/
```

ARM64 构建使用 `compiler.arm.cmake`，默认期望工具链位于 `toolchain/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu`。如果工具链放在其他目录，需要修改 `compiler.arm.cmake` 中的 `GCC_PATH`。

常用安装产物位置：

```text
install/<arch>/bin/
install/<arch>/lib/
install/<arch>/include/
```

需要手动控制 CMake 参数或 target 时，请参考开发手册中的“高级构建”章节。

## 快速验证

### x86_64 本机验证

连接 8030/L4 设备后，可先确认 USB 设备是否可见：

```bash
lsusb
```

启动 daemon：

```bash
cd L4_Linux_SDK/install/x86_64/bin
sudo ./l4_daemon
```

另开一个终端查询设备版本信息：

```bash
cd L4_Linux_SDK/install/x86_64/bin
./l4_basic_info -V
```

如果程序提示连接 daemon 失败，先确认 `l4_daemon` 是否正在运行。如果提示没有设备，先确认设备连接、USB 权限和 daemon 日志。

### arm64 开发板验证

先在 PC 侧执行 ARM64 交叉编译，然后将 `install/arm64/` 中的 `bin/`、`lib/`、`include/` 按项目部署方式拷贝到开发板。

在开发板上启动 daemon：

```bash
cd <deploy-dir>/bin
sudo ./l4_daemon
```

ARM64 脚本会构建并安装示例程序，可根据实际设备状态运行：

```bash
./l4_minidb_config -A
./l4_uart_config -g 1
```

## 清理构建产物

清理全部架构的构建和安装输出：

```bash
cd L4_Linux_SDK
./script/clean.sh
```

或：

```bash
./script/clean.sh all
```

只清理 x86_64：

```bash
./script/clean.sh x86_64
```

`x86` 也可作为 `x86_64` 的别名。

只清理 arm64：

```bash
./script/clean.sh arm64
```

`arm` 也可作为 `arm64` 的别名。

## 下一步阅读

- 第一次使用 SDK：阅读 [L4 Linux SDK 开发手册](docs/L4%20Linux%20SDK%20开发手册.md) 的“SDK 编译使用”和“最快验证”章节。
- 开发自己的应用：先看 `examples/01_basic_info` 和 `examples/00_common`，理解连接 daemon、枚举设备、打开设备和调用 `bb_ioctl()` 的最小流程。
- 查 API 结构体和命令字：查看 `com/bb_api.h`、`com/prj_rpc.h` 和 `app/ar8030/ar_net_api.h`，并结合 `examples/` 中对应示例理解调用方式。
- 使用应用工具：阅读开发手册中的“应用工具”章节。
- 手动 CMake 构建和排错：阅读开发手册中的“高级构建”和“常见问题”章节。

# 00_common 示例公共库说明

这份文档解释 `00_common` 目录里的公共代码做了什么，以及其它示例程序为什么都要依赖它。目标是让刚接触这个工程的人能先理解示例程序共同的连接和资源管理流程。

## 1. 先理解几个概念

`00_common` 不是一个单独运行的示例程序，而是给其它示例复用的公共静态库。

| 名词 | 可以这样理解 | 在代码里的含义 |
| --- | --- | --- |
| `l4_example_common` | 示例公共库 | `00_common/CMakeLists.txt` 里定义的 static library |
| daemon | 后台服务程序 | 示例程序先连接 daemon，再通过它访问 8030 设备 |
| host | daemon 连接对象 | `bb_host_connect()` 返回后保存在 `ctx->host` |
| device list | 设备列表 | `bb_dev_getlist()` 返回的设备数组 |
| handle | 已打开设备句柄 | 示例后续调用 `bb_ioctl()` 时使用 |
| context | 示例上下文 | `bb_demo_context_t`，集中保存连接和设备资源 |

`01_basic_info`、`02_pair_manager` 这类示例都需要先连接 daemon、枚举设备、打开指定设备。`00_common` 把这部分重复逻辑统一封装起来，避免每个示例都重新写一遍。

## 2. 这个公共库提供什么

`bb_demo_common.c` 和 `bb_demo_common.h` 编译后生成静态库 `l4_example_common`，主要提供四个接口：

| 函数 | 作用 |
| --- | --- |
| `bb_demo_context_init()` | 初始化 `bb_demo_context_t` |
| `bb_demo_open()` | 连接 daemon、枚举设备、打印设备信息、打开指定设备 |
| `bb_demo_close()` | 关闭设备、释放设备列表、断开 daemon |
| `bb_demo_print_device_info()` | 打印指定设备的 MAC 信息 |

示例程序只需要包含：

```c
#include "bb_demo_common.h"
```

然后链接 `l4_example_common`，就可以复用这些公共能力。

## 3. CMake 里是怎么用的

`00_common/CMakeLists.txt` 定义了：

```cmake
add_library(l4_example_common STATIC
    bb_demo_common.c
)
```

这个库公开两个 include 路径：

| include 路径 | 作用 |
| --- | --- |
| `${CMAKE_CURRENT_SOURCE_DIR}` | 让其它示例能包含 `bb_demo_common.h` |
| `${PROJECT_SOURCE_DIR}/com` | 让公共库能包含 SDK 公共头文件，比如 `bb_api.h` |

它还链接：

```cmake
target_link_libraries(l4_example_common
    PUBLIC
        ar8030_client
)
```

因此依赖 `l4_example_common` 的示例程序，也能使用 `ar8030_client` 提供的设备访问接口。

## 4. 典型调用方式

其它示例一般按下面方式使用 `00_common`：

```c
bb_demo_context_t ctx;
int ret;

ret = bb_demo_open(&ctx, addr, port, dev_index);
if (ret) {
    return -1;
}

/* 使用 ctx.handle 调用 bb_ioctl() */

bb_demo_close(&ctx);
```

完整流程可以理解成：

```text
示例解析 -a/-p/-i 参数
    |
bb_demo_open()
    |
bb_host_connect()
    |
bb_dev_getlist()
    |
bb_demo_print_device_info()
    |
bb_dev_open()
    |
示例使用 ctx.handle 调用 bb_ioctl()
    |
bb_demo_close()
    |
bb_dev_close() / bb_dev_freelist() / bb_host_disconnect()
```

## 5. `bb_demo_context_t` 里保存了什么

`bb_demo_context_t` 是示例程序的公共上下文：

```c
typedef struct {
    bb_host_t *host;
    bb_dev_list_t *devs;
    bb_dev_handle_t *handle;
    int dev_count;
    int dev_index;
} bb_demo_context_t;
```

| 字段 | 含义 |
| --- | --- |
| `host` | daemon 连接对象 |
| `devs` | 当前 daemon 返回的设备列表 |
| `handle` | 已打开的设备句柄 |
| `dev_count` | 设备数量 |
| `dev_index` | 当前打开的设备 index |

示例真正需要频繁使用的是 `ctx.handle`。例如 `01_basic_info` 会用它调用 `BB_GET_SYS_INFO` 和 `BB_GET_STATUS`，`02_pair_manager` 会用它启动对频、停止对频和查询对频结果。

## 6. 打开设备时，程序内部做了什么

调用：

```c
bb_demo_open(&ctx, addr, port, dev_index);
```

内部会先清空上下文：

```c
bb_demo_context_init(ctx);
```

然后连接 daemon：

```c
bb_host_connect(&ctx->host, addr, port);
```

连接成功后获取设备列表：

```c
ctx->dev_count = bb_dev_getlist(ctx->host, &ctx->devs);
```

如果找到设备，会逐个打印设备信息：

```c
bb_demo_print_device_info(ctx->devs[i], i);
```

最后检查 `dev_index` 是否合法，并打开对应设备：

```c
ctx->handle = bb_dev_open(ctx->devs[dev_index]);
```

打开成功后，其它示例就可以通过 `ctx.handle` 继续发送 `bb_ioctl()` 命令。

## 7. 关闭资源时，程序内部做了什么

调用：

```c
bb_demo_close(&ctx);
```

内部会按顺序释放资源：

```text
如果 handle 有效，调用 bb_dev_close()
    |
如果 devs 有效，调用 bb_dev_freelist()
    |
如果 host 有效，调用 bb_host_disconnect()
    |
清空 dev_count 和 dev_index
```

这样每个示例都可以用同一个函数做收尾，不需要重复写资源释放逻辑。

## 8. 错误处理怎么看

`bb_demo_open()` 遇到错误时会直接打印原因并返回非 0：

| 失败位置 | 常见原因 | 行为 |
| --- | --- | --- |
| `bb_host_connect()` | daemon 未启动、地址或端口错误 | 打印 `bb_host_connect failed` 并返回错误 |
| `bb_dev_getlist()` | daemon 没有发现设备 | 打印 `found no device`，清理资源后返回错误 |
| `dev_index` 检查 | `-i` 指定的设备序号超出范围 | 打印合法范围，清理资源后返回错误 |
| `bb_dev_open()` | 指定设备打开失败 | 打印 index，清理资源后返回错误 |

因此示例程序通常只需要判断 `bb_demo_open()` 的返回值：

```c
ret = bb_demo_open(&ctx, addr, port, dev_index);
if (ret) {
    return -1;
}
```

## 9. 新手最容易混淆的点

### `00_common` 本身不会生成可执行程序

它生成的是静态库 `l4_example_common`。真正能运行的是依赖它的示例程序，比如 `l4_basic_info` 和 `l4_pair_manager`。

### `dev_index` 不是设备内部的 slot

`dev_index` 是 daemon 枚举出的设备序号，表示打开第几个物理设备。slot 是设备内部的链路位置，两者不是一回事。

### 示例程序不是直接访问底层 USB 或 UART

示例程序通过 `bb_host_connect()` 连接 daemon，再通过 daemon 管理设备。USB、UART 等底层通信由 daemon 和 SDK 内部实现负责。

### 失败时也会尽量清理资源

`bb_demo_open()` 在获取设备列表后，如果发现 index 非法或打开设备失败，会调用 `bb_demo_close()` 清理已经获得的资源。

## 10. 和源码函数的对应关系

| 函数 | 做的事情 |
| --- | --- |
| `print_mac_field()` | 按可打印字符串或十六进制格式打印设备 MAC 字段 |
| `bb_demo_context_init()` | 清空上下文，并把 `dev_index` 初始化为 `-1` |
| `bb_demo_print_device_info()` | 调用 `bb_dev_getinfo()` 并打印设备 MAC |
| `bb_demo_open()` | 连接 daemon、获取设备列表、打开指定设备 |
| `bb_demo_close()` | 关闭设备、释放设备列表、断开 daemon |

## 11. 一句话总结

`00_common` 做的事情就是：把所有示例都需要的“连接 daemon、枚举设备、打开设备、关闭资源”流程封装成 `l4_example_common`，让其它示例只专注于演示具体的 SDK 功能。

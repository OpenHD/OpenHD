# 02_pair_manager 图传对频说明

这份文档解释 `l4_pair_manager` 这个示例程序做了什么，以及一次完整的图传对频大概是怎么跑起来的。目标是让刚接触这个工程的人也能看懂。

## 1. 先理解几个概念

可以把图传对频理解成“AP 和 DEV 互相认识，并把对方记到指定位置上”。

| 名词 | 可以这样理解 | 在代码里的含义 |
| --- | --- | --- |
| AP | 基站、主机、接收端 | 负责开放某个 slot，等待 DEV 接入 |
| DEV | 终端、从机、发射端 | 主动寻找 AP 并完成绑定 |
| slot | AP 上的座位号 | 一个 AP 可以管理多个 DEV，每个 DEV 占一个 slot |
| bitmap | 一组座位开关 | `1 << slot` 表示开放某个 slot |
| pair | 对频、配对 | 让 AP 和 DEV 建立对应关系 |
| peer_mac | 对端 MAC | 对频成功后记住的对方设备地址 |

举例：AP 执行 `-s 0` 时，程序会让 AP 开放 `slot 0`。DEV 也执行对频后，如果双方搜索到并匹配成功，AP 的 `slot 0` 就会记录这个 DEV 的 MAC。

## 2. 这个程序能做什么

`pair_manager.c` 编译后生成可执行程序 `l4_pair_manager`，它主要支持四类操作：

| 参数 | 作用 |
| --- | --- |
| `-P` | 启动对频，并等待对频结果事件 |
| `-X` | 停止当前对频 |
| `-m` | 查询对频结果和对端 MAC |
| `-M <mac>` | 按角色设置对端 MAC |

其它常用参数：

| 参数 | 作用 | 默认值 |
| --- | --- | --- |
| `-a <addr>` | daemon 地址 | `127.0.0.1` |
| `-p <port>` | daemon 端口 | `BB_PORT_DEFAULT` |
| `-i <index>` | 选择第几个设备 | `0` |
| `-s <slot>` | AP 侧开放的 slot | `0` |
| `-t <sec>` | 设备侧对频超时时间 | `100` |

## 3. 最常用的操作方式

### AP 侧启动 slot 0 对频

```sh
./l4_pair_manager -P -s 0
```

意思是：让 AP 开放 `slot 0`，等待 DEV 来对频。

### DEV 侧启动对频

```sh
./l4_pair_manager -P
```

意思是：让 DEV 进入对频流程，去寻找可以匹配的 AP。

### 查询对频结果

```sh
./l4_pair_manager -m
```

对频成功后，使用 `-m` 按当前角色查看对端 MAC：AP 侧查看指定 slot 上的 DEV MAC，DEV 侧查看 AP MAC。

### AP 侧手动设置 DEV MAC

```sh
./l4_pair_manager -M 11:22:33:44 -s 0
```

意思是：在 AP 侧把 `11:22:33:44` 设置为 `slot 0` 的候选 DEV MAC。内部调用 `BB_SET_CANDIDATES`。

### DEV 侧手动设置 AP MAC

```sh
./l4_pair_manager -M 11:22:33:44
```

意思是：在 DEV 侧把 `11:22:33:44` 设置为目标 AP MAC。内部调用 `BB_SET_AP_MAC`。

MAC 是 4 字节格式，支持冒号或横线分隔，例如 `11:22:33:44` 或 `11-22-33-44`。

### 停止对频

```sh
./l4_pair_manager -X
```

如果对频一直没有结束，或者想手动退出对频，可以执行停止命令。

### 指定对频超时时间

```sh
./l4_pair_manager -P -s 0 -t 60
```

这里 `-t 60` 表示设备侧最多对频 60 秒。未对频成功时，设备会通过 `BB_EVENT_PAIR_RESULT` 返回对频超时事件。

## 4. 一次启动对频时，程序内部做了什么

执行：

```sh
./l4_pair_manager -P -s 0
```

程序内部大致按下面流程运行：

```text
解析命令行参数
    |
连接 daemon 和指定设备
    |
读取当前设备角色：AP 还是 DEV
    |
订阅 BB_EVENT_PAIR_RESULT 对频结果事件
    |
发送 PRJ_CMD_EVENT_PAIR 启动对频
    |
循环等待事件回调
    |
收到成功事件后按角色查询对端 MAC
    |
AP 侧查询 BB_GET_CANDIDATES，DEV 侧查询 BB_GET_AP_MAC
    |
打印对端 MAC
    |
关闭设备连接
```

### 第一步：连接设备

`main()` 里会调用：

```c
bb_demo_open(&ctx, addr, port, dev_index);
```

它的作用是连接本机或指定地址上的 daemon，然后选择某个设备。后续所有 `bb_ioctl()` 都通过这个连接发给设备。

### 第二步：判断自己是 AP 还是 DEV

`start_pair()` 会先调用 `get_role()`，内部用的是：

```c
bb_ioctl(handle, BB_GET_STATUS, &input, &status);
```

程序从 `status.role` 得到当前设备角色：

| 角色 | 程序行为 |
| --- | --- |
| `BB_ROLE_AP` | AP 侧启动对频，要指定开放哪个 slot |
| `BB_ROLE_DEV` | DEV 侧启动对频，不需要指定 slot bitmap |
| 其它角色 | 不支持对频，程序退出 |

### 第三步：订阅对频结果事件

启动对频前，程序会先订阅事件：

```c
cb.event = BB_EVENT_PAIR_RESULT;
cb.callback = pair_result_cb;
bb_ioctl(handle, BB_SET_EVENT_SUBSCRIBE, &cb, NULL);
```

这样设备对频结束后，就能通过 `pair_result_cb()` 通知程序结果。

这一步很重要：对频是异步动作，不是发完命令马上就能知道结果。程序必须先注册回调，再启动对频，然后等待设备回调。

### 第四步：发送启动对频命令

程序通过项目命令分发接口启动对频：

```c
hdr->cmdid = PRJ_CMD_EVENT_PAIR;
pair->bitmap = (role == BB_ROLE_AP) ? (uint8_t)(1u << slot) : 0;
pair->timeout = (int16_t)timeout_sec;
pair->asyn = 1;
bb_ioctl(handle, BB_SET_PRJ_DISPATCH, &request, NULL);
```

这里几个字段的意思是：

| 字段 | 含义 |
| --- | --- |
| `cmdid = PRJ_CMD_EVENT_PAIR` | 告诉设备：我要启动对频 |
| `bitmap` | AP 侧开放哪些 slot；DEV 侧填 0 |
| `timeout` | 设备侧对频最多持续多少秒 |
| `asyn = 1` | 异步对频，结果通过事件回调返回 |

AP 和 DEV 的 `bitmap` 不一样：

| 当前角色 | bitmap 写法 | 含义 |
| --- | --- | --- |
| AP | `1 << slot` | 只开放指定 slot 给 DEV 对频 |
| DEV | `0` | DEV 不管理 slot，这个字段忽略 |

例如 `slot = 0` 时，AP 侧 `bitmap = 0x01`；`slot = 1` 时，AP 侧 `bitmap = 0x02`。

### 第五步：等待对频结果

启动对频后，程序每秒打印一次：

```text
waiting for pair result...
waiting for pair result...
...
```

这表示程序还没收到 `BB_EVENT_PAIR_RESULT` 事件。

如果收到事件，会进入 `pair_result_cb()`：

```c
g_pair_ret = event->ret;
g_pair_done = 1;
```

然后根据 `event->ret` 打印结果：

| ret | 含义 |
| --- | --- |
| `0` | 对频成功 |
| `-2` | 对频超时 |
| 其它值 | 对频结束但结果异常，需要结合日志继续分析 |

### 第六步：成功后按角色读取对端 MAC

如果事件结果是 `ret = 0`，程序会继续调用 `get_pair_result(handle, slot)`。

注意：现在这个函数名虽然还叫 `get_pair_result()`，但它不再通过 `BB_GET_PAIR_RESULT` 读取旧的对频结果结构，而是先判断当前设备角色，再按角色读取对端 MAC。

| 当前角色 | 查询命令 | 查到什么 |
| --- | --- | --- |
| AP | `BB_GET_CANDIDATES` | 指定 slot 上的 DEV MAC 列表 |
| DEV | `BB_GET_AP_MAC` | DEV 当前记录的 AP MAC |

AP 侧会执行类似逻辑：

```c
input.slot = (uint8_t)slot;
bb_ioctl(handle, BB_GET_CANDIDATES, &input, &candidates);
```

输出示例：

```text
[BB_GET_CANDIDATES]
slot: 0 mac_num: 1
dev_mac[0]: xx:xx:xx:xx:xx:xx
```

这表示 AP 的 `slot 0` 上读取到了一个 DEV MAC。

DEV 侧会执行：

```c
bb_ioctl(handle, BB_GET_AP_MAC, NULL, &ap_mac);
```

输出示例：

```text
[BB_GET_AP_MAC]
ap_mac: xx:xx:xx:xx:xx:xx
```

这表示 DEV 当前记录的配对 AP MAC。

## 5. 停止对频时，程序内部做了什么

执行：

```sh
./l4_pair_manager -X
```

程序会发送：

```c
hdr->cmdid = PRJ_CMD_EVENT_PAIR_STOP;
bb_ioctl(handle, BB_SET_PRJ_DISPATCH, &request, NULL);
```

这条命令告诉设备退出对频状态。

启动对频时如果按 `Ctrl+C`，程序不会直接退出，而是先设置 `g_stop_requested = 1`，然后在等待循环里调用同样的停止命令，尽量让设备也退出对频。

## 6. 查询命令怎么看

`-m` 是当前唯一的对频结果查询命令；`-M <mac>` 用于手动设置对端 MAC。

### `-m` 查询对频结果

```sh
./l4_pair_manager -m
```

内部会先查询当前角色，然后按角色选择查询方式：

| 当前角色 | `-m` 实际调用 | 结果 |
| --- | --- | --- |
| AP | `BB_GET_CANDIDATES` | 指定 slot 上的 DEV MAC 列表 |
| DEV | `BB_GET_AP_MAC` | 当前配对 AP 的 MAC |

所以 AP 侧使用 `-m` 时通常要带上 `-s <slot>`，例如：

```sh
./l4_pair_manager -m -s 0
```

### `-M` 设置对端 MAC

`-M` 会先查询当前角色，然后按角色选择设置方式：

| 当前角色 | `-M` 实际调用 | 结果 |
| --- | --- | --- |
| AP | `BB_SET_CANDIDATES` | 设置指定 slot 的 DEV 候选 MAC |
| DEV | `BB_SET_AP_MAC` | 设置目标 AP MAC |

AP 侧示例：

```sh
./l4_pair_manager -M 11:22:33:44 -s 0
```

DEV 侧示例：

```sh
./l4_pair_manager -M 11:22:33:44
```

如果和启动对频一起使用，程序会先写入 MAC，再启动对频：

```sh
./l4_pair_manager -M 11:22:33:44 -s 0 -P
```

## 7. 新手最容易混淆的点

### 对频不是只在 AP 侧执行

通常 AP 和 DEV 都需要进入对频流程。AP 侧开放 slot，DEV 侧寻找 AP。只有一边启动时，另一边如果不在可匹配状态，可能会一直等到超时。

### AP 侧的 `slot` 不是 DEV 的编号

`slot` 是 AP 上的位置。比如 AP 开放 `slot 0`，成功后表示这个 DEV 被绑定到 AP 的 `slot 0`。

### `-t` 控制设备侧对频超时

`-t` 是发给设备的对频超时时间。程序启动对频后会一直等待 `BB_EVENT_PAIR_RESULT`，对频成功或设备侧超时都会通过这个事件返回。

### 看到 `pair ok` 后还会继续打印结果

这是正常的。`pair ok` 来自事件回调，表示设备通知“对频成功”；后面的 `[BB_GET_CANDIDATES]` 或 `[BB_GET_AP_MAC]` 是程序主动按角色查询对端 MAC。

## 8. 和源码函数的对应关系

| 函数 | 做的事情 |
| --- | --- |
| `main()` | 解析参数、打开设备、按参数执行启动/停止/查询 |
| `start_pair()` | 启动对频并等待结果 |
| `get_role()` | 查询当前设备角色 |
| `subscribe_pair_event()` | 注册对频结果回调 |
| `pair_result_cb()` | 收到对频结果事件后记录结果 |
| `send_pair_stop()` | 发送停止对频命令 |
| `get_candidates_mac()` | AP 侧查询指定 slot 上的 DEV MAC 列表 |
| `get_ap_mac()` | DEV 侧查询当前记录的 AP MAC |
| `get_pair_result()` | 按角色选择 AP 查询 DEV MAC 或 DEV 查询 AP MAC |
| `set_pair_mac()` | 按角色选择 AP 设置 DEV MAC 或 DEV 设置 AP MAC |

## 9. 一句话总结

`l4_pair_manager` 做的事情就是：连接设备，判断自己是 AP 还是 DEV，支持启动/停止对频，也支持查询和手动设置对端 MAC；AP 侧通过 slot 管理 DEV MAC，DEV 侧记录目标 AP MAC。

# 项目自定义事件例程

`l4_project_event` 用于演示地面端 Linux 向天空端 Linux 发送项目自定义事件。

完整链路如下：

```text
地面端 Linux
  BB_REMOTE_IOCTL_REQ
  msg_id = BB_SET_PRJ_DISPATCH
        ↓
天空端 8030
  prj_rpc_set(PRJ_CMD_EVENT_DEMO)
  bb_event_publish(BB_EVENT_PRJ_DISPATCH)
        ↓
天空端 Linux
  BB_SET_EVENT_SUBSCRIBE
  回调处理 BB_EVENT_PRJ_DISPATCH
```

## 测试协议

固定的 256 字节项目分发缓冲区以 `prj_rpc_hdr_t` 开头。当
`cmdid = PRJ_CMD_EVENT_DEMO` 时，`hdr->data` 中保存
`prj_cmd_event_demo_t`：

| `data` | 天空端 Linux 动作 |
| --- | --- |
| `0x00` | 将演示软件状态切换为 `DISARMED` |
| `0xff` | 将演示软件状态切换为 `ARMED` |

相关命令号和结构已经定义在 `com/prj_rpc.h` 中，本例程不新增公共 API，
也不新增 XData 消息号。

## 编译

在 L4 Linux SDK 根目录执行：

```sh
cmake -S . -B build
cmake --build build --target l4_project_event
```

天空端和地面端必须使用包含 `PRJ_CMD_EVENT_DEMO` 处理分支的 KT 固件。
该分支位于 `board/KT/CODE/prj_rpc.c`，构建 KT 板型时会覆盖到实际参与
编译的 `apps/tests/p401/prj_rpc.c`。

## 两台电脑测试

先在天空端 Linux 上启动接收程序：

```sh
./l4_project_event -r
```

然后在地面端 Linux 上通过远端 slot 0 发送状态：

```sh
./l4_project_event -t armed -s 0
./l4_project_event -t disarmed -s 0
```

## 一台电脑测试

可以使用一台电脑完成端到端测试，但需要满足以下条件：

- 地面端和天空端两台 8030 均连接到这台电脑。
- 两台 8030 已配对并建立无线链路。
- daemon 能同时枚举两台设备，且可以通过不同的设备索引区分它们。
- 两台设备均已刷入包含本例程固件修改的 KT 固件。

先运行任意一个例程命令查看设备列表，确认地面端和天空端对应的索引。例如，
天空端索引为 1、地面端索引为 0。

终端 1 启动天空端事件接收程序：

```sh
./l4_project_event -r -i 1
```

终端 2 从地面端发送事件：

```sh
./l4_project_event -t armed -s 0 -i 0
./l4_project_event -t disarmed -s 0 -i 0
```


## 验收结果

发送端显示 `BB_REMOTE_IOCTL_REQ` 成功，表示对端 8030 已接受请求并完成
事件发布调用。天空端收到 `BB_EVENT_PRJ_DISPATCH`、打印对应数据并完成
`ARMED/DISARMED` 状态变化，才表示端到端测试通过。

本例程不发送天空端 Linux 动作完成后的反向 ACK。使用 `-a`、`-p` 和
`-i` 可以指定 daemon 地址、端口和本地设备索引；执行
`./l4_project_event -h` 可以查看全部选项。

## 进一步阅读

详细的数据链路、协议格式、固件处理、回调机制和扩展方法，请参阅
[《项目自定义事件实现原理》](实现原理.md)。

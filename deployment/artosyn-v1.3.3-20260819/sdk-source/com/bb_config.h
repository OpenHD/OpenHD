/**
 * \file
 * \brief 定义基带软件的主要规格与功能特性
 */
#ifndef __BB_CONFIG_H__
#define __BB_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \page sdk_cmd AR8030-SDK基带调试命令
 *
 * \section sdk_cmd_brief 概述
 * 为了方便进行系统级的调试诊断，SDK封装了一些常用的命令，同时为了版本镜像大小的考虑，所有命令可可以使用编译宏进行关闭，所以当前SDK是否支持某个命令，请从bb_config.h中查看定义。
 * 调试命令中涉及的数值，支持灵活识别机制-以0x开头的强制16进制，不是0x开头的但是含有a-f等字符的识别为16进制，否则为10进制。
 *
 * \section sdk_cmd_address 地址空间访问命令
 * \subsection cmd_d 1.d命令
 * 格式：d [addr] [size]
 * 说明：显示地址空间，其中addr和size有记忆功能，如果第一次执行 d 0xa110800 1，第二次执行d，得到的效果与前一次相同
 *
 * \subsection cmd_m 2.m命令
 * 格式：m [addr] [width] [value]
 * 说明：修改地址空间，其中addr为想要修改的地址，width是访问位宽，支持8、16、32，具体用哪一种取决于地址空间所处总线的访问限制，value是要写入的数值
 *
 * \subsection cmd_p 3.p命令
 * 格式：p [page]
 * 说明：显示基带寄存器page的命令，内部是对d命令的简单封装，其中page有效范围为0~15
 *
 * \section sdk_cmd_base 基带SDK支撑层命令
 * \subsection cmd_bi 1.bi命令
 * 格式：bi 显示统计信息 或 bi reset 统计信息复位
 * <pre>
 * 说明：显示基带消息驱动任务的统计信息，字段定义如下：
 *       task - 任务名
 *       State - 任务状态
 *       Pri - 任务优先级
 *       Stack - 栈大小
 *       QSize - 消息队列大小（消息个数）
 *       CurM - 当前处理的消息ID，0为不在处理
 *       CurQ - 当前消息队列中的消息个数
 *       MaxQ - 消息队列历史最大消息数量
 *       MaxTM - 历史执行时间最长的消息ID
 *       MaxT - 历史执行时间最长的时间
 * </pre>
 *
 * \subsection cmd_mt 2.mt命令
 * 格式：mt [msg_id]
 * 说明：跟踪指定的消息的执行，一旦捕捉到，会得到"msg [msg_id] proc in [time] us"，表示这个消息在time时间内完成执行，如果要停止跟踪执行mt 0就可以了
 *
 * \subsection cmd_log 3.log命令
 * 格式：log 或 log [module id] [level]
 * <pre>
 * 说明：log命令用于控制基带子模块的调试输出等级，目前三个预定义等级为0：debug，1：information，2：error，当调试等级设置到3，一切输出都停止。
 *       如果仅执行log命令，则显示当前基带所有子模块名、对应ID以及调试等级。
 *       log信息的输出格式，举例：[1025610][10][INF] bb_phy_distc_proc: skip td_out 1048166，其中第一个字段是启动启动后的毫秒数，第二是子模块ID，第三个为打印等级，第四个为函数名，最后为log信息
 * </pre>
 *
 * \subsection cmd_debug 4.debug命令
 * 格式：debug
 * 说明：debug命令设计为toggle命令，执行一次，进debug模式，再执行一次，退出debug模式。debug模式是SDK的内部调试模式，软件不参与非必须的逻辑，仅用于基带诊断或产品测试
 *
 * \section sdk_cmd_bb 基带相关调试命令
 * \subsection cmd_frame 1.frame命令
 * 格式：frame
 * <pre>
 * 说明：显示当前设备超级帧及其他相关信息，字段定义如下：
 *       type - 帧单元类型
 *       dir - 帧单元方向
 *       prec - 预编码位置，格式 [单元参考]@[8分位中的第n位]，参考单元如果是0就是本单元，1为后一个单元，-1为前一个单元
 *       time - 单元的时间长度，单位：微秒
 *       freq - 单元的射频频率，单位：KHz
 *       bw - 单元的频宽，单位：MHz
 *       pld - 单元是否有payload，on（有）off（没有）
 *       tintlv - 是否时域交织
 *       MCS - 发送MCS级别
 *       TX_Mode - 发送RF模式
 *       RX_Mode - 接收RF模式
 *       FCH - FCH信息，格式[OFDM数]@[总数据bit数]
 *       LTP - LTP信息，格式[长短模式]:[LTP ID]
 *       BL(B) - Byte Length，每个交织块的最大的可携带数据量，单位：Byte
 *       TP(K) - Throughout，当前单元的发送速率，单位：Kbps
 * </pre>
 *
 * \subsection cmd_ll 2.ll命令
 * 格式：ll
 * <pre>
 * 说明： 显示当前的链路层运行时状态，对于AP侧来说，会显示所有使能位置设备的状态，对于DEV来说，仅显示与AP的状态，关键字段说明：
 *       Slot Bmp - AP侧配置的使能的slot位置
 *       RT Slot Bmp - 运行时的slot bmp，当使用动态slot模式时，此字段表示真实的超级帧状态，当使用固定slot模式，这个字段与Slot Bmp一致
 *       State Bmp - 状态bitbmap（LTX：链路TX使能 LRX：链路RX使能 STX：数据TX使能 SRX：数据RX使能 PAIR：配对状态）
 *       Frame Time - 超级帧时间长度，向上对其到毫秒单位
 *       Up Channel - 上行信道
 *       Down Channel - 下行信道
 *       AP State - AP状态
 *       DEV State - DEV状态，注意不同角色的设备的状态以对应的状态为准，另一个为对端状态，仅用于辅助状态机的推动
 *       My MAC - 本设备所使用MAC
 *       AP MAC - 用于接入的目标接入点MAC
 *       DEV MAC - 对应slot位置的DEV MAC
 *       RX MCS - 当前位置的接收MCS等级
 *       Link unlock - 链路层unlock计数
 * </pre>
 *
 * \subsection cmd_mac 3.mac命令
 * 格式：mac [--tx|--rx] [--user n] [--detail]
 * <pre>
 * 说明：MAC层的命令用基带物理用户为参数，所以如果查寻BR通道，--user应为8
 *       1. mac --tx [--user n]
 *       发送端MAC侧统计信息，主要的统计字段解释如下：
 *       User ID - 物理用户ID
 *       Log Slot - 逻辑slotID
 *       PKg Index - MAC包循环ID
 *       byte length - 基带实际发送最大Byte数
 *       Total TLV size - 总的发送字节数统计值
 *       Total Pkg size - 总的包大小，含MAC协议头
 *       Total Data size - 总的用户数据大小
 *       Throughput - 总基带吞吐率
 *       Real Throughput - 实际数据吞吐率
 *
 *       --detail detail选项用于显示一个统计窗内的发送记录，包括时间戳、byte length和实际包长度
 *
 *       2. mac --rx [--user n]
 *       接收端MAC侧统计信息，主要的统计字段解释如下：
 *       User ID - 物理用户ID
 *       Log Slot - 逻辑slotID
 *       PKg Index - MAC包循环ID
 *       byte length - 基带实际收到的最大Byte数
 *       Total TLV size - 总的接收到的字节数统计值(有效数据，可能与发送端实际发送的长度不符)
 *       Total Pkg size - 总的包大小，含MAC协议头
 *       Total Data size - 总的用户数据大小
 *       Throughput - 总基带吞吐率
 *       Real Throughput - 实际数据吞吐率
 *
 *       --detail detail选项用于显示一个统计窗内的接收记录，包括时间戳、byte length和实际包长度
 * </pre>
 *
 * \subsection cmd_chan 4.chan命令
 * 格式： chan
 * <pre>
 * 说明：信道状态查看命令，用于显示信道相关的信息，字段解释如下：
 *       Band BMP   - 系统支持的band的bitmap
 *       Band Mode  - band工作模式
 *       Cur Bnad   - 当前工作band
 *       Chan Num   - 信道数量
 *       ACS Chan - 起机自动选频机制下选择的最佳信道索引
 *       Work Chan - 当前工作信道，通常指RX方向
 *       Best Chan - 当前扫频子系统认为的最好信道，会动态改变
 *       Freq(MHz) - 信道中心频点
 *       Chan Power - 信道的扫频能量
 * 格式：chan --auto_band [0|1]
 * 说明：控制band自适应或手动模式 0=手动 1=自动
 *
 * 格式：chan --set_band [0|1|2] [--tx]
 * 说明：在band手动模式下，设置本地或对端的工作band，0=1G 1=2G 2=5G
 *
 * 格式：chan --auto_chan [0|1] [--tx]
 * 说明：控制本地或对端的信道工作模式 0=手动 1=自动
 *
 * 格式：chan --set_chan [chan index] [--tx] [--offline]
 * 说明：控制本地或对端的工作信道，注意--offline选项提供了在脱机模式下修改BR信道的可能
 *
 * 格式：chan --br_hop [fixed|follow_up_chan|hop_on_idle]
 * 说明：控制BR链路的跳频策略
 *
 * </pre>
 *
 * \subsection cmd_irq 5.irq命令
 * 格式：irq [bb_irq_idx]
 * <pre>
 * 说明：基带中断子系统调试命令，这个命令设计为toggle模式，执行一次打开中断跟踪，再执行一次则关闭跟踪
 *       bb_irq_idx具体的对应关系，由SDK人员进行说明
 * </pre>
 *
 * \subsection cmd_power 6.power命令
 * 格式：power
 * <pre>
 * 说明：显示当前功率状态以及信号质量相关信息：
 * </pre>
 * <pre>
 * 格式：power --m [openloop|closeloop]
 * </pre>
 * <pre>
 * 说明：功率操作命令，用于设置功率开闭环模式：
 * </pre>
 * <pre>
 * 格式：power --a [on|off]
 * </pre>
 * <pre>
 * 说明：功率操作命令，用于设置功率自适应开关：
 * </pre>
 * <pre>
 * 格式：power --u [0-11] --d n
 * </pre>
 * <pre>
 * 说明：功率操作命令，用于设置用户功率：
 * </pre>
 * <pre>
 * 格式：power --u [0-11] --get
 * </pre>
 * <pre>
 * 说明：功率操作命令，用于获取用户功率：
 * </pre>
 */


/********************************Macro Definition********************************/
/** \addtogroup      BB_API */
/** @{ */  /** <!-- [BB_API] */

#define BB_CONFIG_FPGA_TEST_MODE            0           /**<芯片验证阶段模式*/

#define BB_CONFIG_DEBUG_BUILD               1           /**<@atendtion 当前SDK是debug构建版本*/
#define BB_CONFIG_PSRAM_ENABLE              1           /**<@attention 使能基带PSRAM机制*/
#define BB_CONFIG_MRC_ENABLE                1           /**<@attention 使能基带MRC机制*/
#define BB_CONFIG_MAX_TRANSPORT_PER_SLOT    4           /**<@attention 每个SLOT上最大的transport数量*/
#define BB_CONFIG_MAX_INTERNAL_MSG_SIZE     128         /**<@attention SDK内部消息通道最大消息长度，不含消息头*/
#define BB_CONFIG_MAX_TX_NODE_NUM           10          /**<@note MAC层最大发送节点数量*/
#define BB_CONFIG_MAC_RX_BUF_SIZE           60000       /**<@note 默认socket的接收buffer大小*/
#define BB_CONFIG_MAC_TX_BUF_SIZE           40000       /**<@note 默认socket的发送buffer大小*/
#define BB_CONFIG_MAX_USER_MCS_NUM          16          /**<@note 最大用户可设置的MCS等级数量*/
#define BB_CONFIG_MAX_CHAN_NUM              32          /**<@note 最大用户可设置的信道数量*/
#define BB_CONFIG_MAX_CHAN_HOP_ITEM_NUM     5           /**<@note 最大跳频触发项条目数量*/
#define BB_CONFIG_MAX_SLOT_CANDIDATE        5           /**<@note 每个SLOT可设置的最大候选人数量*/
#define BB_CONFIG_BR_FREQ_OFFSET            0           /**<@note BR与信道的频偏值 单位：KHz*/
#define BB_CONFIG_LINK_UNLOCK_TIMEOUT       1000        /**<@note Link通道超时门限 单位：毫秒*/
#define BB_CONFIG_SLOT_UNLOCK_TIMEOUT       1000        /**<@note FCH超时门限 单位：毫秒*/
#define BB_CONFIG_IDLE_SLOT_THRED           1          /**<@note SLOT空闲门限，用于动态slot模式，单位：秒*/
#define BB_CONFIG_EOP_SAMPLE_NUM            8           /**<@note EOP处理最近样本大小*/
#define BB_CONFIG_ENABLE_BR_MCS             1           /**<@note 使能BR的MCS控制，仅对1V1模式有效*/
#define BB_CONFIG_ENABLE_BLOCK_SWITCH       1           /**<@note 使用阻塞式模式切换机制（实验室阶段）*/
#define BB_CONFIG_1V1_DEV_CTRL_BR_CHAN      1           /**<@note 1V1模式下，使能DEV控制BR的TX信道*/
#define BB_CONFIG_1V1_COMPT_BR              1           /**<@note 1V1模式下，BR压缩模式（实验室阶段）*/
#define BB_CONFIG_ENABLE_LTP                1           /**<@note 使能网络隔离机制*/
#define BB_CONFIG_ENABLE_TIME_DISPATCH      1           /**<@note 使能链路授时机制*/
#define BB_CONFIG_ENABLE_FRAME_CHANGE       1           /**<@note 使能1V1模式下，改变帧结构的功能*/
#define BB_CONFIG_ENABLE_RC_HOP_POLICY      1           /**<@note 使能选择性跳频策略*/
#define BB_CONFIG_ENABLE_AUTO_BAND_POLICY   1           /**<@note 使能频段自适应功能*/
#define BB_CONFIG_ENABLE_LNA_POLICY         1           /**<@note 使能LNA策略*/
#define BB_CONFIG_ENABLE_1V1_POWER_SAVE     1           /**<@note 使能1V1模式的节能机制*/
#define BB_CONFIG_ENABLE_RF_POLICY          1           /**<@note 使能B路开关自适应策略*/
#define BB_CONFIG_DEMO_STREAM               0           // TBD
#define BB_CONFIG_OLD_PLOT_MODE             0           // TBD
#define BB_CONFIG_FRAME_CROPPING            1           /**<@note 1VN模式下，动态删除或增加csma帧结构*/
#define BB_CONFIG_LINK_BY_GROUPID           0           /**<@note 分组配对开关(for hyy)*/
#define BB_CONFIG_ENABLE_RF_FILTER_PATCH    0           /**<@note 使能RF滤波patch*/
#define BB_CONFIG_ENABLE_DFS                0           /**<@note 使能DFS开关*/
#define BB_CONFIG_DFS_FREQ_OFFSET           500         /**<@note dfs检测的频偏值 单位：KHz*/
#define BB_CONFIG_TLV_PACKET_LOST_DEBUG     0           /**<@note 基带丢包检测开关*/
#define BB_CONFIG_ENABLE_THROUGHPUT_FUNC    1           /**<@note 使能获取吞吐率功能*/
#define BB_CONFIG_PWR_AUTO2                 1           /**<@note 功率自适应2.0版本，参考了packet err*/
#define BB_CONFIG_SOCKET_RX_BUF_SHARE       0           /**<@note 使用mblk提供socket rx buffer*/
#define BB_CONFIG_ENABLE_TR_SWITCH_HOOK     1           /**<@note 使能TR SWITCH钩子*/

#define BB_CONFIG_INCLUDE_D_CMD             1           /**<@note 包括d、p命令（显示地址空间）*/
#define BB_CONFIG_INCLUDE_M_CMD             1           /**<@note 包括m命令（修改地址空间）*/
#define BB_CONFIG_INCLUDE_BI_CMD            1           /**<@note 包括bi命令（显示基于消息推动任务的信息）*/
#define BB_CONFIG_INCLUDE_MT_CMD            1           /**<@note 包括mt命令（消息跟踪）*/
#define BB_CONFIG_INCLUDE_CL_CMD            0           /**<@note 包括cl命令（code len计算）*/
#define BB_CONFIG_INCLUDE_BL_CMD            1           /**<@note 包括bl命令（byte length计算）*/
#define BB_CONFIG_INCLUDE_FRAME_CMD         1           /**<@note 包括frame命令（显示超级帧信息）*/
#define BB_CONFIG_INCLUDE_FREQ_CMD          1           /**<@note 包括freq命令（频率与寄存器值互转）*/
#define BB_CONFIG_INCLUDE_IRQ_CMD           1           /**<@note 包括irq命令（基带中断调试）*/
#define BB_CONFIG_INCLUDE_CHAN_CMD          1           /**<@note 包括chan命令（信道信息）*/
#define BB_CONFIG_INCLUDE_MCS_CMD           1           /**<@note 包括mcs命令（mcs控制）*/
#define BB_CONFIG_INCLUDE_SWITCH_CMD        0           /**<@note 包括switch命令（停流切换）*/
#define BB_CONFIG_INCLUDE_CFG_CMD           1           /**<@note 包括cfg命令（基带软件配置）*/
#define BB_CONFIG_INCLUDE_EOP_CMD           1           /**<@note 包括eop命令（策略信息）*/
#define BB_CONFIG_INCLUDE_SLEEP_CMD         1           /**<@note 包括sleep命令（低功耗休眠），TBD*/
#define BB_CONFIG_INCLUDE_ECHO_CMD          1           /**<@note 包括echo命令（回显命令）*/
#define BB_CONFIG_INCLUDE_DISTC_CMD         1           /**<@note 包括distc命令（显示测距信息）*/
#define BB_CONFIG_INCLUDE_BR_CMD            0           /**<@note 包括br命令 （基带软件restart命令，实验室阶段）*/
#define BB_CONFIG_INCLUDE_CI_CMD            1           /**<@note 包括chipid命令*/
#define BB_CONFIG_INCLUDE_RT_CMD            1           /**<@note 包括rt命令*/
#define BB_CONFIG_SO_BUFFER_CNT             16          /**<@note rpc缓冲块数量*/
#define BB_PORT_DEFAULT                     (50000)     /**<@note host默认端口号*/
#define MBLK_PKT_DEBUG
/** @} */  /** <!-- ==== Macro Definition end ==== */


#if (BB_CONFIG_MAX_TRANSPORT_PER_SLOT > 255)
#error "invalid transport number"
#endif

#if (BB_CONFIG_MRC_ENABLE == 1 && BB_CONFIG_PSRAM_ENABLE != 1)
#error "invalid mrc compile cfg"
#endif

#ifdef __cplusplus
}
#endif

#endif

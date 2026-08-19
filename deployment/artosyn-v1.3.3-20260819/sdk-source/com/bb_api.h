/**
 * \file
 * \brief 描述Artosyn 8030基带SDK数据结构和接口函数
 */
#ifndef __BB_API_H__
#define __BB_API_H__

#include <stdint.h>
#include "bb_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/********************************Macro Definition********************************/
/** \addtogroup      BB_API */
/** @{ */  /** <!-- [BB_API] */

#if defined(SWIG)
#define PACK(...) __VA_ARGS__
#elif defined(_MSC_VER)
#define PACK(__Decl__) __pragma(pack(push, 1)) __Decl__ __pragma(pack(pop))
#else // GCC
#define PACK(__Decl__) __Decl__ __attribute__ ((packed))
#endif

#ifdef WIN32
#define AR8030_API __declspec(dllexport)
#else
#define AR8030_API
#endif

#define BB_MAC_LEN                  4                           /**<MAC地址字节长度*/
#define BB_REG_PAGE_NUM             16                          /**<基带寄存器页表的数量*/
#define BB_REG_PAGE_SIZE            256                         /**<基带寄存器页表的字节数量*/
#define BB_CFG_PAGE_SIZE            1024                        /**<基带配置文件分页的字节数量*/
#define BB_PLOT_POINT_MAX           10                          /**<基带plot事件的最大数据点数量*/
#define BB_BLACK_LIST_SIZE          3                           /**<基带配对黑名单大小*/
#define BB_RC_FREQ_NUM              4
#define BB_SOCK_INFO_NUM            8
#define BB_REMOTE_CMD_WAIT_MAX      8                          /**<远程基带同时配置最大数量*/

// socket option flags
#define BB_SOCK_FLAG_RX             (1 << 0)                    /**<@attention 指示socket传输方向为接收的bit位标志*/
#define BB_SOCK_FLAG_TX             (1 << 1)                    /**<@attention 指示socket传输方向为发送的bit位标志*/
#define BB_SOCK_FLAG_TROC           (1 << 2)                    /**<@attention 指示socket当基带连接时清空TX buffer中的历史数据（TX buffer reset on connect），仅芯片侧支持*/
#define BB_SOCK_FLAG_DATAGRAM       (1 << 3)                    /**<@attention 指示socket传输为数据包模式，仅host driver侧支持*/
#define BB_SOCK_FLAG_SBUS           (1 << 4)                    /**<@attention 指示socket为SBUS模式，即先清除发送队列再写入数据，以防止历史数据堆积，芯片侧支持且要求整包写入*/
#define BB_SOCK_FLAG_ENCRYPT        (1 << 5)                    /**<@attention 指示socket为加密模式*/

// chan cfg flags
#define BB_CHAN_HOP_AUTO            (1 << 0)                    /**<@note 指示使能信道自适应的bit位标志*/
#define BB_CHAN_BAND_HOP_AUTO       (1 << 1)                    /**<@note 指示使能band自适应的bit位标志*/
#define BB_CHAN_COMPLIANCE          (1 << 2)                    /**<@note 指示使能信道合规模式*/
#define BB_CHAN_MULTI_MODE          (1 << 3)                    /**<@note 指示使能多套模式*/
#define BB_CHAN_SUBCHAN_ENABLE      (1 << 4)                    /**<@note 指示使能子信道机制的bit位标志*/

// mcs cfg flags
#define BB_MCS_SWITCH_ENABLE        (1 << 0)                    /**<@note 指示使能MCS切换的bit位标志*/
#define BB_MCS_SWITCH_AUTO          (1 << 1)                    /**<@note 指示使能MCS自适应的bit位标志*/

// bb ioctl request definition
#define BB_REQ_CFG                  0                           /**<定义bb_ioctl配置大类*/
#define BB_REQ_GET                  1                           /**<定义bb_ioctl读取大类*/
#define BB_REQ_SET                  2                           /**<定义bb_ioctl设置大类*/
#define BB_REQ_CB                   3                           /**<定义bb_callback设置大类*/
#define BB_REQ_SOCKET               4                           /**<定义bb_socket设置大类*/
#define BB_REQ_DBG                  5                           /**<定义bb_debug设置大类*/
#define BB_REQ_REMOTE               6                           /**<定义bb_ioctl远程大类*/
#define BB_REQ_RPC                  10                          /**<定义bb_ioctl rpc 大类,用于*/
#define BB_REQ_RPC_IOCTL            11                          /**<定义bb_ioctl rpc 内部ioctl大类*/
#define BB_REQ_PLAT_CTL             12                          /**<定义bb_ioctl plat 控制 内部ioctl大类*/

#define BB_REQUEST(type, order)     ((type) << 24 | (order))    /**<定义bb_ioctl请求类型的生成宏*/
#define BB_REQUEST_TYPE(req)        ((req) >> 24)               /**<定义从请求类型获取请求大类*/

// bb ioctl request type - configure
#define BB_CFG_AP_BASIC                     BB_REQUEST(BB_REQ_CFG, 0)   /**<@attention AP角色的基本配置命令字*/
#define BB_CFG_DEV_BASIC                    BB_REQUEST(BB_REQ_CFG, 1)   /**<@attention DEV角色的基本配置命令字*/
#define BB_CFG_CHANNEL                      BB_REQUEST(BB_REQ_CFG, 2)   /**<@attention 信道配置命令字*/
#define BB_CFG_CANDIDATES                   BB_REQUEST(BB_REQ_CFG, 3)   /**<@attention AP候选人配置命令字*/
#define BB_CFG_USER_PARA                    BB_REQUEST(BB_REQ_CFG, 4)   /**<@attention 基带用户参数配置命令字*/
#define BB_CFG_SLOT_RX_MCS                  BB_REQUEST(BB_REQ_CFG, 5)   /**<@attention 基于SLOT的MCS策略配置命令字*/
#define BB_CFG_ANY_CHANNEL                  BB_REQUEST(BB_REQ_CFG, 6)   /**<@attention 任意工作信道配置命令字，依赖于功能宏BB_CONFIG_ENABLE_ANY_CHANNEL*/
#define BB_CFG_DISTC                        BB_REQUEST(BB_REQ_CFG, 7)   /**<@attention 基带测距功能命令字*/
#define BB_CFG_AP_SYNC_MODE                 BB_REQUEST(BB_REQ_CFG, 9)   /**<@attention 配置AP同步模式，默认为非同步模式*/
#define BB_CFG_BR_HOP_POLICY                BB_REQUEST(BB_REQ_CFG, 10)  /**<@attention 配置BR的跳频策略*/
#define BB_CFG_PWR_BASIC                    BB_REQUEST(BB_REQ_CFG, 11)  /**<@attention 配置功率参数*/
#define BB_CFG_RC_HOP_POLICY                BB_REQUEST(BB_REQ_CFG, 12)  /**<@attention 配置选择性跳频策略*/
#define BB_CFG_POWER_SAVE                   BB_REQUEST(BB_REQ_CFG, 13)  /**<@attention 配置节能模式*/
#define BB_CFG_LNA                          BB_REQUEST(BB_REQ_CFG, 14)  /**<@attention 配置LNA策略*/
#define BB_CFG_RF_POLICY                    BB_REQUEST(BB_REQ_CFG, 15)  /**<@attention 配置射频B路自适应策略*/
#define BB_CFG_SHARE_SLOT                   BB_REQUEST(BB_REQ_CFG, 16)  /**<@attention 配置共享时隙机制*/
#define BB_CFG_AGC                          BB_REQUEST(BB_REQ_CFG, 17)  /**<@attention 配置扫频校准参数*/
#define BB_CFG_AGIN                         BB_REQUEST(BB_REQ_CFG, 18)  /**<@attention 配置RSSI校准参数*/
#define BB_CFG_PWR_BASIC_EX                 BB_REQUEST(BB_REQ_CFG, 19)  /**<@attention 配置扩展功率参数*/
#define BB_CFG_RF_CALI                      BB_REQUEST(BB_REQ_CFG, 20)  /**<@attention 配置RF校准增益>*/

// bb ioctl request type - get
#define BB_GET_STATUS                       BB_REQUEST(BB_REQ_GET, 0)   /**<@attention 读取基带工作状态命令字*/
#define BB_GET_PAIR_RESULT                  BB_REQUEST(BB_REQ_GET, 1)   /**<@attention 读取配对命令结果命令字*/
#define BB_GET_AP_MAC                       BB_REQUEST(BB_REQ_GET, 2)   /**<@attention DEV读取目标AP的MAC命令字*/
#define BB_GET_CANDIDATES                   BB_REQUEST(BB_REQ_GET, 3)   /**<@attention AP读取指定SLOT的候选人命令字*/
#define BB_GET_USER_QUALITY                 BB_REQUEST(BB_REQ_GET, 4)   /**<@attention 读取物理用户信号质量*/
#define BB_GET_DISTC_RESULT                 BB_REQUEST(BB_REQ_GET, 5)   /**<@attention 读取测距结果*/
#define BB_GET_MCS                          BB_REQUEST(BB_REQ_GET, 6)   /**<@attention 读取指定SLOT的MCS及理论吞吐率*/
#define BB_GET_POWER_MODE                   BB_REQUEST(BB_REQ_GET, 7)   /**<@attention 读取功率开、闭环模式*/
#define BB_GET_CUR_POWER                    BB_REQUEST(BB_REQ_GET, 8)   /**<@attention 读取当前发射功率值*/
#define BB_GET_POWER_AUTO                   BB_REQUEST(BB_REQ_GET, 9)   /**<@attention 当前功率自适应是否开启*/
#define BB_GET_CHAN_INFO                    BB_REQUEST(BB_REQ_GET, 10)  /**<@attention 获取信道相关信息，如信道列表、周期扫频结果等*/
#define BB_GET_PEER_QUALITY                 BB_REQUEST(BB_REQ_GET, 11)  /**<@attention 获取对端信号质量（数据通道）*/
#define BB_GET_AP_TIME                      BB_REQUEST(BB_REQ_GET, 12)  /**<@attention 获取AP时间戳*/
#define BB_GET_BAND_INFO                    BB_REQUEST(BB_REQ_GET, 13)  /**<@attention 获取频段信息*/
#define BB_GET_REMOTE                       BB_REQUEST(BB_REQ_GET, 14)  /**<@attention 获取通讯对端的配置*/
#define BB_GET_RF                           BB_REQUEST(BB_REQ_GET, 15)  /**<@attention 获取RF状态*/
#define BB_GET_SOCK_INFO                    BB_REQUEST(BB_REQ_GET, 16)  /**<@attention 获取系统socket的信息*/
#define BB_GET_THROUGHPUT                   BB_REQUEST(BB_REQ_GET, 17)  /**<@attention 获取指定SLOT的实时吞吐率*/

#define BB_GET_REG                          BB_REQUEST(BB_REQ_GET, 100) /**<@note 基带寄存器读取命令字，本类型用于调试诊断*/
#define BB_GET_CFG                          BB_REQUEST(BB_REQ_GET, 101) /**<@note 基带配置文件读取命令字*/
#define BB_GET_DBG_MODE                     BB_REQUEST(BB_REQ_GET, 102) /**<@note 获取SDK debug模式状态*/
#define BB_GET_HARDWARE_VERSION             BB_REQUEST(BB_REQ_GET, 103) /**<@note 软硬件版本*/
#define BB_GET_FIRMWARE_VERSION             BB_REQUEST(BB_REQ_GET, 104) /**<@note 固件版本*/
#define BB_GET_SYS_INFO                     BB_REQUEST(BB_REQ_GET, 105) /**<@note 获取系统信息，如运行时间，软硬件版本等*/
#define BB_GET_USER_INFO                    BB_REQUEST(BB_REQ_GET, 106) /**<@note 获取基带用户基带信息*/
#define BB_GET_1V1_INFO                     BB_REQUEST(BB_REQ_GET, 107) /**<@note 获取基带用户基带信息*/
#define BB_GET_POWER_OFFSET_SAVE            BB_REQUEST(BB_REQ_GET, 108) /**<@note 读取功率补偿，用于校验功率测试*/
#define BB_GET_FACTORY_POWER_OFFSET_SAVE    BB_REQUEST(BB_REQ_GET, 109) /**<@note 读取flash中功率补偿，用于校验功率测试*/
#define BB_GET_POWER_SAVE_MODE              BB_REQUEST(BB_REQ_GET, 110) /**<@note 1V1模式低功耗策略模式*/
#define BB_GET_POWER_SAVE                   BB_REQUEST(BB_REQ_GET, 111) /**<@note 1V1模式低功耗手动周期*/
#define BB_GET_RUN_SYS                      BB_REQUEST(BB_REQ_GET, 112) /**<@note 获取系统当前运行app*/
#define BB_GET_POWER_OFFSET2                BB_REQUEST(BB_REQ_GET, 113) /**<@note 获取功率补偿值*/
#define BB_GET_CUSTOMER_KEY                 BB_REQUEST(BB_REQ_GET, 114) /**<@note 获取customer key*/
#define BB_GET_BOOT_REASON                  BB_REQUEST(BB_REQ_GET, 115) /**<@note 获取重启原因*/
#define BB_GET_CLEAN_MODE_MSG_CHECK         BB_REQUEST(BB_REQ_GET, 116) /**<@note 检测msg在纯净模式下是否合法*/
#define BB_GET_RECOVERY_MODE                BB_REQUEST(BB_REQ_GET, 117) /**<@note 当前是否为恢复模式*/
#define BB_GET_DEV_CONNECT_SLOT             BB_REQUEST(BB_REQ_GET, 118) /**<@note 获取dev当前connect的slot*/
#define BB_GET_MCS_MODE                     BB_REQUEST(BB_REQ_GET, 119) /**<@note 读取指定SLOT的MCS模式*/
#define BB_GET_BANDWIDTH_MODE               BB_REQUEST(BB_REQ_GET, 120) /**<@note 读取指定SLOT的频宽控制模式*/

#define BB_GET_PRJ_DISPATCH                 BB_REQUEST(BB_REQ_GET, 200) /**<@note 二级GET命令分发*/

// bb ioctl request type - set
#define BB_SET_EVENT_SUBSCRIBE              BB_REQUEST(BB_REQ_SET, 0)   /**<@attention 事件订阅类型命令字*/
#define BB_SET_EVENT_UNSUBSCRIBE            BB_REQUEST(BB_REQ_SET, 1)   /**<@attention 事件反订阅类型命令字*/
#define BB_SET_PAIR_MODE                    BB_REQUEST(BB_REQ_SET, 2)   /**<@attention 设置指定SLOT进入配对模式命令字*/
#define BB_SET_AP_MAC                       BB_REQUEST(BB_REQ_SET, 3)   /**<@attention DEV设置AP的MAC命令字*/
#define BB_SET_CANDIDATES                   BB_REQUEST(BB_REQ_SET, 4)   /**<@attention AP设置候选人命令字*/
#define BB_SET_CHAN_MODE                    BB_REQUEST(BB_REQ_SET, 5)   /**<@attention 设置信道工作模式*/
#define BB_SET_CHAN                         BB_REQUEST(BB_REQ_SET, 6)   /**<@attention 设置信道*/
#define BB_SET_POWER_MODE                   BB_REQUEST(BB_REQ_SET, 7)   /**<@attention 设置功率开、闭环模式*/
#define BB_SET_POWER                        BB_REQUEST(BB_REQ_SET, 8)   /**<@attention 设置发射功率值*/
#define BB_SET_POWER_AUTO                   BB_REQUEST(BB_REQ_SET, 9)   /**<@attention 使能功率自适应*/
#define BB_SET_HOT_UPGRADE_WRITE            BB_REQUEST(BB_REQ_SET, 10)  /**<@note 基带热升级命令*/
#define BB_SET_HOT_UPGRADE_CRC32            BB_REQUEST(BB_REQ_SET, 11)  /**<@note 基带热升级校验命令*/
#define BB_SET_MCS_MODE                     BB_REQUEST(BB_REQ_SET, 12)  /**<@note 设置MCS控制模式手动、自动*/
#define BB_SET_MCS                          BB_REQUEST(BB_REQ_SET, 13)  /**<@note 设置MCS挡位，仅在手动模式下支持*/
#define BB_SET_SYS_REBOOT                   BB_REQUEST(BB_REQ_SET, 14)  /**<@note 系统重启*/
#define BB_SET_MASTER_DEV                   BB_REQUEST(BB_REQ_SET, 15)  /**<@note 设置导演模式下的主DEV设备，仅导演模式AP侧支持*/
#define BB_SET_FRAME_CHANGE                 BB_REQUEST(BB_REQ_SET, 16)  /**<@note 运行中帧结构改变(仅1V1模式)*/
#define BB_SET_COMPLIANCE_MODE              BB_REQUEST(BB_REQ_SET, 17)  /**<@note 设置频点合规模式*/
#define BB_SET_BAND_MODE                    BB_REQUEST(BB_REQ_SET, 18)  /**<@note 设置频段切换模式*/
#define BB_SET_BAND                         BB_REQUEST(BB_REQ_SET, 19)  /**<@note 设置工作频段*/
#define BB_FORCE_CLS_SOCKET_ALL             BB_REQUEST(BB_REQ_SET, 20)  /**<@note 强制关闭所有socket 可能会造成socket信息不同步*/
#define BB_SET_REMOTE                       BB_REQUEST(BB_REQ_SET, 21)  /**<@note 设置通讯对端的配置*/
#define BB_SET_BANDWIDTH                    BB_REQUEST(BB_REQ_SET, 22)  /**<@note 1V1模式下手动改变bandwidth*/
#define BB_SET_DFS                          BB_REQUEST(BB_REQ_SET, 23)  /**<@note 设置DFS检测*/
#define BB_SET_RF                           BB_REQUEST(BB_REQ_SET, 24)  /**<@note RF收发开关动态控制*/
#define BB_SET_POWER_SAVE_MODE              BB_REQUEST(BB_REQ_SET, 25)  /**<@note 1V1模式低功耗策略模式*/
#define BB_SET_POWER_SAVE                   BB_REQUEST(BB_REQ_SET, 26)  /**<@note 1V1模式低功耗手动周期*/
#define BB_SET_MCS_RANGE                    BB_REQUEST(BB_REQ_SET, 27)  /**<@note 设置mcs配置表中生效的mcs范围*/
#define BB_SET_LNA_MODE                     BB_REQUEST(BB_REQ_SET, 28)  /**<@note 设置LNA策略模式*/
#define BB_SET_LNA                          BB_REQUEST(BB_REQ_SET, 29)  /**<@note 设置LNA bypass状态*/
#define BB_SET_POWER_RANGE                  BB_REQUEST(BB_REQ_SET, 30)  /**<@note 设置power范围*/
#define BB_FORCE_CLS_SOCKET                 BB_REQUEST(BB_REQ_SET, 31)  /**<@note 强制关闭一个socket 可能会造成socket信息不同步*/
#define BB_SET_BANDWIDTH_MODE               BB_REQUEST(BB_REQ_SET, 32)  /**<@note 设置频宽控制模式*/
#define BB_SET_LOCAL_MAC                    BB_REQUEST(BB_REQ_SET, 33)  /**<@note 设置本机运行时MAC*/
#define BB_SET_CUSTOMER_KEY                 BB_REQUEST(BB_REQ_SET, 34)  /**<@note 写入customer key*/
#define BB_SET_CHAN_PWR_PLUS                BB_REQUEST(BB_REQ_SET, 35)  /**<@note 设置频点粒度的功率最大值*/
#define BB_SET_ENTER_RECOVERY_MODE          BB_REQUEST(BB_REQ_SET, 36)  /**<@note 重启并进入恢复模式*/
#define BB_SET_FORCE_UPD_SOCKET_ENC         BB_REQUEST(BB_REQ_SET, 37)  /**<@note 强制更新socket 加密配置*/

// 以下为调试诊断类命令字
#define BB_SET_REG                          BB_REQUEST(BB_REQ_SET, 100) /**<@note 基带寄存器写入命令字，本类型用于调试诊断*/
#define BB_SET_CFG                          BB_REQUEST(BB_REQ_SET, 101) /**<@note 基带配置文件写入命令字*/
#define BB_RESET_CFG                        BB_REQUEST(BB_REQ_SET, 102) /**<@note reset基带配置文件命令字*/
#define BB_SET_PLOT                         BB_REQUEST(BB_REQ_SET, 103) /**<@note 设置基带plot debug参数*/
#define BB_SET_DBG_MODE                     BB_REQUEST(BB_REQ_SET, 104) /**<@note 设置基带SDK进入debug mode（软件不工作）*/
#define BB_SET_FREQ                         BB_REQUEST(BB_REQ_SET, 105) /**<@note 设置物理用户的发送或接收频率*/
#define BB_SET_TX_MCS                       BB_REQUEST(BB_REQ_SET, 106) /**<@note 设置物理用户的发送MCS*/
#define BB_SET_RESET                        BB_REQUEST(BB_REQ_SET, 107) /**<@note 基带RESET命令*/
#define BB_SET_TX_PATH                      BB_REQUEST(BB_REQ_SET, 108) /**<@note 设置无线发射通道，用于功率测试*/
#define BB_SET_RX_PATH                      BB_REQUEST(BB_REQ_SET, 109) /**<@note 设置无线接收通道，用于灵敏度测试*/
#define BB_SET_POWER_OFFSET                 BB_REQUEST(BB_REQ_SET, 110) /**<@note 设置功率补偿，用于功率测试*/
#define BB_SET_POWER_TEST_MODE              BB_REQUEST(BB_REQ_SET, 111) /**<@note 进入产测功率测试模式*/
#define BB_SET_SENSE_TEST_MODE              BB_REQUEST(BB_REQ_SET, 112) /**<@note 进入产测灵敏度测试模式*/
#define BB_SET_ORIG_CFG                     BB_REQUEST(BB_REQ_SET, 113) /**<@note 加载原始镜像配置，仅在基带IDLE状态支持*/
#define BB_SET_SINGLE_TONE                  BB_REQUEST(BB_REQ_SET, 114) /**<@note 单音信号，需要先进入debug模式*/
#define BB_SET_PURE_SLOT                    BB_REQUEST(BB_REQ_SET, 115) /**<@note 纯图传模式，注意暂不支持退出*/
#define BB_SET_FACTORY_POWER_OFFSET_SAVE    BB_REQUEST(BB_REQ_SET, 116) /**<@note 产测功率校准保存指令*/
#define BB_SET_TR_SWITCH                    BB_REQUEST(BB_REQ_SET, 150) /**<@note 项目注册tr switch回调钩子*/

#define BB_SET_PRJ_DISPATCH                 BB_REQUEST(BB_REQ_SET, 200) /**<@note 二级SET命令分发*/
#define BB_SET_PRJ_DISPATCH2                BB_REQUEST(BB_REQ_SET, 201) /**<@note 二级SET命令分发(USB),1K缓存*/
#define BB_SET_PRJ_DISPATCH2_UART           BB_REQUEST(BB_REQ_SET, 202) /**<@note 二级SET命令分发(UART),1K缓存*/
#define BB_SET_PRJ_DISPATCH2_SDIO           BB_REQUEST(BB_REQ_SET, 203) /**<@note 二级SET命令分发(SDIO),1K缓存*/

// 以下为特殊控制
#define BB_REMOTE_IOCTL_REQ                 BB_REQUEST(BB_REQ_REMOTE, 0) /**<@note 远端命令分发*/

#define BB_START_REQ                        BB_REQUEST(BB_REQ_RPC_IOCTL, 0)
#define BB_STOP_REQ                         BB_REQUEST(BB_REQ_RPC_IOCTL, 1)
#define BB_INIT_REQ                         BB_REQUEST(BB_REQ_RPC_IOCTL, 2)
#define BB_DEINIT_REQ                       BB_REQUEST(BB_REQ_RPC_IOCTL, 3)

#define BB_RPC_GET_LIST                     BB_REQUEST(BB_REQ_RPC, 0) /**<@note 获取8030可用列表*/
#define BB_RPC_SEL_ID                       BB_REQUEST(BB_REQ_RPC, 1) /**<@note 选择指定的8030进行通讯*/
#define BB_RPC_GET_MAC                      BB_REQUEST(BB_REQ_RPC, 2) /**<@note 获取指定8030的mac*/
#define BB_RPC_GET_HOTPLUG_EVENT            BB_REQUEST(BB_REQ_RPC, 3) /**<@note 获取设备上下线通知*/
#define BB_RPC_SOCK_BUF_STA                 BB_REQUEST(BB_REQ_RPC, 4) /**<@note 查询socket buff状态*/
#define BB_RPC_TEST                         BB_REQUEST(BB_REQ_RPC, 5) /**<@note 测试服务器连通性*/

#define BB_RPC_SERIAL_LIST                  BB_REQUEST(BB_REQ_PLAT_CTL, 0) /**<@note 获取串口列表*/
#define BB_RPC_SERIAL_SETUP                 BB_REQUEST(BB_REQ_PLAT_CTL, 1) /**<@note 设置串口*/
#define BB_RPC_SET_DEBUG_LV                 BB_REQUEST(BB_REQ_PLAT_CTL, 2) /**<@note 设置daemon 打印等级*/
#define BB_RPC_GET_DEBUG_LV                 BB_REQUEST(BB_REQ_PLAT_CTL, 3) /**<@note 获取daemon 打印等级*/
#define BB_RPC_HOST_EXEC                    BB_REQUEST(BB_REQ_PLAT_CTL, 4) /**<@note host 执行程序*/

/** @} */  /** <!-- ==== Macro Definition end ==== */

/******************************* Structure Definition ***************************/
/** \addtogroup      BB_API */
/** @{ */  /** <!-- [BB_API] */

/**定义基带MAC地址*/
typedef struct {
    uint8_t addr[BB_MAC_LEN];                                   /**<@note MAC地址字节*/
} bb_mac_t;

/**定义基带通用的方向属性*/
typedef enum {
    BB_DIR_TX,                                                  /**<@note 发送方向*/
    BB_DIR_RX,                                                  /**<@note 接收方向*/
    BB_DIR_MAX
} bb_dir_e;

/**定义debug 通道 msgid*/
typedef enum {
    BB_DBG_CLIENT_CHANGE,
    BB_DBG_DATA,
    BB_DBG_MAX,
} bb_debug_id_e;

/**定义8030基带工作频率*/
typedef enum {
    BB_CLK_100M_SEL = 0,                                        /**<@note 100M工作频率*/
    BB_CLK_200M_SEL = 1                                         /**<@note 200M工作频率*/
} bb_clk_sel_e;

/**定义基带基本角色*/
typedef enum {
    BB_ROLE_AP,                                                 /**<@note 网络根节点设备*/
    BB_ROLE_DEV,                                                /**<@note 网络叶节点设备*/
    BB_ROLE_MAX
} bb_role_e;

/**定义基带工作模式*/
typedef enum {
    BB_MODE_SINGLE_USER,                                        /**<@note 单用户模式*/
    BB_MODE_MULTI_USER,                                         /**<@note 多用户模式*/
    BB_MODE_RELAY,                                              /**<@note 中继模式（暂不支持）*/
    BB_MODE_DIRECTOR,                                           /**<@note 导演模式-1对多可靠广播模式，不支持MCS负数*/
    BB_MODE_MAX
} bb_mode_e;

/**定义逻辑SLOT类型*/
typedef enum {
    BB_SLOT_0 = 0,                                              /**<@note 逻辑位置SLOT0*/
    BB_SLOT_AP = 0,                                             /**<@attention DEV侧用于标识AP的逻辑位置*/
    BB_SLOT_1,                                                  /**<@note 逻辑位置SLOT1*/
    BB_SLOT_2,                                                  /**<@note 逻辑位置SLOT2*/
    BB_SLOT_3,                                                  /**<@note 逻辑位置SLOT3*/
    BB_SLOT_4,                                                  /**<@note 逻辑位置SLOT4*/
    BB_SLOT_5,                                                  /**<@note 逻辑位置SLOT5*/
    BB_SLOT_6,                                                  /**<@note 逻辑位置SLOT6*/
    BB_SLOT_7,                                                  /**<@note 逻辑位置SLOT7*/
    BB_SLOT_MAX
} bb_slot_e;

/**定义基带物理用户类型*/
typedef enum {
    BB_USER_0 = 0,                                              /**<@note 物理用户0*/
    BB_USER_1,                                                  /**<@note 物理用户1*/
    BB_USER_2,                                                  /**<@note 物理用户2*/
    BB_USER_3,                                                  /**<@note 物理用户3*/
    BB_USER_4,                                                  /**<@note 物理用户4*/
    BB_USER_5,                                                  /**<@note 物理用户5*/
    BB_USER_6,                                                  /**<@note 物理用户6*/
    BB_USER_7,                                                  /**<@note 物理用户7*/
#if 0
    BB_USER_DFS = BB_USER_7,                                    /**<@note 物理用户DFS*/
#endif
    BB_USER_BR_CS,                                              /**<@note 物理用户BR/CS*/
    BB_USER_BR2_CS2,                                            /**<@note 物理用户BR2/CS2*/
    BB_DATA_USER_MAX,                                           /**<@attention 最大数据用户标识，用于软件编程辅助*/
    BB_USER_SWEEP = BB_DATA_USER_MAX,                           /**<@note 物理长扫频用户*/
    BB_USER_SWEEP_SHORT,                                        /**<@note 物理短扫频用户*/
    BB_USER_MAX
} bb_user_e;

/**定义基带MCS类型*/
typedef enum {
    BB_PHY_MCS_NEG_2,                                           /**<@note BPSK   CR_1/2 REP_4 单流*/
    BB_PHY_MCS_NEG_1,                                           /**<@note BPSK   CR_1/2 REP_2 单流*/
    BB_PHY_MCS_0,                                               /**<@note BPSK   CR_1/2 REP_1 单流*/
    BB_PHY_MCS_1,                                               /**<@note BPSK   CR_2/3 REP_1 单流*/
    BB_PHY_MCS_2,                                               /**<@note BPSK   CR_3/4 REP_1 单流*/
    BB_PHY_MCS_3,                                               /**<@note QPSK   CR_1/2 REP_1 单流*/
    BB_PHY_MCS_4,                                               /**<@note QPSK   CR_2/3 REP_1 单流*/
    BB_PHY_MCS_5,                                               /**<@note QPSK   CR_3/4 REP_1 单流*/
    BB_PHY_MCS_6,                                               /**<@note 16QAM  CR_1/2 REP_1 单流*/
    BB_PHY_MCS_7,                                               /**<@note 16QAM  CR_2/3 REP_1 单流*/
    BB_PHY_MCS_8,                                               /**<@note 16QAM  CR_3/4 REP_1 单流*/
    BB_PHY_MCS_9,                                               /**<@note 64QAM  CR_1/2 REP_1 单流*/
    BB_PHY_MCS_10,                                              /**<@note 64QAM  CR_2/3 REP_1 单流*/
    BB_PHY_MCS_11,                                              /**<@note 64QAM  CR_3/4 REP_1 单流*/
    BB_PHY_MCS_12,                                              /**<@note 256QAM CR_1/2 REP_1 单流*/
    BB_PHY_MCS_13,                                              /**<@note 256QAM CR_2/3 REP_1 单流*/
    BB_PHY_MCS_14,                                              /**<@note QPSK   CR_1/2 REP_1 双流*/
    BB_PHY_MCS_15,                                              /**<@note QPSK   CR_2/3 REP_1 双流*/
    BB_PHY_MCS_16,                                              /**<@note QPSK   CR_3/4 REP_1 双流*/
    BB_PHY_MCS_17,                                              /**<@note 16QAM  CR_1/2 REP_1 双流*/
    BB_PHY_MCS_18,                                              /**<@note 16QAM  CR_2/3 REP_1 双流*/
    BB_PHY_MCS_19,                                              /**<@note 16QAM  CR_3/4 REP_1 双流*/
    BB_PHY_MCS_20,                                              /**<@note 64QAM  CR_1/2 REP_1 双流*/
    BB_PHY_MCS_21,                                              /**<@note 64QAM  CR_2/3 REP_1 双流*/
    BB_PHY_MCS_22,                                              /**<@note 64QAM  CR_3/4 REP_1 双流*/
    BB_PHY_MCS_MAX
} bb_phy_mcs_e;

/**定义多用户模式下的SLOT工作模式*/
typedef enum {
    BB_SLOT_MODE_FIXED = 0,                                     /**<@note SLOT数量固定不变*/
    BB_SLOT_MODE_DYNAMIC                                        /**<@note 根据DEV的接入与退出，动态的调整帧结构*/
} bb_slot_mode_e;

/**定义SDK链路层设备状态*/
typedef enum {
    BB_LINK_STATE_IDLE,                                         /**<@note 空闲状态*/
    BB_LINK_STATE_LOCK,                                         /**<@note 链路信道与对方Lock*/
    BB_LINK_STATE_CONNECT,                                      /**<@note 链路信道、数据信道都与对方Lock*/
    BB_LINK_STATE_MAX
} bb_link_state_e;

/**定义SDK的callback事件类型*/
typedef enum {
    BB_EVENT_LINK_STATE = 0,                                    /**<@note 链路状态发生变化事件*/
    BB_EVENT_MCS_CHANGE,                                        /**<@note MCS等级发生变化事件*/
    BB_EVENT_CHAN_CHANGE,                                       /**<@note 工作信道发生变化事件*/
    BB_EVENT_PLOT_DATA,                                         /**<@note 用于debug的异步信号质量Plot数据*/
    BB_EVENT_FRAME_START,                                       /**<@note 每一个基带帧开始的事件*/
    BB_EVENT_OFFLINE,                                           /**<@note 当设备离线时获得通知           仅host侧有效*/
    BB_EVENT_PRJ_DISPATCH,                                      /**<@note 项目自定义事件分发*/
    BB_EVENT_PAIR_RESULT,                                       /**<@note 配对结果事件分发*/
    BB_EVENT_PRJ_DISPATCH2,                                     /**<@note 项目自定义事件分发2(用于USB RPC)*/
    BB_EVENT_MCS_CHANGE_END,                                    /**<@note MCS等级发生变化结束事件*/
    BB_EVENT_PRJ_DISPATCH2_UART,                                /**<@note 项目自定义事件分发2(用于UART RPC)*/
    BB_EVENT_PRJ_DISPATCH2_SDIO,                                /**<@note 项目自定义事件分发2(用于SDIO RPC)*/
    BB_EVENT_BW_CHANGE,                                         /**<@note BW发生变化事件*/
    BB_EVENT_MAX
} bb_event_e;

/**定义SDK内统一信道类型*/
typedef uint16_t chan_t;                                        /**<@note 统一信道=子信道(高Byte)+主信道(低Byte)*/

/**定义8030物理用户信号质量结构体*/
typedef struct {
    uint16_t snr;                                               /**<@note 物理用户接收信噪比，转化db公式：10log(snr/36)*/
    uint16_t ldpc_err;                                          /**<@note 交织块中解码错误的LDPC块个数*/
    uint16_t ldpc_num;                                          /**<@note 交织块中总的LDPC块个数*/
    uint8_t  gain_a;                                            /**<@note A路天线接收信号强度*/
    uint8_t  gain_b;                                            /**<@note B路天线接收信号强度*/
} bb_quality_t;

/**定义8030支持的的频段类型*/
typedef enum {
    BB_BAND_1G = 0,                                             /**<@note 1G: 150MHZ  ~ 1000MHZ*/
    BB_BAND_2G,                                                 /**<@note 2G: 1000MHZ ~ 4000MHZ*/
    BB_BAND_5G,                                                 /**<@note 5G: 4000MHZ ~ 7000MHZ*/
    BB_BAND_MAX
} bb_band_e;

/**定义8030射频通道类型*/
typedef enum {
    BB_RF_PATH_A = 0,                                               /**<@note 射频A路*/
    BB_RF_PATH_B,                                               /**<@note 射频B路*/
    BB_RF_PATH_MAX
} bb_rf_path_e;

/**定义当前工作频段模式*/
typedef enum {
    BB_BAND_MODE_SINGLE = 0,                                    /**<@note 单频模式*/
    BB_BAND_MODE_2G_5G,                                         /**<@note 2G和5G组合*/
    BB_BAND_MODE_1G_2G,                                         /**<@note 1G和2G组合*/
    BB_BAND_MODE_1G_5G,                                         /**<@note 1G和5G组合*/
    BB_BAND_MODE_MAX
}bb_band_mode_e;

/**定义8030支持的频宽类型*/
typedef enum {
    BB_BW_1_25M = 0,                                            /**<@note 1.25MHZ*/
    BB_BW_2_5M  = 1,                                            /**<@note 2.5MHZ*/
    BB_BW_5M    = 2,                                            /**<@note 5MHZ*/
    BB_BW_10M   = 3,                                            /**<@note 10MHZ*/
    BB_BW_20M   = 4,                                            /**<@note 20MHZ*/
    BB_BW_40M   = 5,                                            /**<@note 40MHZ*/
    BB_BW_MAX
} bb_bandwidth_e;

/**定义8030支持的交织块长度*/
typedef enum {
    BB_TIMEINTLV_LEN_3  = 0,                                    /**<@note 3  OFDM*/
    BB_TIMEINTLV_LEN_6  = 1,                                    /**<@note 6  OFDM*/
    BB_TIMEINTLV_LEN_12 = 2,                                    /**<@note 12 OFDM*/
    BB_TIMEINTLV_LEN_24 = 3,                                    /**<@note 24 OFDM*/
    BB_TIMEINTLV_LEN_48 = 4,                                    /**<@note 48 OFDM*/
    BB_TIMEINTLV_LEN_MAX
} bb_timeintlv_len_e;

/**定义8030交织类型*/
typedef enum {
    BB_TIMEINTLV_OFF = 0,                                       /**<@note 交织块时域不交织*/
    BB_TIMEINTLV_ON  = 1,                                       /**<@note 交织块时域交织*/
    BB_TIMEINTLV_ENABLE_MAX
} bb_timeintlv_enable_e;

/**定义8030支持的交织块数量*/
typedef enum {
    BB_TIMEINTLV_1_BLOCK = 0,                                   /**<@note 一个交织块*/
    BB_TIMEINTLV_2_BLOCK = 1,                                   /**<@note 两个交织块*/
    BB_TIMEINTLV_NUM_MAX
} bb_timeintlv_num_e;

/**定义8030 slot内是否需要payload*/
typedef enum {
    BB_PAYLOAD_ON   = 0,                                        /**<@attention slot内有payload*/
    BB_PAYLOAD_OFF  = 1,                                        /**<@attention slot内没有payload*/
    BB_PAYLOAD_MAX
} bb_payload_e;

/**定义8030 FCH的长度模式*/
typedef enum {
    BB_FCH_INFO_96BITS  = 0,                                    /**<@attention 96bits FCH, 默认推荐*/
    BB_FCH_INFO_48BITS  = 1,                                    /**<@attention 48bits FCH, 谨慎使用*/
    BB_FCH_INFO_192BITS = 2,                                    /**<@attention 192bits FCH*/
    BB_FCH_INFO_MAX
} bb_fch_info_len_e;

/**定义8030支持的RF发送模式*/
typedef enum {
    BB_TX_1TX          = 0,                                     /**<@note 单天线发送信号*/
    BB_TX_2TX_STBC     = 1,                                     /**<@note 双天线发送相关信号*/
    BB_TX_2TX_MIMO     = 2,                                     /**<@note 双天线发送独立信号*/
    BB_TX_MODE_MAX
} bb_tx_mode_e;

/**定义8030支持的RF接收模式*/
typedef enum {
    BB_RX_1T1R         = 0,                                     /**<@note 单天线接收信号*/
    BB_RX_1T2R         = 1,                                     /**<@note 双天线接收对端的单发信号*/
    BB_RX_2T2R_STBC    = 2,                                     /**<@note 双天线接收对方的双发STBC信号*/
    BB_RX_2T2R_MIMO    = 3,                                     /**<@note 双天线接收对方的双发MIMO信号*/
    BB_RX_MODE_MAX
} bb_rx_mode_e;

/**定义8030功率模式*/
typedef enum {
    BB_PHY_PWR_OPENLOOP = 0,                                    /**<@note 开环模式*/
    BB_PHY_PWR_CLOSELOOP,                                       /**<@note 闭环模式*/
} bb_phy_pwr_mode_e;

/**定义8030 BR跳频模式*/
typedef enum {
    BB_BR_HOP_MODE_FIXED,                                       /**<@note 固定模式*/
    BB_BR_HOP_MODE_FOLLOW_UP_CHAN,                              /**<@note BR信道与AP的上行信道保持同步*/
    BB_BR_HOP_MODE_HOP_ON_IDLE,                                 /**<@note BR信道在无DEV连接时，周期性改变*/
    BB_BR_HOP_MODE_MAX
} bb_br_hop_mode_e;

typedef enum {
    BB_BR_HOP_ON_IDLE_ROUND_HOP,                                /**<@note BR hop on idle循环跳频*/
    BB_BR_HOP_ON_IDLE_FS_RESULT                                 /**<@note BR hop on idle基于扫频结果跳频*/
} bb_br_hop_on_idle_policy_e;

/**定义8030 auto band的切换类型*/
typedef enum {
    BB_BAND_HOP_2G_2_5G = 0,
    BB_BAND_HOP_5G_2_2G,
    BB_BAND_HOP_ITEM_MAX
} bb_band_hop_item_e;

/**定义dfs认证类型*/
typedef enum {
    BB_DFS_TYPE_FCC = 0,
    BB_DFS_TYPE_CE
} bb_dfs_type_e;

/**定义dfs操作类型*/
typedef enum {
    BB_DFS_CONF_GET = 0,
    BB_DFS_CONF_SET,
    BB_DFS_EVENT
} bb_dfs_sub_cmd_e;

/**定义mcs自动切换策略*/
typedef enum {
    BB_MCS_AUTO_MODE_POLICY_DELAY,
    BB_MCS_AUTO_MODE_POLICY_THROUGHPUT,
    BB_MCS_AUTO_MODE_POLICY_MAX
} bb_mcs_auto_mode_policy_e;

#if (BB_CONFIG_AUTH_MODE_CE == 1)
/**定义CE认证模式*/
typedef enum {
    BB_CE_AUTH_MODE_INVALID,
    BB_CE_AUTH_MODE_CHANGE_CHANNEL,                             // change channel safe
    BB_CE_AUTH_MODE_CHANGE_CHANNEL_FORSE,                       // change as soon as possible
    BB_CE_AUTH_MODE_CLOSE_PA,
    BB_CE_AUTH_MODE_MAX,
} bb_ce_auth_mode_e;
#endif

/**定义链路状态变化的事件结构体*/
typedef struct {
    uint8_t         slot;                                       /**<@note 链路状态发生位置, 类型：bb_slot_e*/
    uint8_t         cur_state;                                  /**<@note 链路当前状态，类型：bb_link_state_e*/
    uint8_t         prev_state;                                 /**<@note 链路之前状态，类型：bb_link_state_e*/
} bb_event_link_state_t;

/**定义MCS等级变化的事件结构体*/
typedef struct {
    uint8_t         slot;                                       /**<@note MCS等级发生位置, 类型：bb_slot_e*/
    uint8_t         dir;                                        /**<@note MCS等级变化的方向, 类型：bb_dir_e*/
    uint8_t         cur_mcs;                                    /**<@note 当前MCS等级, 类型：bb_phy_mcs_e*/
    uint8_t         prev_mcs;                                   /**<@note 之前MCS等级, 类型：bb_phy_mcs_e*/
} bb_event_mcs_change_t;

/**定义频宽变化的事件结构体*/
typedef struct {
    uint8_t         slot;                                       /**<@note BW发生位置, 类型：bb_slot_e*/
    uint8_t         dir;                                        /**<@note BW变化的方向, 类型：bb_dir_e*/
    uint8_t         cur_bw;                                     /**<@note 当前BW, 类型：bb_bandwidth_e*/
    uint8_t         prev_bw;                                    /**<@note 当前BW, 类型：bb_bandwidth_e*/
} bb_event_bw_change_t;

/**定义MCS等级变化的事件结构体*/
typedef struct {
    bb_event_mcs_change_t info;
    uint32_t tx_result;
    uint8_t pkt_num;
} bb_event_mcs_change_end_t;

/**定义信道发生变化的事件结构体*/
typedef struct {
    uint8_t         slot;                                       /**<@attention 发生信道变化的位置，类型：bb_slot_e。如果dir参数为RX，则本字段无意义，即当前SDK，RX侧信道改变时，对所有SLOT同步改变*/
    uint8_t         dir;                                        /**<@note 信道变化的方向, 类型：bb_dir_e*/
    uint8_t         cur_chan;                                   /**<@note 当前信道, 类型：uint8_t 信道索引值*/
    uint8_t         prev_chan;                                  /**<@note 之前信道, 类型：uint8_t 信道索引值*/
} bb_event_chan_change_t;

typedef struct {
    uint16_t        snr;                                        /**<@note 信噪比*/
    uint16_t        ldpc_err;                                   /**<@note LDPC错误数*/
    uint16_t        ldpc_num;                                   /**<@note LDPC总数*/
    uint8_t         gain_a;                                     /**<@note A路天线GAIN*/
    uint8_t         gain_b;                                     /**<@note B路天线GAIN*/
    uint8_t         mcs_rx : 5;                                 /**<@note RX MCS*/
    uint8_t         fch_lock : 1;                               /**<@note FCH LOCK状态*/
    uint8_t         padding[23];                                /**<@note padding to 32 Bytes*/
} bb_plot_data_t;

/**定义异步Plot Data数据结构*/
typedef struct {
    uint8_t         user;
    uint8_t         plot_num;
    bb_plot_data_t  plot_data[BB_PLOT_POINT_MAX];
} bb_event_plot_data_t;

typedef struct {
    uint8_t         slot_state[BB_SLOT_MAX];                    /**<@note slot状态 类型：bb_link_stat_e*/
} bb_event_frame_start_t;

typedef struct bb_dev_info_t {
    uint8_t mac[32];
    int     maclen;
} bb_dev_info_t;

typedef struct {
    uint32_t id;
    uint32_t status;
    bb_dev_info_t bb_mac;
} bb_event_hotplug_t;

/**定义项目自定义异步事件结构体*/
typedef struct {
    uint8_t         data[256];                                  /**二级分发内容*/
} bb_event_prj_dispatch_t;

/**定义项目自定义异步事件结构体*/
typedef struct {
    uint8_t         data[1024];                                 /**二级分发内容*/
} bb_event_prj_dispatch2_t;

#define SO_USER_BASE_START 0xc0
enum ctl_opt {
    sock_opt_echo,
};

// data plane
/**定义socket的控制命令字*/
typedef enum {
    BB_SOCK_QUERY_TX_BUFF_LEN,
    BB_SOCK_QUERY_RX_BUFF_LEN,
    BB_SOCK_READ_INV_DATA,
    BB_SOCK_SET_TX_LIMIT,
    BB_SOCK_GET_TX_LIMIT,

    BB_SOCK_IOCTL_ECHO = SO_USER_BASE_START + sock_opt_echo,
    BB_SOCK_TX_LEN_GET,
    BB_SOCK_TX_LEN_RESET,
    BB_SOCK_ENC_SET,
    BB_SOCK_ENC_GET,

    BB_SOCK_IOCTL_MAX,
} bb_sock_cmd_e;

/**项目层面注册的回调函数类型*/
typedef enum {
    BB_CB_TYPE_FREQ_PWR_OFFSET,                                 /**<@attention int8_t func(int tab, int chan)*/
    BB_CB_TYPE_MAX,
} bb_cb_type_e;

/**定义socket的加密类型*/
typedef enum {
    BB_SOCK_ENCRYPT_MODE_DEFAULT,                               /**<@attention 默认加密模式，不使用key字段*/
    BB_SOCK_ENCRYPT_MODE_AES128,                                /**<@attention AES128模式，key字段使用前16字节*/
    BB_SOCK_ENCRYPT_MODE_AES256,                                /**<@attention AES256模式，key字段使用32字节*/
    BB_SOCK_ENCRYPT_MODE_MAX,
} bb_sock_encrypt_mode_t;

/**定义数传socket创建时的自定义参数*/
typedef struct bb_sock_opt_t {
    uint32_t tx_buf_size;                                       /**<@attention 用于指定发送buffer的大小，创建的socket具备TX属性时有效*/
    uint32_t rx_buf_size;                                       /**<@attention 用于指定接收buffer的大小，创建的socket具备RX属性时有效*/
    void*    tx_buf;                                            /**<@attention 仅芯片侧有效 由8030侧应用指定tx buffer的内存地址 20250205*/
    void*    rx_buf;                                            /**<@attention 仅芯片侧有效 由8030侧应用指定rx buffer的内存地址 20250205*/
    uint8_t  encrypt_mode;                                      /**<@attention 创建加密通道时指定加密模式，类型为bb_sock_encrypt_mode_t*/
    uint8_t  key[32];                                           /**<@attention 创建加密通道时指定密钥*/
} bb_sock_opt_t;

/**定义数传socket更新加密参数的自定义参数*/
typedef struct bb_sock_upd_enc_s {
    uint8_t  encrypt_en;                                        /**<@attention 更新加密通道是否使能加密模式*/
    uint8_t  encrypt_mode;                                      /**<@attention 更新加密通道指定加密模式，类型为bb_sock_encrypt_mode_t*/
    uint8_t  key[32];                                           /**<@attention 更新加密通道指定密钥*/
} bb_sock_upd_enc_t;

/**定义基本内存块*/
typedef struct {
    uint8_t* addr;                                              /**<@note 定义内存起始地址*/
    uint32_t size;                                              /**<@note 定义内存大小*/
} bb_mem_block_t;

/**定义socket query操作返回的内存信息*/
typedef struct {
    bb_mem_block_t block[2];                                    /**<@attention 基带SDK内以ringbuf管理RX数据，所以为减少内存拷贝，每个query操作最多返回两块基本内存块，如果第一个内存块为空（size=0），那么第二块必定为空*/
} bb_query_mem_t;

/**定义通用事件回调接口函数*/
typedef void (*bb_event_callback)(void* arg, void* user);  /**<@attention arg由事件类型决定，对应相应的事件结构体，user是用户注册时带入的自定义参数，需要特别注意的是，事件回调在本地是同步的，对RPC是异步的*/

#define UART_NUM_MAX  4
#define UART_LIST_MAX 32
#define UART_NAME_LEN 24
typedef struct {
    uint32_t num;
    char     uart_dev_name[1000];
} uart_list_hd;

typedef struct {
    int BaudRate; /* Baudrate at which running       */
    int ByteSize; /* Number of bits/byte, 4-8        */
    int Parity;   /* 0-4=None,Odd,Even,Mark,Space    */
    int StopBits; /* 0,1,2 = 1, 1.5, 2               */
} uart_par;

typedef enum {
    uart_add_ok = 0,
    uart_open_fail,
    uart_had_added,
} uart_msg;

typedef struct {
    char     uart_dev_name[UART_NAME_LEN];
    uart_par par;
} uart_ioctl;

/**定义socket加密头部*/
PACK(typedef struct {
    uint8_t     magic[2];         /**<<@note 魔数: 0x80 0x30 */
    uint8_t     ver;              /**<<@note 头部版本 */
    uint8_t     algo;             /**<<@note 加密算法 */
    uint8_t     mode;             /**<<@note 加密模式 */
    uint16_t    len;              /**<<@note 负载长度(来自头部) */
    uint8_t     iv[16];           /**<<@note AES/DES初始向量 */
    uint16_t    hdr_crc;          /**<<@note 头部CRC校验 */
    uint8_t     rsv[3];           /**<<@note 保留字段 */
    uint8_t     data[0];          /**<<@note 加密负载数据 */
}) bb_socket_encrypt_hdr_t;

/**定义RPC所使用的通用设备类型*/
typedef struct bb_dev_t        bb_dev_t;        ///<@attention 本类型由bb_dev_getlist接口返回，用于标识物理接口（USB总线、SDIO）上的8030设备
typedef struct bb_dev_handle_t bb_dev_handle_t; ///<@attention 本类型由bb_dev_open接口返回，用于标识打开的8030设备
typedef struct bb_host_t       bb_host_t;       ///<@attention 本类型由bb_host_connect接口返回，用于标识远程host设备
typedef struct bb_dev_info_t   bb_dev_info_t;   ///<@attention 本类型由bb_dev_getinfo接口返回，用于获取8030信息包括id/mac等
#ifdef SWIG
typedef struct bb_dev_list_t   bb_dev_list_t;
#else
typedef struct bb_dev_t *      bb_dev_list_t;
#endif
// ################# conf struct definition #####################

/**定义配置命令BB_CFG_AP_BASIC的输入参数结构*/
typedef struct {
    bb_mac_t        ap_mac;                                     /**<@note AP自己的MAC*/
    uint8_t         clock;                                      /**<@note 基带工作频率 类型：bb_clk_sel_e*/
    uint8_t         init_mcs;                                   /**<@note 初始MCS等级 类型：bb_phy_mcs_e*/
    uint8_t         sq_report;                                  /**<@note 发送无线质量给对端*/
    uint8_t         bb_mode;                                    /**<@note 基带工作模式 类型：bb_mode_e*/
    uint8_t         slot_mode;                                  /**<@note slot工作模式，1V1模式本字段无效 类型：bb_slot_mode_e*/
    uint8_t         slot_bmp;                                   /**<@note 有效slot的bitmap，1V1模式本字段无效*/
    uint8_t         compact_br;                                 /**<@note 压缩BR模式，仅1V1模式有效 类型：布尔*/
    uint8_t         ant_num;                                    /**<@note 天线数量*/
} bb_conf_ap_t;

/**定义配置命令BB_CFG_DEV_BASIC的输入参数结构*/
typedef struct {
    bb_mac_t        ap_mac;                                     /**<@note AP的MAC，即目标接入点ID*/
    uint8_t         clock;                                      /**<@note 基带工作频率 类型：bb_clk_sel_e*/
    uint8_t         init_mcs;                                   /**<@note 初始MCS等级 类型：bb_phy_mcs_e*/
    uint8_t         sq_report;                                  /**<@note 发送无线质量给对端*/
    uint8_t         ant_num;                                    /**<@note 天线数量*/
    bb_mac_t        dev_mac;                                    /**<@note DEV自己的MAC*/
} bb_conf_dev_t;

/**定义配置命令BB_CFG_AP_SYNC_MODE的输入参数结构*/
typedef struct {
    uint8_t         enable;                                     /**<@note 使能AP同步模式*/
    uint8_t         master;                                     /**<@note AP同步模式中Master或Slave*/
    uint8_t         pin_group;                                  /**<@note PIN GROUP 类型：AR_PIN_GROUP*/
    uint8_t         pin_port;                                   /**<@note pin port，PIN GROUP中的port位置*/
} bb_conf_ap_sync_mode_t;

/**定义配置命令BB_CFG_BR_HOP_POLICY的输入参数结构*/
typedef struct {
    uint8_t         hop_mode;                                   /**<@note BR跳频模式 */
    uint8_t         hop_on_idle_dir_bmp;                        /**<@note BR跳频(IDLE)方向BMP*/
    uint8_t         hop_on_idle_cnt;                            /**<@note BR跳频(IDLE)帧计数*/
    uint8_t         hop_on_idle_policy;                         /**<@note BR跳频(IDLE)依据*/
    uint8_t         hop_on_idle_band_switch;                    /**<@note BR跳频(IDLE)频段切换*/
    uint8_t         chan_num;                                   /**<@note 指定chan_freq中的频点数量*/
    uint8_t         chan_id;                                    /**<@note 指定chan_freq中生效的频点*/
    uint32_t        chan_freq[BB_CONFIG_MAX_CHAN_NUM];          /**<@note 频点表 单位：KHz*/
} bb_conf_br_hop_policy_t;

/**定义自适应跳频的触发条件项*/
typedef struct {
    uint8_t         retx_cnt;
    uint8_t         power_diff;
} bb_chan_hop_item_t;

/**定义自适应跳频触发参数*/
typedef struct {
    uint16_t        blind_hop_period;                           /**<@note 盲跳限制时间长度*/
    uint8_t         blind_hop_count;                            /**<@note 触发盲跳额错误次数*/
    uint8_t         hop_exclude_radius;                         /**<@note 跳频的信道间隔半径*/
    uint8_t         hop_stat_win_err : 1;                       /**<@note 是否把FCH错误当成LDPC错误处理*/
    uint8_t         item_num         : 7;
    bb_chan_hop_item_t items[BB_CONFIG_MAX_CHAN_HOP_ITEM_NUM];
} bb_chan_hop_para_t;

typedef struct {
    uint8_t         retx_cnt;
    uint8_t         power_diff;
    uint8_t         chan_inr;
} bb_chan_hop_multi_para_t;

typedef struct {
    uint16_t        snr_diff;
    uint8_t         rssi_diff;
    uint8_t         keep_count;
} bb_band_hop_item_t;

typedef struct {
    uint8_t         same_band_enable;
    uint8_t         same_band_master;
    uint16_t        snr_thred;
    uint8_t         scan_count;
    uint8_t         scan_inr;
    uint8_t         round_inr;
    uint8_t         gain_thred;
    uint8_t         bg_noise_diff;
    bb_band_hop_item_t items[BB_BAND_HOP_ITEM_MAX];
} bb_auto_band_para_t;

typedef struct {
    uint8_t         main_bw;                                    /**<@note 主频点带宽*/
    uint8_t         sub_bw;                                     /**<@note 子频点带宽*/
    uint8_t         chan_num;                                   /**<@note 子频点数*/
    int32_t         offset;                                     /**<@note 子频点计算的起始偏移*/
} bb_subchan_t;

/**定义配置命令BB_CFG_RC_POLICY的输入参数结构*/
typedef struct {
    uint8_t         enable;                                     /**<@note 使能选择性跳频策略*/
    uint8_t         max_freq_num;                               /**<@note 最大的循环跳频数量 最多4个*/
    uint8_t         stat_period;                                /**<@note 信号质量统计周期*/
    uint8_t         remove_percent;                             /**<@note 删除信道的错误百分比*/
} bb_conf_rc_hop_policy_t;

/**定义配置命令BB_CFG_CHANNEL的输入参数结构*/
typedef struct {
    uint8_t         band_mode;                                  /**<@note 频带模式 类型：bb_band_mode_e*/
    uint8_t         init_band;                                  /**<@note 初始band，如果为BB_BAND_MAX，则尝试自动选频段*/
    int8_t          init_chan;                                  /**<@note 初始信道，如果<0，则启动ACS起机自动选频功能*/
    int8_t          bonus_low_band;                             /**<@note 初始化选择频段时低频段底噪bonus db*/
    uint8_t         flags;                                      /**<@note 信道策略标志位*/
    uint8_t         fs_bw;                                      /**<@note 扫频频宽     类型：bb_bandwidth_e*/
    uint8_t         chan_num;                                   /**<@note 指定chan_freq中的频点数量*/
    uint32_t        chan_freq[BB_CONFIG_MAX_CHAN_NUM];          /**<@note 频点表 单位：KHz*/
    uint32_t        chan_freq_slave[BB_CONFIG_MAX_CHAN_NUM];    /**<@note 从机频点表，仅对主从AP模式有效*/
    bb_subchan_t    subchan;                                    /**<@note 子信道的配置*/
    bb_chan_hop_para_t hop_para;                                /**<@note 跳频条件参数*/
    bb_chan_hop_multi_para_t hop_para_multi;                    /**<@note 多套模式跳频参数*/
    bb_auto_band_para_t auto_band_para;                         /**<@note 频段自适应参数*/
} bb_conf_chan_t;

/**定义频率范围的通用结构体*/
typedef struct {
    uint32_t        freq_low;                                   /**<@note 最低频率值*/
    uint32_t        freq_high;                                  /**<@note 最高频率值*/
} bb_freq_range_t;

/**定义配置命令BB_CFG_CANDIDATES的输入参数结构*/
typedef struct {
    uint8_t         slot;                                       /**<@note 接入候选人配置，当slot>=BB_SLOT_MAX表示不固定slot位置，类型：bb_slot_e*/
    uint8_t         mac_num;                                    /**<@note 指定mac_tab中MAC的数量*/
    bb_mac_t        mac_tab[BB_CONFIG_MAX_SLOT_CANDIDATE];      /**<@note MAC表*/
} bb_conf_candidates_t;

/**定义物理用户单向基带参数*/
typedef struct {
    uint8_t         payload;                                    /**@<note 是否有payload，如果没有payload，下面的交织配置被忽略 类型：bb_payload_e*/
    uint8_t         fch_info_len;                               /**@<note FCH的bit长度 类型：bb_fch_info_len_e*/
    uint8_t         tintlv_enable;                              /**<@note 是否使能时域交织 类型：bb_timeintlv_enable_e*/
    uint8_t         tintlv_num;                                 /**<@note 交织数量 类型：bb_timeintlv_num_e*/
    uint8_t         tintlv_len;                                 /**<@note 交织块OFDM数量 类型：bb_timeintlv_len_e*/
    uint8_t         tx_mode;                                    /**<@note 用户的TX模式 类型：bb_tx_mode_e*/
    uint8_t         rx_mode;                                    /**<@note 用户的RX模式 类型：bb_rx_mode_e*/
    uint8_t         bandwidth;                                  /**<@note 频宽 类型：bb_bandwidth_e*/
    int8_t          retx_count;                                 /**<@note 重传到对次数 < 0: 重传到对           0 ~ 15：重传次数*/
    int8_t          pre_enc_ref;                                /**<@note 预编码参考位置 0为本单元，-1为前一个单元，1为后一个单元*/
    uint8_t         pre_enc_sp;                                 /**<@note 预编码start pointer 0 ~ 7 / 8*/
} bb_user_para_t;

/**定义配置命令BB_CFG_USER_PARA的输入参数结构*/
typedef struct {
    uint8_t         user;                                       /**<@note 基带物理用户 类型：bb_user_e*/
    bb_user_para_t  tx_para;                                    /**<@note 基带物理用户TX参数 忽略内部rx_mode字段*/
    bb_user_para_t  rx_para;                                    /**<@note 基带物理用户RX参数 忽略内部tx_mode字段 pre_enc_ref和prec_enc_sp字段*/
} bb_conf_user_para_t;

/**定义MCS策略条目结构*/
typedef struct {
    uint8_t         mcs;                                        /**<@note MCS等级 类型：bb_phy_mcs_e*/
    uint16_t        snr_up;                                     /**<@note 进入本级MCS所应大于等于的SNR线性值*/
    uint16_t        snr_dw;                                     /**<@note 退出本级MCS所应低于的SNR线性值*/
    uint8_t         ldpc_up_num;                                /**<@note 进入本级MCS所应小于等于的LDPC错误点数*/
    uint8_t         ldpc_dw_num;                                /**<@note 退出本级MCS所应大于的LDPC错误点数*/
    uint16_t        up_keep_time;                               /**<@note 进入本级MCS，SNR和LDPC条件需保持的时间长度 单位：毫秒*/
    uint16_t        dw_keep_time;                               /**<@note 退出本级MCS，SNR和LDPC条件需保持的时间长度 单位：毫秒*/
} bb_mcs_para_t;

/**定义auto bw的参数结构*/
typedef struct {
    uint8_t         enable;                                     /**<@note 使能频宽自适应 1：使能 0：禁用*/
    uint8_t         bw_low;                                     /**<@note 频宽自适应最低挡 类型：bb_bandwidth_e*/
    uint8_t         bw_high;                                    /**<@note 频宽自适应最高挡 类型：bb_bandwidth_e*/
    uint8_t         thred_dw;                                   /**<@note 触发降频宽的mcs挡位 类型：bb_phy_mcs_e*/
    uint8_t         thred_up;                                   /**<@note 触发升频宽的mcs挡位 类型：bb_phy_mcs_e*/
} bb_bw_auto_para_t;

/**定义配置命令BB_CFG_SLOT_RX_MCS的输入参数结构*/
typedef struct {
    uint8_t         slot;                                       /**<@note MCS策略使用位置 类型：bb_slot_e*/
    uint8_t         flags;                                      /**<@note MCS策略标志位*/
    uint16_t        delay_start;                                /**<@note MCS延迟启动计数*/
    uint16_t        hold_time;                                  /**<@note MCS切换后需要保持的时间*/
    uint8_t         init_mcs;                                   /**<@attention 初始MCS等级，如果>=BB_PHY_MCS_MAX那么，SDK根据接入时的SNR检测，选择一个合适的MCS级别 类型：bb_phy_mcs_e*/
    uint8_t         mcs_num;                                    /**<@note MCS级别数量*/
    uint16_t        max_wait_time;                              /**<@note 最多堵塞多长时间后放行*/
    uint8_t         mcs_auto_mode_policy;                       /**<@note MCS自动切换策略bb_mcs_auto_mode_policy_e*/
    uint8_t         rsv;
    bb_mcs_para_t   mcs_tab[BB_CONFIG_MAX_USER_MCS_NUM];        /**<@note MCS级别表*/
    bb_bw_auto_para_t bw_auto;
} bb_conf_mcs_t;

/**定义配置命令BB_CFG_DISTC的输入参数结构*/
typedef struct {
    uint8_t         enable;                                     /**<@note 是否使能测距功能*/
    uint8_t         window;                                     /**<@note 平均窗口大小，本参数2^n参数，n=0~3*/
    uint8_t         timeout;                                    /**<@note 超时阈值 2.6*(n+1)ms n=0~63*/
    uint32_t        offset;                                     /**<@note 测距校准值，即在近距离时基带td_out的输出值，此值用于在测距输出时进行基值扣除*/
} bb_conf_distc_t;

/**定义配置命令BB_CFG_PWR_BASIC的输入参数结构*/
typedef struct {
    bb_phy_pwr_mode_e pwr_mode;                                 /**<@note 功率开闭环模式*/
    uint8_t pwr_init;                                           /**<@note 功率初始值*/
    uint8_t pwr_auto;                                           /**<@note 功率自适应开关*/
    uint8_t pwr_min;                                            /**<@note 功率自适应打开时，功率区间最小值*/
    uint8_t pwr_max;                                            /**<@note 功率自适应打开时，功率区间最大值，当功率自适应关闭时，为系统功率初始值*/
}bb_phy_pwr_basic_t;

/**定义配置命令BB_CFG_PWR_BASIC_EX的输入参数结构*/
typedef struct {
    uint8_t link_auto           : 1;                            /**<@note 自动连接模式*/
    uint8_t auto_reset          : 5;                            /**<@note 功率自适应打开时，基带reset时功率*/
    uint8_t manu_init           : 5;                            /**<@note 功率自适应关闭时，基带初始功率*/
    uint8_t manu_reset          : 5;                            /**<@note 功率自适应关闭时，基带重置功率*/
    uint8_t csma_offset         : 4;                            /**<@note csma功率偏移*/
} bb_phy_pwr_basic_ex_t;

typedef struct {
    uint8_t         up_thred;                                   /**<@note 吞吐率提升门限 百分比*/
    uint8_t         dw_thred;                                   /**<@note 吞吐率降低门限 百分比*/
    uint8_t         keep_count;                                 /**<@note 保持的帧周期计数*/
    uint8_t         period_max;                                 /**<@note 最大发送周期*/
} bb_auto_power_save_t;

/**定义配置命令BB_CFG_POWER_SAVE的输入参数结构*/
typedef struct {
    uint8_t         master;                                     /**<@note 节能主控端 类型：bb_role_e*/
    uint8_t         flags;                                      /**<@note 1=自动模式 0=手动模式*/
    uint8_t         period_fix;                                 /**<@note 手动模式固定周期*/
    bb_auto_power_save_t auto_policy;                           /**<@note 自动模式策略*/
} bb_conf_power_save_t;

typedef struct {
    uint16_t        on_snr;                                     /**<@note <=该MCS时打开B路*/
    uint16_t        off_snr;                                    /**<@note >=该MCS时关闭B路*/
    uint16_t        on_keep;                                    /**<@note 打开B路时需保持的豪秒数*/
    uint16_t        off_keep;                                   /**<@note 关闭B路时需保持的豪秒数*/
} bb_auto_rf_policy_t;

/**定义配置命令BB_CFG_RF_POLICY的输入参数结构*/
typedef struct {
    uint8_t         flags;                                      /**<@note 1=自动模式 0=手动模式*/
    uint8_t         state_fix;                                  /**<@note 1=固定开启B路 0=固定关闭B路*/
    bb_auto_rf_policy_t auto_policy;                            /**<@note 自动模式策略*/
} bb_conf_rf_policy_t;

#if (BB_CONFIG_ENABLE_LNA_POLICY == 1)
typedef struct {
    uint8_t         auto_mode   : 1;                            /**<@note 1=自动模式 0=手动模式*/
    uint8_t         manu_on     : 1;                            /**<@note 1=手动时使能lna 0=手动时关闭lna*/
    uint8_t         offset      : 6;                            /**<@note lna开启与关闭的能量差*/
    uint8_t         on_must_thred;                              /**<@note 必须开启lna的极限gain值*/
    uint8_t         on_thred;                                   /**<@note 开启lna的gain阈值(>)*/
    uint8_t         off_thred;                                  /**<@note 关闭lna的gain阈值(<)*/
    int8_t          on_fs_thred;                                /**<@note 开启lna的频段噪声(<)*/
    int8_t          off_fs_thred;                               /**<@note 关闭lna的频段噪声(>)*/
    uint8_t         keep_count;                                 /**<@note gain连续保持计数*/
} bb_lna_t;

/**定义配置命令BB_CFG_LNA的输入参数结构*/
typedef struct {
    bb_lna_t        lna_inner;                                  /**<@note 内部lna策略*/
    bb_lna_t        lna_fem;                                    /**<@note 外部lna策略*/
} bb_conf_lna_t;
#endif

#if (BB_CONFIG_ENABLE_SHARE_SLOT == 1)
typedef void (*SHARE_SLOT_CALLBACK)(int slot_start);
/**定义配置命令BB_CFG_SHARE_SLOT的输入参数结构*/
typedef struct {
    uint16_t        slot_time_us;                               /**<@note 共享时隙时间长度 单位：us*/
    uint16_t        padding;
    SHARE_SLOT_CALLBACK callback;                               /**<@note 共享时隙回调函数*/
} bb_conf_share_slot_t;
#endif

#if (BB_CONFIG_AUTH_MODE_CE == 1)
/**定义配置命令BB_CFG_AUTH_MODE的输入参数结构*/
typedef struct {
    uint8_t         en : 1;                                     /**<@note 使能ce认证*/
    uint8_t         anti_interference : 3;                      /**<@note 抗干扰数*/
    uint8_t         mode : 4;                                   /**<@note ce认证模式, bb_ce_auth_mode_e*/
    int8_t          thres;                                      /**<@note 跳频阈值，单位：db*/
    int8_t          thres_5g;                                   /**<@note 5G跳频阈值，单位：db*/
    uint8_t         timeout;                                    /**<@note 频点禁用时长，单位秒*/
} bb_conf_ce_t;
#endif

/**定义配置命令BB_CFG_AGC的输入参数结构*/
typedef struct {
    uint8_t agc_gain;
    int8_t agc_gain_offset;
    int8_t gain_offset;
} bb_conf_agc_t;

/**定义配置命令BB_CFG_GAIN的输入参数结构*/
typedef struct {
    int8_t gain_a_offset_low;
    int8_t gain_b_offset_low;
    int8_t gain_a_offset_high;
    int8_t gain_b_offset_high;
    int8_t gain_a_thres;
    int8_t gain_b_thres;
} bb_conf_gain_t;

/**定义配置命令BB_CFG_RF_CALI的输入参数结构*/
typedef uint8_t bb_conf_rf_cali_t[BB_BAND_MAX];

// ################# get struct definition #####################

/**定义物理状态*/
typedef struct {
    uint8_t         mcs;                                        /**<@attention MCS级别 注意：如果是RX端此字段无意义，应该从link status中获取 类型：bb_phy_mcs_e*/
    uint8_t         rf_mode;                                    /**<@attention TX和RX模式，用户根据所在是RX参数或TX参数来决定值的意义 类型：bb_tx_mode_e或bb_rx_mode_e*/
    uint8_t         tintlv_enable;                              /**<@note 是否进行时域交织 类型：bb_timeintlv_enable_e*/
    uint8_t         tintlv_num;                                 /**<@note 交织块数量 类型：bb_timeintlv_num_e*/
    uint8_t         tintlv_len;                                 /**<@note 交织块OFDM数量 类型：bb_timeintlv_len_e*/
    uint8_t         bandwidth;                                  /**<@note 频宽 类型：bb_bandwidth_e*/
    uint32_t        freq_khz;                                   /**<@note 频点 单位：KHz*/
} bb_phy_status_t;

/**定义链路状态*/
typedef struct {
    uint8_t         state;                                      /**<@note 链路层状态 类型：bb_link_state_e*/
    uint8_t         rx_mcs     : 5;                             /**<@note 链路层接收MCS      类型：bb_phy_mcs_e*/
    uint8_t         pair_state : 1;                             /**<@note 处于配对状态*/
    uint8_t         padding    : 2;
    bb_mac_t        peer_mac;                                   /**<@note 对端MAC*/
} bb_link_status_t;

/**定义用户的状态结构*/
typedef struct {
    bb_phy_status_t tx_status;                                  /**<@note 物理层发送侧状态*/
    bb_phy_status_t rx_status;                                  /**<@note 物理层接收侧状态*/
} bb_user_status_t;

/**定义读取命令BB_GET_STATUS的输出参数结构*/
typedef struct {
    uint8_t         role;                                       /**<@note 设备角色 类型：bb_role_e*/
    uint8_t         mode;                                       /**<@note 基带模式 类型：bb_mode_e*/
    uint8_t         sync_mode;                                  /**<@note 芯片同步模式 1: 使能 0：不使能*/
    uint8_t         sync_master;                                /**<@note 同步模式中身份 1：   master      0：slave*/
    uint8_t         cfg_sbmp;                                   /**<@note 配置的SLOT的bitmap*/
    uint8_t         rt_sbmp;                                    /**<@note 运行时SLOT的bitmap*/
    bb_mac_t        mac;                                        /**<@note 本机的MAC地址*/
    bb_user_status_t user_status[BB_DATA_USER_MAX];             /**<@note 物理用户状态*/
    bb_link_status_t link_status[BB_SLOT_MAX];                  /**<@note 链路状态*/
} bb_get_status_out_t;

/**定义读取命令BB_GET_STATUS的输入参数结构*/
typedef struct {
    uint16_t        user_bmp;                                   /**<@attention 需要获取的物理用户的bitmap，如果不关注物理层信息，可以填0忽略*/
} bb_get_status_in_t;

/**定义读取命令BB_GET_PAIR_RESULT的输出参数结构*/
typedef struct {
    uint8_t         slot_bmp;                                   /**<@note 配对成功的SLOT位置bitmap*/
    bb_mac_t        peer_mac[BB_SLOT_MAX];                      /**<@note 对应SLOT上配对成功的MAC地址*/
    bb_quality_t    quality[BB_SLOT_MAX];                       /**<@note 配对完成时的无线质量统计*/
} bb_get_pair_out_t;

/**定义读取命令BB_GET_AP_MAC的输出参数结构*/
typedef struct {
    bb_mac_t        mac;                                        /**<@note 接入点MAC*/
} bb_get_ap_mac_out_t;


/**定义读取命令BB_GET_CANDIDATES的输入参数结构*/
typedef struct {
    uint8_t         slot;                                       /**<@note 指定获取候选人SLOT位置 类型：bb_slot_e*/
} bb_get_candidates_in_t;

/**定义读取命令BB_GET_CANDIDATES的输出参数结构*/
typedef struct {
    uint8_t         mac_num;                                    /**<@note mac_tab中MAC地址的数量*/
    bb_mac_t        mac_tab[BB_CONFIG_MAX_SLOT_CANDIDATE];      /**<@note 候选人MAC地址表*/
} bb_get_candidates_out_t;

/**定义读取命令BB_GET_USER_QUALITY的输入参数结构*/
typedef struct {
    uint16_t        user_bmp;                                   /**<@note 需要获取信号量的物理用户bitmap*/
    uint16_t        average;                                    /**<@note 当arerage为0，返回SDK内部实时信号质量，当为1时，返回SDK内部一个采样周期的平均值*/
} bb_get_user_quality_in_t;

/**定义读取命令BB_GET_USER_QUALITY的输出参数结构*/
typedef struct {
    bb_quality_t    qualities[BB_DATA_USER_MAX];                /**<@note 分用户的信号量，当值为全0，表示该用户信号质量无效*/
} bb_get_user_quality_out_t;

/**定义读取命令BB_GET_DISTC_RESULT的输入参数结构*/
typedef struct {
    uint8_t         slot_bmp;                                   /**<@note 需要读取  slot位置bitmap*/
} bb_get_distc_result_in_t;

typedef struct {
    uint8_t         slot_bmp;                                   /**<@note 需要读取peer slot位置bitmap，dev侧按BB_SLOT_AP来设置*/
    uint8_t         arverage;                                   /**<@note 为扩展预留*/
} bb_get_peer_quality_in_t;

typedef struct {
    bb_quality_t    qualities[BB_SLOT_MAX];                     /**<@note peer slot信号质量*/
} bb_get_peer_quality_out_t;

/**定义读取命令BB_GET_AP_TIME的输出参数结构*/
typedef struct {
    uint32_t        timestamp;                                  /**<@note AP的时间戳，系统起来后运行的毫秒数*/
} bb_get_ap_time_out_t;

/**定义读取命令BB_GET_DISTC_RESULT的输出参数结构*/
typedef struct {
    int32_t         distance[BB_SLOT_MAX];                      /**<@note 测距结果，无测距结果时为-1，>=为测距结果，用户未指定slot位置的值未知*/
} bb_get_distc_result_out_t;

/**定义读取命令BB_GET_MCS的输入参数结构*/
typedef struct {
    uint8_t         dir;                                        /**<@note 获取MCS的方向 类型：bb_dir_e*/
    uint8_t         slot;                                       /**<@note 获取MCS的slot 类型：bb_slot_e*/
} bb_get_mcs_in_t;

/**定义读取命令BB_GET_MCS的输出参数结构*/
typedef struct {
    uint8_t         mcs;                                        /**<@note 当前slot的MCS，类型：bb_phy_mcs_e*/
    uint32_t        throughput;                                 /**<@note 当前slot的理论吞吐率（数据发送能力）单位：kbps*/
} bb_get_mcs_out_t;

/**定义读取命令BB_GET_MCS_MODE的输入参数结构*/
typedef struct {
    uint8_t         slot;                                       /**<@attention MCS模式控制的SLOT位置 类型：bb_slot_e*/
} bb_get_mcs_mode_in_t;

/**定义读取命令BB_GET_MCS_MODE的输出参数结构*/
typedef struct {
    uint8_t         auto_mode;                                  /**<@note MCS控制模式 1：MCS自适应，0：MCS手工模式，只有在手工模式才支持设置MCS挡位*/
} bb_get_mcs_mode_out_t;

/**定义读取命令BB_GET_BANDWIDTH_MODE的输入参数结构*/
typedef struct {
    uint8_t         slot;                                       /**<@attention 频宽模式控制的SLOT位置 类型：bb_slot_e*/
} bb_get_bandwidth_mode_in_t;

/**定义读取命令BB_GET_BANDWIDTH_MODE的输出参数结构*/
typedef struct {
    uint8_t         auto_mode;                                  /**<@note 频宽控制模式 1=频宽自适应 0=频宽手动控制*/
} bb_get_bandwidth_mode_out_t;

/**定义读取命令BB_GET_CHAN_INFO的输出参数结构*/
typedef struct {
    uint8_t         chan_num;                                   /**<@note 信道数量*/
    uint8_t         auto_mode;                                  /**<@note 信道自适应 1：自适应 0：手动*/
    uint8_t         acs_chan;                                   /**<@note ACS信道，启动选频时的信道*/
    uint8_t         work_chan;                                  /**<@attention 工作信道，不同基带模式及不同角色对工作信道定义有区别，一般来说，work_chan指示当前设备收信号的工作信道*/
    uint32_t        freq[BB_CONFIG_MAX_CHAN_NUM];               /**<@note 信道频点 单位：KHz*/
    int32_t         power[BB_CONFIG_MAX_CHAN_NUM];              /**<@note 扫频均化能量 单位：dbm*/
} bb_get_chan_info_out_t;

/**定义读取命令BB_GET_REG的输入参数结构*/
typedef struct {
    uint16_t        offset;                                     /**<@note 读取基带寄存器偏移量，应<BB_REG_PAGE_NUM*BB_REG_PAGE_SIZE*/
    uint16_t        length;                                     /**<@note 读取基带寄存器的字节长度，应<=BB_REG_PAGE_SIZE并且不超过寄存器空间有效范围*/
} bb_get_reg_in_t;

/**定义读取命令BB_GET_REG的输出参数结构*/
typedef struct {
    uint8_t         data[BB_REG_PAGE_SIZE];                     /**<@note 由输入参数确定读取的寄存器内容*/
} bb_get_reg_out_t;

/**定义读取命令BB_GET_CFG的输入参数结构*/
typedef struct {
    uint16_t        seq;                                        /**<@note 命令序列号，单调递增*/
    uint8_t         mode;                                       /**<@note 加载模式, 0:auto, 1:memory, 2:flash*/
    uint8_t         rsv;                                        /**<@note 保留字段*/
    uint16_t        offset;                                     /**<@note 读取基带配置文件偏移量*/
    uint16_t        length;                                     /**<@note 读取基带配置文件的字节长度，应<=ioctrl缓冲区最大长度*/
} bb_get_cfg_in_t;

/**定义读取命令BB_GET_CFG的输出参数结构*/
typedef struct {
    uint16_t        seq;                                        /**<@note 命令序列号，等于请求的序列号*/
    uint16_t        rsv;                                        /**<@note 保留字段*/
    uint16_t        total_length;                               /**<@note 基带配置文件总长度*/
    uint16_t        total_crc16;                                /**<@note 基带配置文件crc16校验码*/
    uint16_t        offset;                                     /**<@note 设置基带配置文件偏移量*/
    uint16_t        length;                                     /**<@note 设置基带配置文件的字节长度*/
    uint8_t         data[BB_CFG_PAGE_SIZE-12];                  /**<@note 由输入参数确定读取的配置文件内容*/
} bb_get_cfg_out_t;

/**定义读取命令BB_GET_DBG_MODE的输出参数结构*/
typedef struct {
    uint8_t         enable;                                     /**<@note debug模式使能与否 1：使能 0：关闭*/
} bb_get_dbg_mode_out_t;

/**定义读取命令BB_GET_POWER_MODE的输出参数结构*/
typedef struct {
    uint8_t pwr_mode;                                           /**<@note 获取发射功率开闭环模式，取值参见:bb_phy_pwr_mode_e*/
} bb_get_pwr_mode_out_t;

/**定义读取命令BB_GET_CUR_POWER的输入参数结构*/
typedef struct {
    uint8_t usr;                                                /**<@note 获取指定用户的当前功率，取值参见：bb_user_e*/
} bb_get_cur_pwr_in_t;

/**定义读取命令BB_GET_CUR_POWER的输入参数结构*/
typedef struct {
    uint8_t usr;                                                /**<@note 当前发射功率对应的用户*/
    uint8_t pwr;                                                /**<@note 当前发射功率*/
} bb_get_cur_pwr_out_t;

/**定义读取命令BB_GET_PWR_AUTO的输入参数结构*/
typedef struct {
    uint8_t pwr_auto;                                           /**<@note 功率自适应是否使能*/
} bb_get_pwr_auto_out_t;

/**定义读取命令BB_GET_SYS_INFO的输出参数结构*/
typedef struct {
    uint64_t uptime;                                           /**<@note 获取系统运行时间*/
    char compile_time[32];                                     /**<@note 编译时间*/
    char soft_ver[32];                                         /**<@note 软件版本*/
    char hardware_ver[32];                                     /**<@note 硬件版本*/
    char firmware_ver[32];                                     /**<@note 固件版本*/
} bb_get_sys_info_out_t;

/**定义读取命令BB_GET_PRJ_DISPATCH的输入参数结构*/
typedef struct {
    uint8_t data[256];                                         /**二级分发内容*/
} bb_get_prj_dispatch_in_t;

/**定义读取命令BB_GET_PRJ_DISPATCH的输出参数结构*/
typedef bb_get_prj_dispatch_in_t bb_get_prj_dispatch_out_t;

/**定义异步调用回调函数指针*/
typedef int (*ioctl_asyn_cb)(void* priv, uint32_t ret);
/**定义命令BB_REMOTE_IOCTL_REQ的输入参数结构*/
PACK(typedef struct {
    uint8_t     slot;               /* Slot for remote cmd */
    uint16_t    len;                /* Length of the data */
    uint32_t    msg_id;             /* Id of the msg(bb_ioctl) */
    uint8_t     data[1024];
}) bb_remote_ioctl_in_t;

/**定义读取命令BB_REMOTE_IOCTL_REQ的输出参数结构*/
typedef bb_remote_ioctl_in_t bb_remote_ioctl_out_t;

/**定义读取命令BB_GET_BAND_INFO的输入输出参数*/
typedef struct {
    char rsv[32];
} bb_get_band_info_in_t;

typedef struct {
    uint8_t         band_mode;                                /**<@note 1=频段自适应，0=频段手动控制*/
    uint8_t         work_band;                                /**<@note 当前工作频段，类型：bb_band_e*/
    char            rsv[30];                                  /**保留*/
} bb_get_band_info_out_t;

/**定义读取命令BB_GET_USER_INFO的输入参数*/
typedef struct {
    uint8_t         user;                                       /**<@note 用户ID 类型：bb_user_e*/
} bb_get_user_info_in_t;

/**定义读取命令BB_GET_BAND_INFO的输出参数*/
typedef struct {
    int32_t         freq_offset;                                /**<@note 用户频偏值*/
    uint8_t         rsv[124];
} bb_get_user_info_out_t;

/**定义读取命令BB_GET_1V1_INFO的输出参数*/
typedef struct {
    uint8_t         frame_num;                                  /**<@note 统计多少帧，0代表默认(64帧)，最大64帧*/
    uint8_t         rsv[3];
} bb_get_1v1_info_in_t;

typedef struct {
    uint16_t    chan_snr;
    uint8_t     gain_a;
    uint8_t     gain_b;
} bb_lfs_quality_t;

typedef struct {
    uint16_t    snr;                     /**<@note 物理用户接收信噪比，转化db公式：10log(snr/36)*/
    uint16_t    ldpc_tlv_err_ratio;      /**<@note 交织块中解码错误的LDPC块个数所占的比例，不想出现小数，扩大了10000倍*/
    uint16_t    ldpc_num_err_ratio;      /**<@note 解码错误的帧个数比例，不想出现小数，扩大了10000倍*/
    uint8_t     gain_a;                  /**<@note A路天线接收信号强度*/
    uint8_t     gain_b;                  /**<@note B路天线接收信号强度*/
    uint8_t     tx_mcs;                  /**<@note TX MCS*/
    uint8_t     tx_chan;                 /**<@note TX channel*/
    uint8_t     tx_power;                /**<@note TX power*/
    uint8_t     lna_inner_bypass : 1;    /**<@note 内部LNA bypass*/
    uint8_t     lna_fem_bypass   : 1;    /**<@note FEM LNA bypass*/
    uint8_t     rf_1tx           : 1;    /**<@note 1=单发模式 0=双发模式*/
    uint32_t    tx_freq_khz;             /**<@note TX freq khz*/
    bb_lfs_quality_t lfs_low_band;       /**<@note 2G长扫频质量*/
    bb_lfs_quality_t lfs_high_band;      /**<@note 5G长扫频质量*/
    uint8_t     rev[48];
} bb_info_t;

/**定义读取命令BB_GET_1V1_INFO的输出参数*/
typedef struct {
    bb_info_t self;                      /**<@note 本设备属性*/
    bb_info_t peer;                      /**<@note 对端设备属性*/
    uint8_t   rev[64];
} bb_get_1v1_info_out_t;

/**定义读取命令BB_GET_SOCK_INFO的输入参数*/
typedef struct {
    uint8_t         slot;                   /**<@note 指定要获取的slot 类型：bb_slot_e*/
    int8_t          port;                   /**<@note 指定要获的port id，当port < 0则获取系统所有socket的信息*/
    uint8_t         padding[2];
} bb_get_sock_info_in_t;

typedef struct {
    uint8_t         available;              /**<@note 数据有效*/
    uint8_t         padding;
    uint16_t        overflow_cnt;           /**<@note buf溢出的计数*/
    uint32_t        buf_size;               /**<@note buf的size*/
    uint32_t        data_size;              /**<@note 当前buf内数据量*/
    uint64_t        total_size;             /**<@note 从buf通过的总数据量*/
    uint8_t         reserved[12];
} bb_sock_uni_t;

typedef struct {
   bb_sock_uni_t    uni_info[BB_DIR_MAX];   /**<@note 双向数据信息*/
} bb_sock_info_t;

/**定义读取命令BB_GET_SOCK_INFO的输出参数*/
typedef struct {
    uint8_t         port_bmp;
    uint8_t         padding[3];
    bb_sock_info_t  sock_info[BB_SOCK_INFO_NUM];
} bb_get_sock_info_out_t;

typedef struct {
    uint8_t         slot;                   /**<@note 指定要获取的slot 类型：bb_slot_e*/
    uint8_t         dir_bmp;                /**<@note bb_dir_e的bitmap，用于指定获取吞吐率的方向组合*/
    uint8_t         padding[2];
} bb_get_throughput_in_t;

typedef struct {
    uint32_t        phy_throughput;         /**<@note 基带提供的实时物理吞吐率*/
    uint32_t        real_throughput;        /**<@note 实际承载的数据吞吐率（含SDK协议头）*/
} bb_throughtput_t;

typedef struct {
    bb_throughtput_t throughput[BB_DIR_MAX]; /**<@note 实时吞吐率*/
} bb_get_throughput_out_t;

typedef enum {
    BB_REMOTE_TYPE_BAND_MODE = 0,
    BB_REMOTE_TYPE_TARGET_BAND,
    BB_REMOTE_TYPE_CHAN_MODE,
    BB_REMOTE_TYPE_TARGET_CHAN,
    BB_REMOTE_TYPE_COMPLIANCE_MODE,
    BB_REMOTE_TYPE_PWR_AUTO2,
    BB_REMOTE_TYPE_TARGET_PWR,
    BB_REMOTE_TYPE_MAX
} bb_remote_type_e;

typedef struct {
    uint8_t auto_band;                                          /**<@note 1=频段自适应，0=频段手动控制*/
    uint8_t target_band;                                        /**<@note 设置的目标band*/
    uint8_t auto_chan;                                          /**<@note 1=信道自适应，0=信道手动控制*/
    uint8_t target_chan;                                        /**<@note 设置的目标信道*/
    uint8_t compliance_mode;                                    /**<@note 1=合规模式，0=自由模式*/
    uint8_t pwr_auto2;                                           /**<@note 1=功率自适应，0=功率手动模式*/
    uint8_t target_pwr;                                         /**<@note 设置目标功率*/
    uint8_t padding[121];                                       /***/
} bb_remote_setting_t;

typedef struct {
    uint8_t         slot;                                       /**<@note 获取指定slot位置的remote设置*/
    uint8_t         padding[3];
    uint32_t        type_bmp;                                   /**<@note 指定获取的类型bitmap*/
} bb_get_remote_in_t;

typedef struct {
    uint32_t            valid_bmp;                              /**<@note 对应bit置位，说明获取的类型有设置值，否则远端以它的本地配置运行*/
    bb_remote_setting_t setting;                                /**<@note 设置结构体*/
} bb_get_remote_out_t;

typedef struct {
    uint8_t             path_a_tx_state;                        /**<@note A路RF TX状态 0=关闭 1=开启*/
    uint8_t             path_a_rx_state;                        /**<@note A路RF RX状态 0=关闭 1=开启*/
    uint8_t             path_b_tx_state;                        /**<@note B路RF TX状态 0=关闭 1=开启*/
    uint8_t             path_b_rx_state;                        /**<@note B路RF RX状态 0=关闭 1=开启*/
} bb_get_rf_out_t;

typedef enum {
    BB_RUNSYS_UNKNOWNN = -1,
    BB_RUNSYS_MASTER,
    BB_RUNSYS_BACKUP
} bb_runsys_t;

typedef struct {
    bb_runsys_t runsys_id;
} bb_get_runsys_out_t;

// ################# set struct definition #####################

/**定义设置命令BB_SET_EVENT_SUBSCRIBE和BB_SET_EVENT_UNSUBSCRIBE的输入参数结构*/
typedef struct {
    bb_event_e          event;                                  /**<@note 事件类型*/
    bb_event_callback   callback;                               /**<@note 事件对应的回调函数*/
    void*               user;                                   /**<@note 事件回调用户自定义参数，对BB_SET_EVENT_UNSUBSCRIBE命令字无效*/
} bb_set_event_callback_t;

/**定义设置命令BB_SET_PAIR_MODE的输入参数结构*/
typedef struct {
    uint8_t         start;                                      /**<@note 配对动作，1: 进入配对模式, 0: 退出配对模式*/
    uint8_t         slot_bmp;                                   /**<@note AP侧允许进行配对的SLOT位置bitmap，DEV忽略本字段*/
    bb_mac_t        black_list[BB_BLACK_LIST_SIZE];             /**<@note 进入配对时，设置禁止配对  的黑名单MAC, 如不设黑名单请清0*/
} bb_set_pair_mode_t;

/**定义设置命令BB_SET_AP_MAC的输入参数结构*/
typedef struct {
    bb_mac_t        mac;                                        /**<@note 接入点MAC地址*/
} bb_set_ap_mac_t;

/**定义设置命令BB_SET_LOCAL_MAC的输入参数结构*/
typedef struct {
    bb_mac_t        mac;
} bb_set_local_mac_t;

/**定义设置命令BB_SET_CHAN_PWR_PLUS的输入参数结构*/
typedef struct {
    uint8_t num;                                                /**<@note 频点数量*/
    int8_t chan_pwr_plus[512];                                  /**<@note 频点最大功率偏移值*/
} bb_set_chan_pwr_plus_t;

/**定义设置命令BB_SET_CHAN_MODE的输入参数结构*/
typedef struct {
    uint8_t         auto_mode;                                  /**<@note 信道工作模式 1：信道自适应，0：手工模式，只有在手工模式才支持设置信道*/
} bb_set_chan_mode_t;

/**定义设置命令BB_SET_CHAN的输入参数结构*/
typedef struct {
    uint8_t         chan_dir;                                   /**<@note 跳频的信道方向，目前只有导演模式支持TX方向（广播信道）的频道控制 类型：bb_dir_e*/
    uint8_t         chan_index;                                 /**<@note 信道索引值，必须是合法额预制信道索引*/
} bb_set_chan_t;

/**定义设置命令BB_SET_MCS_MODE的输入参数结构*/
typedef struct {
    uint8_t         slot;                                       /**<@attention MCS模式控制的SLOT位置 类型：bb_slot_e*/
    uint8_t         auto_mode;                                  /**<@note MCS控制模式 1：MCS自适应，0：MCS手工模式，只有在手工模式才支持设置MCS挡位*/
} bb_set_mcs_mode_t;

/**定义设置命令BB_SET_MCS的输入参数结构*/
typedef struct {
    uint8_t         slot;                                       /**<@attention MCS设置的SLOT位置 类型：bb_slot_e*/
    uint8_t         mcs;                                        /**<@attention 目标MCS挡位,目前挡位必须是MCS表中有定义的值 类型：bb_phy_mcs_e*/
} bb_set_mcs_t;

typedef struct {
    uint8_t         slot;                                       /**<@attention 设置主dev的slot位置，当slot>最大导演模式dev数量，则表示取消主dev模式 类型：bb_slot_e*/
} bb_set_master_dev_t;

/**定义设置命令BB_SET_CANDIDATES的输入参数结构*/
typedef bb_conf_candidates_t bb_set_candidate_t;                /**<@note 复用配置候选人输入结构*/

/**定义设置命令BB_SET_CUSTOMER_KEY的输出参数结构*/
#define BB_CUSTOMER_KEY_LEN 8                                   /**<@note customer key可用空间8个字节*/
typedef struct {
    uint8_t         addr;                                       /**<@note customer key的写入地址,共8个字节,按照4个字节倍数写入:0-从地址0开始写入, 1-从地址4开始写入*/
    uint8_t         len;                                        /**<@note customer key的写入长度*/
    uint8_t         data[BB_CUSTOMER_KEY_LEN];                  /**<@note customer key*/
} bb_set_customer_key_in_t;

/**定义设置命令BB_SET_REG的输入参数结构*/
typedef struct {
    uint16_t        offset;                                     /**<@note 设置基带寄存器偏移量，应<BB_REG_PAGE_NUM*BB_REG_PAGE_SIZE*/
    uint16_t        length;                                     /**<@note 设置基带寄存器的字节长度，应<=BB_REG_PAGE_SIZE并且不超过寄存器空间有效范围*/
    uint8_t         data[BB_REG_PAGE_SIZE];                     /**<@note 设置基带寄存器的内容*/
} bb_set_reg_t;

/**定义设置命令BB_SET_CFG的输入参数结构*/
typedef struct {
    uint32_t        seq;                                        /**<@note 命令序列号，单调递增*/
    uint16_t        total_length;                               /**<@note 基带配置文件总长度*/
    uint16_t        total_crc16;                                /**<@note 基带配置文件crc16校验码*/
    uint16_t        offset;                                     /**<@note 设置基带配置文件偏移量*/
    uint16_t        length;                                     /**<@note data字节长度*/
    uint8_t         data[BB_CFG_PAGE_SIZE-12];                  /**<@note 设置基带配置文件的内容,注意：结构体的总大小不能大于1024*/
} bb_set_cfg_t;

/**定义设置命令BB_SET_PLOT的输入参数结构*/
typedef struct {
    uint8_t         user;                                       /**<@note 需要plot的物理用户 类型：bb_user_e*/
    uint8_t         enable;                                     /**<@note 使能plot与否 1：使能 0：关闭*/
    uint8_t         cache_num;                                  /**<@note 缓存多少个点后发送回调事件 不大于BB_PLOT_POINT_MAX*/
} bb_set_plot_t;

/**定义设置命令BB_SET_DBG_MODE的输入参数结构*/
typedef bb_get_dbg_mode_out_t bb_set_dbg_mode_t;                /**<@note 复用获取debug mode输出结构*/

/**定义设置命令BB_SET_POWER_MODE的输入参数结构*/
typedef bb_get_pwr_mode_out_t bb_set_pwr_mode_in_t;             /**<@note 复用获取power mode输出结构*/

/**定义设置命令BB_SET_POWER的输入参数结构*/
typedef bb_get_cur_pwr_out_t bb_set_pwr_in_t;                   /**<@note 复用获取当前功率输出结构, power取值范围[0-31dbm]*/

/**定义设置命令BB_SET_POWER_AUTO的输入参数结构*/
typedef bb_get_pwr_auto_out_t bb_set_pwr_auto_in_t;             /**<@note 复用获取功率自适应使能输出结构*/

/**定义设置命令BB_SET_FREQ的输入参数结构*/
typedef struct {
    uint8_t         user;                                       /**<@note 需要设置频率的物理用户 类型：bb_user_e*/
    uint8_t         dir_bmp;                                    /**<@note 发送或接收方向的bitmap （bb_dir_e的bit掩码）*/
    uint32_t        freq_khz;                                   /**<@note 频率值 单位：KHz*/
} bb_set_freq_t;

/**定义设置命令BB_SET_TX_MCS的输入参数结构*/
typedef struct {
    uint8_t         user;                                       /**<@note 需要设置MCS的物理用户，仅数据用户支持 类型：bb_user_e*/
    uint8_t         mcs;                                        /**<@note MCS等级 类型：bb_phy_mcs_e*/
} bb_set_tx_mcs_t;

/**定义设置命令BB_SET_REET的输入参数结构*/
typedef struct {
    uint8_t         user;                                       /**<@attension 需要复位的物理用户，仅数据用户支持（如果>= BB_DATA_USER_MAX则进行基带复位） 类型：bb_user_e*/
    uint8_t         dir_bmp;                                    /**<@note 发送或接收方向的bitmap （bb_dir_e的bit掩码）*/
} bb_set_reset_t;

/**定义设置命令 BB_SET_SYS_REBOOT 的输入参数结构*/
typedef struct {
    uint32_t        tim_ms;                                       /**<@note 复位参数 单位毫秒*/
} bb_set_reboot_t;

/**定义设置命令BB_SET_TX_PATH的输入参数结构*/
typedef struct  {
    uint8_t         path_bmp;                                   /**<@attention 射频通道的bitmap*/
} bb_set_tx_path_t;

/**定义设置命令BB_SET_RX_PATH的输入参数结构*/
typedef bb_set_tx_path_t bb_set_rx_path_t;                      /**<@attention 射频通道的bitmap*/

typedef struct {
    uint8_t         role;
} bb_set_orig_cfg_t;

/**定义设置命令BB_SET_RF的输入参数结构*/
typedef struct {
    uint8_t         rf_path;                                    /**<@attention 射频AB路 类型：bb_rf_path_e*/
    uint8_t         dir;                                        /**<@attention 射频收发方向 类型：bb_dir_e*/
    uint8_t         state;                                      /**<@attention RF目标状态 0：关 1：开*/
} bb_set_rf_t;

/**定义设置命令BB_SET_POWER_OFFSET的输入参数结构*/
typedef struct  {
    uint8_t         band;                                       /**<@attention 频段，取值参见bb_band_e*/
    int8_t          offset;                                     /**<@attention 功率补偿值*/
    uint8_t         path;                                       /**<@attention 参见bb_rf_path_e*/
    uint8_t         rsv;
} bb_set_power_offset_t;

/**定义设置命令BB_GET_POWER_OFFSET的输入参数结构*/
typedef struct  {
    uint8_t         band;                                       /**<@attention 频段，取值参见bb_band_e*/
    uint8_t         path;
    uint16_t        rsv;                                        /**<@note 保留字段*/
} bb_get_power_offset_in_t;

/**定义设置命令BB_GET_POWER_OFFSET的输出参数结构*/
typedef struct  {
    int8_t          offset;
} bb_get_power_offset_out_t;

/**定义设置命令BB_GET_POWER_OFFSET2的输入参数结构*/
typedef struct  {
    uint8_t         band;                                       /**<@attention 频段，取值参见bb_band_e*/
    uint8_t         rsv[3];                                     /**<@note 保留字段*/
} bb_get_power_offset2_in_t;

/**定义设置命令BB_GET_POWER_OFFSET2的输出参数结构*/
typedef struct  {
    int8_t          path_a;                                     /**<@attention a路功率校准值*/
    int8_t          path_b;                                     /**<@attention b路功率校准值*/
    uint8_t         rsv[2];                                     /**<@note 保留字段*/
} bb_get_power_offset2_out_t;

/**定义设置命令BB_GET_CUSTOMER_KEY的输出参数结构*/
typedef struct {
    uint8_t         data[BB_CUSTOMER_KEY_LEN];                  /**<@note customer key*/
} bb_get_customer_key_out_t;

/**定义设置命令BB_SET_HOT_UPGRADE_WRITE的输入参数结构*/
typedef struct {
    uint16_t seq;                                               /**<@note 命令字序列号, 可以任意产生*/
    uint16_t len;                                               /**<@note data中数据的长度*/
    uint32_t addr;                                              /**<@note 写入地址*/
    uint8_t data[BB_CFG_PAGE_SIZE-8];                           /**<@note 数据内容*/
} bb_set_hot_upgrade_write_in_t;

/**定义设置命令BB_SET_HOT_UPGRADE_WRITE的输出参数结构*/
typedef struct {
    uint16_t seq;                                               /**<@note 命令字序列号，可以任意产生*/
    int16_t ret;                                                /**<@note 命令执行结果，参见bb_errno*/
} bb_set_hot_upgrade_write_out_t;

/**定义设置命令BB_SET_HOT_UPGRADE_CRC32的输入参数结构*/
typedef struct {
    uint16_t seq;                                               /**<@note 命令字序列号，可以任意产生*/
    uint16_t reserve;                                           /**<@note reserve*/
    uint32_t len;                                               /**<@note crc32校验数据的长度*/
    uint32_t addr;                                              /**<@note 待校验的数据地址*/
    uint32_t crc32;                                             /**<@note crc32校验值*/
} bb_set_hot_upgrade_crc32_in_t;

/**定义读取命令BB_SET_PRJ_DISPATCH的输入参数结构*/
typedef bb_get_prj_dispatch_in_t bb_set_prj_dispatch_in_t;

/**定义读取命令BB_SET_PRJ_DISPATCH2的输入参数结构*/
typedef struct {
    uint8_t data[1024];
} bb_set_prj_dispatch2_in_t;

/**定义设置命令BB_SET_FRAME_CHANGE的输入参数结构*/
typedef struct {
    uint8_t mode;                                               /**<@note mode>0执行交换, mode=1(10M), mode=0恢复原始帧结构*/
} bb_set_frame_change_t;

/**定义设置命令BB_SET_COMPLIANCE_MODE的输入参数结构*/
typedef struct {
    uint8_t enable;                                             /**<@note 1=使能合规模式，0=自由模式*/
} bb_set_compliance_mode_t;

typedef struct {
    uint8_t auto_mode;                                          /**@note 1=频段自适应，0=频段手动控制*/
} bb_set_band_mode_t;

/**定义设置命令BB_SET_BAND的输入参数结构*/
typedef struct {
    uint8_t target_band;                                        /**<@note 设置的目标band*/
} bb_set_band_t;

typedef struct {
    uint8_t slot;                                        /**<@note 设置的目标slot*/
    uint8_t port;                                        /**<@note 设置的目标port*/
} bb_force_close_socket_t;

typedef struct  {
    uint8_t             slot;                                   /**<@note 目标slot 类型：bb_slot_e*/
    uint8_t             padding[3];
    uint32_t            type_bmp;                               /**<@note 类型bitmap，按类型bb_remote_type_e按顺序取位*/
    bb_remote_setting_t setting;                                /**<@note 设置结构体*/
} bb_set_remote_t;

/**定义设置命令BB_SET_BANDWIDTH的输入参数结构*/
typedef struct {
    uint8_t             slot;                                   /**<@note 预留参数*/
    uint8_t             dir;                                    /**<@note 切换bandwidth的方向 类型：bb_dir_e*/
    uint8_t             bandwidth;                              /**<@note 目标bandwidth 类型：bb_bandwidth_e*/
} bb_set_bandwidth_t;

typedef struct {
    uint8_t             slot;                                   /**<@note 目标slot 类型：bb_slot_e*/
    uint8_t             mode;                                   /**<@note 1=频宽自适应 0=频宽手动控制*/
} bb_set_bandwidth_mode_t;

/**定义设置命令BB_SET_POWER_SAVE_MODE的输入参数结构*/
typedef struct {
    int8_t              mode;                                   /**<@note 节能模式     0=手动 1=自动*/
} bb_set_power_save_mode_t;

/**定义设置命令BB_GET_POWER_SAVE_MODE的输出参数结构*/
typedef bb_set_power_save_mode_t bb_get_power_save_mode_t;

/**定义设置命令BB_SET_POWER_SAVE的输入参数结构*/
typedef struct {
    uint8_t             period;                                 /**<@note 节能发送周期*/
} bb_set_power_save_t;

/**定义设置命令BB_GET_POWER_SAVE的输出参数结构*/
typedef bb_set_power_save_t bb_get_power_save_t;

/**定义设置命令BB_SET_MCS_RANGE的输入参数结构*/
typedef struct {
    uint8_t slot;                                               /**<@note 目标slot 类型：bb_slot_e*/
    uint8_t mcs_min;                                            /**<@note 最小mcs, BB_PHY_MCS_MAX代表不限制，类型：bb_phy_mcs_e*/
    uint8_t mcs_max;                                            /**<@note 最大mcs, BB_PHY_MCS_MAX代表不限制，类型：bb_phy_mcs_e*/
    uint8_t rsv;
} bb_set_mcs_range_in_t;

/**定义设置命令BB_SET_LNA_MODE的输入参数结构*/
typedef struct {
    uint8_t             mode;                                   /**<@note 1=自动 0=手动*/
} bb_set_lna_mode_t;

/**定义设置命令BB_SET_LNA的输入参数结构*/
typedef struct {
    uint8_t             lna_bypass;                             /**<@note 1=lna bypasss 0=lna*/
} bb_set_lna_t;

/**定义设置命令BB_SET_POWER_RANGE的输入参数结构*/
typedef struct {
    uint8_t pwr_min;                                            /**<@note 最小power, 范围[5,32]*/
    uint8_t pwr_max;                                            /**<@note 最大power, 范围[5,32]*/
    uint8_t dir_mask;                                           /**<@note 类型：bb_dir_e*/
    uint8_t refresh;                                            /**<@note 是否刷新当前功率*/
    uint8_t rsv[28];
} bb_set_power_range_in_t;

/**定义tr switch hook的参数结构*/
typedef struct {
    uint8_t     user;                                           /**<@note 基带用户 类型：bb_user_e*/
    uint8_t     dir;                                            /**<@note 收发方向 类型：bb_dir_e*/
    uint8_t     padding[2];
    uint32_t    freq_khz;                                       /**<@note 收发频点*/
} bb_tr_switch_t;

/**BB_SET_DFS*/
typedef struct {
    int16_t pwr_min;                                            /**<@note 扫频分级能量起始值*/
    int16_t pwr_max;                                            /**<@note 扫频分级能量结束值*/
    uint8_t gain;                                               /**<@note dfs检测参数gain*/
    uint8_t detected_mask;                                      /**<@note detected类型掩码*/
    uint8_t dynamic;                                            /**<@note dynamic gain*/
} bb_dfs_polic_t;

typedef enum {
    BB_DFS_POLIC_LEVEL0,                                        /**<@note 工作环境策略等级*/
    BB_DFS_POLIC_LEVEL1,
    BB_DFS_POLIC_LEVEL2,
    BB_DFS_POLIC_LEVEL3,
    BB_DFS_POLIC_LAB,                                           /**<@note 实验室环境策略*/
    BB_DFS_POLIC_MAX
} bb_dfs_polic_e;

typedef struct {
    uint8_t mode:4;                                             /**<@note 0-disable dfs frame, 1-enable dfs frame, 开启或关闭dfs帧*/
    uint8_t ctrl:4;                                             /**<@note 0-manu, 1-auto*/
    uint8_t chan;                                               /**<@note 指定检测的chan*/
    uint8_t type:4;                                             /**<@note 认证类型bb_dfs_type_e*/
    uint16_t clear_expir;                                       /**<@note 定时清理dfs寄存器，单位ms*/
    uint8_t gain_db_in;                                         /**<@note dfs 参数*/
    int8_t actual_rx_power_in;                                  /**<@note dfs 参数*/
    uint16_t freq_offset;                                       /**<@note dfs 接收频偏, 单位khz*/

#if 0
    uint8_t lab_chan_count;                                     /**<@note 实验室环境大于噪声频道数量阈值*/
    uint16_t lcc_expir;                                         /**<@note 实验室环境大于噪声频道数量阈值有效窗口*/
    int16_t noise;                                              /**<@note 扫频环境噪声阈值*/
    uint16_t pwr_expir;                                         /**<@note 扫描能量值有效窗口*/
    bb_dfs_polic_t polics[BB_DFS_POLIC_MAX];                    /**<@note 扫频分级检测参数*/
#endif
} bb_conf_dfs_t;

typedef struct {
    uint32_t mode:1;
    uint32_t ctrl:1;
    uint32_t chan:1;
    uint32_t type:1;
    uint32_t clear_expir:1;
    uint32_t gain_db_in:1;
    uint32_t actual_rx_power_in:1;
    uint32_t freq_offset:1;

#if 0
    uint32_t lab_cc:1;
    uint32_t lcc_expir:1;
    uint32_t noise:1;
    uint32_t pwr_expir:1;
    uint32_t polic:1;
#endif
} bb_conf_dfs_mask_t;

typedef struct {
    uint32_t chan:8;
    uint32_t event:4;                                            /**<@note dfs动作：0-stop, 1-start*/
} bb_dfs_act_t;

typedef struct {
    uint32_t sub_cmd:8;
    union {
        struct {
            bb_conf_dfs_t conf;
            bb_conf_dfs_mask_t mask;
        };
        bb_dfs_act_t act;
    } u;
} bb_set_dfs_t;

typedef struct {
    int slot;
    int port;
} QUERY_TX_IN;

typedef struct {
    uint64_t buff_index;
    int32_t  buff_len;
    int32_t  buff_avail;
} QUERY_TX_OUT;

typedef struct {
    int32_t buff_len;
} QUERY_RX_OUT;

typedef struct {
    int32_t buff_len;
} BUFF_LIMIT;

/**定义配对结果异步事件结构体*/
typedef struct {
    int32_t ret;                                               /**<@note 调用结果*/
} bb_event_pair_result_t;

/**定义设置命令BB_SET_HOT_UPGRADE_CRC32的输出参数结构*/

typedef bb_set_hot_upgrade_write_out_t bb_set_hot_upgrade_crc32_out_t; /**<@note 复用结构*/

typedef enum
{
    BB_BOOT_REASON_POWER_OFF = 0,                               /**<@note 下电重启*/
    BB_BOOT_REASON_REBOOT,                                      /**<@note Reboot接口重启*/
    BB_BOOT_REASON_MALLOC_FAIL,                                 /**<@note malloc失败后看门狗重启*/
    BB_BOOT_REASON_STACK_OVERFLOW,                              /**<@note Overflow后看门狗重启*/
    BB_BOOT_REASON_LOAD_INSTRUCTION,                            /**<@note Load Instruction crash后看门狗重启*/
    BB_BOOT_REASON_ILLEGAL_INSTRUCTION,                         /**<@note Illegal Instruction crash后看门狗重启*/
    BB_BOOT_REASON_DEBUG_BREAK,                                 /**<@note Debug break 看门狗重启*/
    BB_BOOT_REASON_LOAD_INSTRUCTION_UNALIGNED,                  /**<@note Load Instruction unaligned crash后看门狗重启*/
    BB_BOOT_REASON_LOAD_INSTRUCTION_ACCESS_ERROR,               /**<@note Load Instruction access crash后看门狗重启*/
    BB_BOOT_REASON_STORE_INSTRUCTION_UNALIGNED,                 /**<@note Store Instruction unaligned crash后看门狗重启*/
    BB_BOOT_REASON_STORE_INSTRUCTION_ACCESS_ERROR,              /**<@note Store Instruction access crash后看门狗重启*/
    BB_BOOT_REASON_USER_MODE_ENVIRONMENT,                       /**<@note User mode enviremnment 后看门狗重启*/
    BB_BOOT_REASON_MACHINE_MODE_ENVIRONMENT,                    /**<@note Machine mode enviremnment 后看门狗重启*/
    BB_BOOT_REASON_UNKNOWN,                                     /**<@note 未知原因重启*/
    BB_BOOT_REASON_ENTER_RECOVERY_MODE,                         /**<@note 未知原因重启*/
    BB_BOOT_REASON_MAX
}ENUM_BB_BOOT_REASON;

/**定义读取命令BB_GET_BOOT_REASON的输出参数结构*/
typedef struct {
    uint32_t boot_reason;                                       /**<@note 重启原因，参考ENUM_BB_BOOT_REASON*/
    uint32_t pc;                                                /**<@note crash时的pc*/
    uint32_t ra;                                                /**<@note crash时的ra*/
    uint32_t ex_data;                                           /**<@note crash时的其他额外信息*/
    uint32_t repeat_time;                                       /**<@note 相同重启原因的重复次数*/
    char desc[64];                                              /**<@note 重启原因描述*/
} bb_get_boot_reason_out_t;

/**定义读取命令BB_GET_CLEAN_MODE_MSG_CHECK的输入参数结构*/
typedef struct {
    uint32_t msg_id;                                            /**<@note 需要检测的msg_id*/
} bb_get_clean_mode_msg_check_in_t;

/**定义读取命令BB_GET_CLEAN_MODE_MSG_CHECK的输出参数结构*/
typedef struct {
    uint8_t valid;                                              /**<@note 0:不可使用, >0:可使用*/
    uint8_t rsv[3];
} bb_get_clean_mode_msg_check_out_t;

/**定义读取命令BB_GET_RECOVERY_MODE的输出参数结构*/
typedef struct {
    uint8_t mode;                                               /**<@note mode=1为recovery mode， mode=0为正常模式 */
} bb_get_recovery_mode_out_t;

/**定义读取命令BB_GET_DEV_CONNECT_SLOT的输出参数结构*/
typedef struct {
    uint8_t slot;                                               /**<@note 未连接或者AP角色下，返回BB_SLOT_MAX */
} bb_get_dev_connect_slot_out_t;

/**定义设置命令BB_SET_FORCE_UPD_SOCKET_ENC的输入参数结构*/
typedef struct {
    uint8_t slot;                                               /**<@note 设置的目标slot*/
    uint8_t port;                                               /**<@note 设置的目标port*/
    bb_sock_upd_enc_t sock_upd_enc;                             /**<@note 设置的目标加密配置*/
} bb_set_force_upd_socket_enc_t;

/** @} */  /** <!-- ==== Structure Definition end ==== */

/******************************* API Definition *********************************/
/** \addtogroup      BB_API
 * @brief Artosyn 8030基带API
 * @{
 */

// system plane
/**
\brief 连接远程bb_host服务器
\param[out] phost           :   用于返回host设备句柄
\param[in] addr             :   host的ip
\param[in] addr             :   host端口号
\retval ::<0                :   失败，其值为错误码
\retval ::0                 :   成功
\see \n
N/A
*/
AR8030_API int bb_host_connect(bb_host_t** phost, const char* addr, int port);

/**
\brief 断开远程bb_host服务器
\retval ::<0                :   失败，其值为错误码
\retval ::0                 :   成功
\see \n
N/A
*/
AR8030_API int bb_host_disconnect(bb_host_t* phost);

/**
\brief 本API为RPC接口，用于打开指定的8030设备
\param[in] devs             :   用于返回host侧的8030设备 必须是由 @ref bb_dev_getlist 返回的数据
\retval ::not NULL          :   打开成功 返回8030句柄
\retval ::NULL              :   打开失败
\see \n
N/A
*/
AR8030_API bb_dev_handle_t* bb_dev_open(bb_dev_t* devs);

/**
\brief 本API为RPC接口，关闭指定的8030设备
\param[in] handle           :   用于返回host侧的8030设备
\retval ::==0               :   成功
\retval ::<0                :   失败，其值为错误码
\see \n
N/A
*/
AR8030_API int bb_dev_close(bb_dev_handle_t* handle);

/**
\brief 基带软件初始化，配置类型应该先于此API执行
\retval ::0                 :   成功
\retval ::non-zero          :   失败，其值为错误码
\see \n
N/A
*/
AR8030_API int bb_init(bb_dev_handle_t* handle);

/**
\brief 基带软件反初始化
\retval ::0                 :   成功
\retval ::non-zero          :   失败，其值为错误码
\see \n
N/A
*/
AR8030_API int bb_deinit(bb_dev_handle_t* handle);

/**
\brief 基带软件开始工作，此API必须在bb_init之后才能执行
\retval ::<0                :   失败，其值为错误码
\retval ::0                 :   成功
\retval ::>0                :   表示可用8030设备的数量
\see \n
N/A
*/
AR8030_API int bb_start(bb_dev_handle_t* handle);

/**
\brief 基带软件停止工作
\retval ::0                 :   成功
\retval ::non-zero          :   失败，其值为错误码
\see \n
N/A
*/
AR8030_API int bb_stop(bb_dev_handle_t* handle);

/**
\brief 本API为RPC接口，用于在host侧 用于注册热插拔回调函数
\param[in] phost            :   指定目标host
\param[in] cbfun            :   注册热插拔函数
\param[in] priv             :   用户数据
\retval ::0                 :   成功
\retval ::<0                :   失败，其值为错误码
\see \n
N/A
*/
AR8030_API int bb_dev_reg_hotplug_cb(bb_host_t* phost, bb_event_callback cbfun, void* priv);

/**
\brief 本API为RPC接口，用于在host侧 获取8030设备列表
\param[in] phost            :   指定目标host
\param[out] plist           :   用于返回8030的设备列表
\retval ::>0                :   成功，其值为返回的8030设备数量
\retval ::<0                :   失败，其值为错误码
\see \n
N/A
*/
AR8030_API int bb_dev_getlist(bb_host_t* phost, bb_dev_list_t ** plist);

/**
\brief 本API为RPC接口，用于在host侧 释放由 bb_dev_getlist 获取的列表
\param[in] plist            :   用于返回8030的设备列表
\retval ::==0               :   成功
\retval ::<0                :   失败，其值为错误码
\see \n
N/A
*/
AR8030_API int bb_dev_freelist(bb_dev_list_t * plist);

/**
\brief 获取8030的设备信息
\param[in] pdev             :   8030设备
\param[out] dev_info        :   8030设备信息
\retval ::==0               :   成功
\retval ::<0                :   失败，其值为错误码
\see \n
N/A
*/
AR8030_API int bb_dev_getinfo(bb_dev_t* pdev, bb_dev_info_t* dev_info);

/**
\brief 测试远程bb_host服务器是否可连接
\param[in] addr             :   host的ip
\param[in] addr             :   host端口号
\retval ::<0                :   失败，其值为错误码
\retval ::0                 :   成功
\see \n
N/A
*/
AR8030_API int bb_host_connect_test(const char* addr, int port);

/**
\brief 基带统一控制接口
\param[in] dev              :   目标8030设备，本地调用可填NULL
\param[in] request          :   请求类型码
\param[in] in               :   请求码对应的入口参数
\param[out] out             :   请求码对应的出口参数，对于配置和设置类请求，此参数可设置为NULL
\retval ::0                 :   成功
\retval ::non-zero          :   失败，其值为错误码
\see \n
N/A
*/
AR8030_API int bb_ioctl(bb_dev_handle_t* dev, uint32_t request, const void* in, void* out);

/**
\brief 基带统一控制接口，支持超时时间，超时后返回错误
\param[in] dev              :   目标8030设备，本地调用可填NULL
\param[in] request          :   请求类型码
\param[in] in               :   请求码对应的入口参数
\param[out] out             :   请求码对应的出口参数，对于配置和设置类请求，此参数可设置为NULL
\param[in] timeout          :   超时时间，单位ms
\retval ::0                 :   成功
\retval ::non-zero          :   失败，其值为错误码
\see \n
N/A
*/
AR8030_API int bb_ioctl_ex(bb_dev_handle_t* dev, uint32_t request, const void* input, void* output, int timeout);

// socket plane
/**
\brief 打开一个数传socket，根据输入参数，socket可以确定通讯的对象设备，逻辑端口，传输方向以及SDK用于收发的buffer大小
\param[in] dev              :   目标8030设备，本地调用可填NULL
\param[in] slot             :   目标SLOT，如果为DEV，目标SLOT均为BB_SLOT_AP
\param[in] port             :   逻辑端口，不同端口的数据互相独立，port的数量受配置宏BB_CONFIG_MAX_TRANSPORT_PER_SLOT控制
\param[in] flag             :   创建socket的控制标志，见BB_SOCK_FLAG_RX,BB_SOCK_FLAG_TX等宏
\param[in] opt              :   创建socket的用户选项，如设置为NULL，表示不做特殊设置
\retval ::>= 0              :   成功，且返回值为socket的id
\retval ::< 0               :   失败，其值为错误码
\see \n
N/A
*/
AR8030_API int bb_socket_open(bb_dev_handle_t* dev, bb_slot_e slot, uint32_t port, uint32_t flag, bb_sock_opt_t* opt);

/**
\brief 关闭指定的数传socket
\param[in] sockfd           :   创建socket时返回的id
\retval ::= 0               :   成功
\retval ::< 0               :   失败，其值为错误码
\see \n
N/A
*/
AR8030_API int bb_socket_close(int sockfd);

/**
\brief 从指定的socket读取数据，注意：只要有数据读到，此API马上返回，并不会装满buffer再返回
\param[in] sockfd           :   socket的id
\param[out] buf             :   用于接收数据的用户buffer
\param[in] len              :   接收数据的用户buffer大小
\param[in] timeout          :   读取数据的等待时间，单位为毫秒，如果timeout<0，则没有超时限制
\retval ::>= 0              :   成功，且返回值为读到数据长度
\retval ::< 0               :   失败，其值为错误码
\see \n
N/A
*/
AR8030_API int bb_socket_read(int sockfd, void* buf, uint32_t len, int timeout);

/**
\brief 向指定的socket写入数据，注意：写入指的是数据进入基带的发送队列，并不是实际已发送完成
\param[in] sockfd           :   socket的id
\param[in] buf              :   用户发送数据的buffer
\param[in] len              :   用户发送数据大小
\param[in] timeout          :   写入数据的等待时间，单位为毫秒，如果timeout<0，则没有超时限制
\retval ::>= 0              :   成功，且返回值为已发送数据长度
\retval ::< 0               :   失败，其值为错误码
\see \n
N/A
*/
AR8030_API int bb_socket_write(int sockfd, void* buf, uint32_t len, int timeout);

/**
\brief 控制和修改数传socket的行为和属性
\param[in] sockfd           :   socket的id
\param[in] cmd              :   控制命令码
\param[in] value            :   控制命令码对应的输入参数
\retval ::= 0               :   成功
\retval ::< 0               :   失败，其值为错误码
\see \n
N/A
*/
AR8030_API int bb_socket_ioctl(int sockfd, bb_sock_cmd_e cmd, void* value);

/**
\brief 从指定的socket请求数据，此API是bb_socket_read的升级版本，减少一次内存复制，此API仅chip侧支持
\param[in] sockfd           :   socket的id
\param[in] len              :   请求的数据长度
\param[out] mem             :   请求到的数据地址与长度信息
\param[in] timeout          :   请求数据的等待时间，单位为毫秒，如果timeout<0，则没有超时限制
\retval ::= 0               :   成功
\retval ::< 0               :   失败，其值为错误码
\see \n
N/A
*/
AR8030_API int bb_socket_query(int sockfd, uint32_t len, bb_query_mem_t* mem, int timeout);

/**
\brief 将bb_socket_query得到的内存从SDK内部释放掉，此API仅chip侧支持，并且严格与bb_socket_query配对使用
\param[in] sockfd           :   socket的id
\param[in] mem              :   请求到的数据地址与长度信息
\retval ::= 0               :   成功
\retval ::< 0               :   失败，其值为错误码
\see \n
N/A
*/
AR8030_API int bb_socket_release(int sockfd, const bb_query_mem_t* mem);

/**
\brief 获取发送了多少字节数
\param[in] psoc             :   socket的id
\param[in] ipt              :   input buffer
\param[in] ipt_size         :   input buffer length
\param[in] opt              :   output buffer
\param[in] opt_size         :   output buffer length
\retval ::= 0               :   成功
\retval ::< 0               :   失败，其值为错误码
\see \n
N/A
*/
AR8030_API int bb_socket_ioctl_tx_len_get(int sockfd, void* ipt, int ipt_size, void* opt, int opt_size);

/**
\brief 重新计数发送的字节数
\param[in] psoc             :   socket的id
\param[in] ipt              :   input buffer
\param[in] ipt_size         :   input buffer length
\param[in] opt              :   output buffer
\param[in] opt_size         :   output buffer length
\retval ::= 0               :   成功
\retval ::< 0               :   失败，其值为错误码
\see \n
N/A
*/
AR8030_API int bb_socket_ioctl_tx_len_rst(int sockfd, void* ipt, int ipt_size, void* opt, int opt_size);

/**
\brief 更新socket加密相关参数
\param[in] sockfd           :   socket的id
\param[in] ipt              :   input buffer
\param[in] ipt_size         :   input buffer length
\param[in] opt              :   output buffer
\param[in] opt_size         :   output buffer length
\retval ::= 0               :   成功
\retval ::< 0               :   失败，其值为错误码
\see \n
N/A
*/
AR8030_API int bb_socket_ioctl_enc_set(int sockfd, void* ipt, int ipt_size, void* opt, int opt_size);

/**
\brief 获取socket加密相关参数
\param[in] sockfd           :   socket的id
\param[in] ipt              :   input buffer
\param[in] ipt_size         :   input buffer length
\param[in] opt              :   output buffer
\param[in] opt_size         :   output buffer length
\retval ::= 0               :   成功
\retval ::< 0               :   失败，其值为错误码
\see \n
N/A
*/
AR8030_API int bb_socket_ioctl_enc_get(int sockfd, void* ipt, int ipt_size, void* opt, int opt_size);

// debug channel
AR8030_API void bb_debug_cmd(const void* data, uint32_t size);

AR8030_API uart_list_hd bb_uart_get_list(bb_host_t* phost);
AR8030_API int          bb_uart_setup(bb_host_t* phost, uart_ioctl* opt);

typedef struct {
    char names[128];
    int  level;
} daemon_dbg_ctrl;

AR8030_API void            bb_daemon_set_dbg_level(bb_host_t* phost, daemon_dbg_ctrl* pctrl);
AR8030_API daemon_dbg_ctrl bb_daemon_get_dbg_level(bb_host_t* phost, const char * names);

AR8030_API int bb_daemon_exec(bb_host_t* phost, const char* cmd, int cmdlen, char* retcmd, int retmax);

AR8030_API int bb_ioctl_set_hook2(bb_cb_type_e type, void* f);
AR8030_API void *bb_ioctl_get_hook2(bb_cb_type_e type);
/** @} */  /** <!-- ==== API Definition end ==== */

#ifdef __cplusplus
}
#endif

#endif

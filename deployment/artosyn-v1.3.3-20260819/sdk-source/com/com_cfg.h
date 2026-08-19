#ifndef __COM_CFG_H__
#define __COM_CFG_H__

typedef enum {
    so_open  = 0,
    so_write = 1,
    so_read  = 2,
    so_close = 3,

    so_write_ret = 4,

    so_query_len    = 0x90,
    so_set_tx_limit = 0x91,
    so_get_tx_limit = 0x92,

    so_user_base_start = 0xc0,
    so_user_base_end   = 0xff,
} so_cmd_opt;

#define SOCK_LEN_DAEMON_TO_APP (200 * 1024)
#define SOCK_LEN_APP_TO_DAEMON (256 * 1024)

#define SUBSCRIBE_REQ     1
#define SUBSCRIBE_REQ_RET 2
#define SUBSCRIBE_DAT_RET 3
#define SUBSCRIBE_REQ_FAL 4

#endif

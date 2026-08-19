
#include "bb_api.h"
#include "session_platform_ctrl.h"
#include <stddef.h>

AR8030_API uart_list_hd bb_uart_get_list(bb_host_t* phost)
{
    uart_list_hd ret = {
        .num = 0,
    };

    int ctrl = platform_com_ctrl(phost, BB_RPC_SERIAL_LIST, NULL, 0, &ret, sizeof(ret));

    return ret;
}

AR8030_API int bb_uart_setup(bb_host_t* phost, uart_ioctl* opt)
{
    int ctrl = platform_com_ctrl(phost, BB_RPC_SERIAL_SETUP, opt, sizeof(uart_ioctl), 0, 0);

    return ctrl;
}

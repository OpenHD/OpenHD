
#include "bb_api.h"
#include "session_platform_ctrl.h"
#include <string.h>

AR8030_API daemon_dbg_ctrl bb_daemon_get_dbg_level(bb_host_t* phost, const char* names)
{
    daemon_dbg_ctrl dctrl;
    strcpy(dctrl.names, names);

    int ctrl = platform_com_ctrl(phost, BB_RPC_GET_DEBUG_LV, &dctrl, sizeof(dctrl), &dctrl, sizeof(dctrl));

    return dctrl;
}

AR8030_API void bb_daemon_set_dbg_level(bb_host_t* phost, daemon_dbg_ctrl* opt)
{
    int ctrl = platform_com_ctrl(phost, BB_RPC_SET_DEBUG_LV, opt, sizeof(daemon_dbg_ctrl), 0, 0);
}

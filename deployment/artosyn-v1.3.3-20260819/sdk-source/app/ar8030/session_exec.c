#include "bb_api.h"
#include "session_platform_ctrl.h"

AR8030_API int bb_daemon_exec(bb_host_t* phost, const char* cmd, int cmdlen, char* retcmd, int retmax)
{
    int ctrl = platform_com_ctrl(phost, BB_RPC_HOST_EXEC, (void*)cmd, cmdlen, retcmd, retmax);
    return ctrl;
}

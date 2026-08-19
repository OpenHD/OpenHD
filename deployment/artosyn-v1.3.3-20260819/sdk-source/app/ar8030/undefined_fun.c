#include "bb_api.h"

AR8030_API int bb_socket_query(int sockfd, uint32_t len, bb_query_mem_t* mem, int timeout)
{
    return -999;
}
AR8030_API int bb_socket_release(int sockfd, const bb_query_mem_t* mem)
{
    return -999;
}
AR8030_API void bb_debug_cmd(const void* data, uint32_t size) { }

AR8030_API int bb_socket_ioctl_tx_len_get(int sockfd, void* ipt, int ipt_size, void* opt, int opt_size)
{
    return -1;
}
AR8030_API int bb_socket_ioctl_tx_len_rst(int sockfd, void* ipt, int ipt_size, void* opt, int opt_size)
{
    return -1;
}

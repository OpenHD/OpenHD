#ifndef __AR8030_H__
#define __AR8030_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "bb_api.h"
#include <stdint.h>

#define RX_BUF_LEN (100 * 1024)

typedef struct bb_sock_opt_t bb_sock_opt_t;

typedef struct bb_dev_t        bb_dev_t;
typedef struct bb_dev_handle_t bb_dev_handle_t;
typedef struct bb_host_t       bb_host_t;
typedef struct bb_dev_info_t   bb_dev_info_t;

AR8030_API int bb_init(bb_dev_handle_t* handle);
AR8030_API int bb_deinit(bb_dev_handle_t* handle);
AR8030_API int bb_start(bb_dev_handle_t* handle);
AR8030_API int bb_stop(bb_dev_handle_t* handle);

AR8030_API int bb_host_connect(bb_host_t** phost, const char* addr, int port);
AR8030_API int bb_host_disconnect(bb_host_t* phost);

AR8030_API int              bb_dev_reg_hotplug_cb(bb_host_t* phost, bb_event_callback cbfun, void* priv);
AR8030_API int              bb_dev_getlist(bb_host_t* phost, bb_dev_list_t** pdev);
AR8030_API int              bb_dev_freelist(bb_dev_list_t* plist);
AR8030_API int              bb_dev_getinfo(bb_dev_t* pdev, bb_dev_info_t* dev_info);
AR8030_API bb_dev_handle_t* bb_dev_open(bb_dev_t* pdev);
AR8030_API int              bb_dev_close(bb_dev_handle_t* pdev);

AR8030_API int bb_ioctl(bb_dev_handle_t* dev, uint32_t request, const void* input, void* output);
AR8030_API int bb_ioctl_ex(bb_dev_handle_t* dev, uint32_t request, const void* input, void* output, int timeout);
AR8030_API int bb_socket_open(bb_dev_handle_t* dev, bb_slot_e usr, uint32_t port, uint32_t flg, bb_sock_opt_t* opt);
AR8030_API int bb_socket_write(int sockfd, void* buff, uint32_t len, int timeout);
AR8030_API int bb_socket_read(int sockfd, void* outbuff, uint32_t len, int timeout);
AR8030_API int bb_socket_close(int sockfd);
AR8030_API int bb_host_connect_test(const char* addr, int port);

#ifdef __cplusplus
}
#endif

#endif

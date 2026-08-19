#ifndef __DAEMON_H__
#define __DAEMON_H__

#ifdef __cplusplus
extern "C" {
#endif
typedef struct rpc_info rpc_info;

rpc_info* rpc_init(int port, const char* udsname);
int       reg_usbrpc_platfrom(rpc_info* prpc);
int       dev8030_poll(rpc_info* prpc);

int reg_usbrpc_hostjnimode_platform(rpc_info* prpc);
int usbrpc_parse_usbfd(rpc_info*   prpc,
                       int         fd,
                       int         iface,
                       int         epin,
                       int         insz,
                       int         epout,
                       int         outsz,
                       const char* serial,
                       int         serlen,
                       const char* path,
                       int         pathlen);

int usbrpc_remove_usbfd(rpc_info* prpc, int fd);

#ifdef __cplusplus
}
#endif
#endif

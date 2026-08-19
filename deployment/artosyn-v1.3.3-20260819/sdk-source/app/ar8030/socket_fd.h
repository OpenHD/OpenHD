#ifndef __SOCKET_FD_H__
#define __SOCKET_FD_H__

typedef struct SOCK_SESSION    SOCK_SESSION;
typedef struct bb_dev_handle_t bb_dev_handle_t;

void socket_session_init(void);

void socket_del_fd(int fd);
int  socket_add_fd(bb_dev_handle_t* pdev, SOCK_SESSION* sock);
void socket_del_dev(bb_dev_handle_t* pdev);

SOCK_SESSION* socket_get_session(int fd);
#endif

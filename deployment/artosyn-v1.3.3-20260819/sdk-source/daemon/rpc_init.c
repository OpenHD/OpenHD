#include "com_log.h"
#include "rpc_node.h"
#include "socketfd_port.h"
#include "usb_event_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int create_tcp_listen(const char* port, SOCKETFD* listenfd)
{
    int      iResult;
    SOCKETFD ListenSocket = -1;
#ifdef WIN32
    WSADATA wsaData;
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        com_log(COM_NET, "WSAStartup failed with error: %d", iResult);
        return 1;
    }
#endif
    struct addrinfo* result = NULL;
    struct addrinfo  hints  = {
          .ai_family   = AF_INET,
          .ai_socktype = SOCK_STREAM,
          .ai_protocol = IPPROTO_TCP,
          .ai_flags    = AI_PASSIVE,
    };

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, port, &hints, &result);
    if (iResult != 0) {
        com_log(COM_NET, "getaddrinfo failed with error: %d", iResult);
        return 1;
    }

    // Create a SOCKET for the server to listen for client connections.
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == -1) {
#ifdef WIN32
        com_log(COM_NET, "socket failed with error: %ld", WSAGetLastError());
        freeaddrinfo(result);
#else
        freeaddrinfo(result);
        com_log(COM_NET, "socket failed with error: %d", errno);
#endif
        return 2;
    }

#ifndef WIN32
    // win禁止 下防止重复绑定
    // 仅限linux使用
    const int trueFlag = 1;
    if (setsockopt(ListenSocket, SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof(int)) < 0) {
        com_log(COM_NET, "set reuse addr error");
    }
#endif

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    freeaddrinfo(result);

    if (iResult == -1) {
#ifdef WIN32
        com_log(COM_NET, "tcp bind failed with error: %d", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
#else
        com_log(COM_NET, "tcp bind failed with error: %d", errno);
        close(ListenSocket);
#endif
        return 3;
    }

    *listenfd = ListenSocket;
    return 0;
}

static void clean_up_node(rpc_node* prpc, threadinfo* tinfo)
{
    if (prpc) {
        prpc->pact->end(prpc);
    }
#ifdef WIN32
    closesocket(tinfo->fd);
#else
    close(tinfo->fd);
#endif
    free(tinfo);
}

static void* nod_read_thread(void* p)
{
    threadinfo*    tinfo = (threadinfo*)p;
    rpc_node*      prpc  = NULL;
    unsigned char* buff  = malloc(500 * 1024);

    com_log(COM_NET, "start rpc tf@%p, fd = %d", tinfo, tinfo->fd);

    if (!buff) {
        socke_close(tinfo->fd);
        free(tinfo);
        return NULL;
    }
    while (1) {
        int rdlen = recv(tinfo->fd, (char*)buff, 500 * 1024, 0);
        if (rdlen == 0) {
            break;
        } else if (rdlen < 0) {
            break;
        }

        if (!prpc) {
            int ret = rpc_recv_chk_init(buff, rdlen, tinfo, &prpc);

            if (ret) {
                continue;
            }
        }

        if (prpc) {
            prpc->pact->rpc_rd_cb(prpc, buff, rdlen);
        }
    }
    com_log(COM_NET, "close rpc tf@%p, fd = %d", tinfo, tinfo->fd);
    clean_up_node(prpc, tinfo);
    free(buff);
    return NULL;
}

void rpc_close_listen(rpc_info* pinfo)
{
    pthread_rwlock_wrlock(&pinfo->rpc_listen_lk);

    while (!list_empty(&pinfo->rpc_listen_head)) {
        rpc_listen_node* tmplisten = list_first_entry(&pinfo->rpc_listen_head, rpc_listen_node, node);

        list_del(&tmplisten->node);
#ifdef WIN32
        closesocket(tmplisten->listen_socket);
#else
        close(tmplisten->listen_socket);
#endif
        free(tmplisten);
    }

    pthread_rwlock_unlock(&pinfo->rpc_listen_lk);
}

static void* rpc_listen_thread(void* p)
{
    rpc_listen_node* listen_nod = (rpc_listen_node*)p;
    rpc_info*        pinfo      = listen_nod->pinfo;
    int              iResult;
    SOCKETFD         client;

    while (1) {
        iResult = listen(listen_nod->listen_socket, SOMAXCONN);
        if (iResult == -1) {
#ifdef WIN32
            com_log(COM_NET, "listen failed with error: %d", WSAGetLastError());
#else
            com_log(COM_NET, "listen failed with error: %d", errno);
#endif
            break;
        }
        client  = accept(listen_nod->listen_socket, NULL, NULL);
        if (client == -1) {
#ifdef WIN32
            com_log(COM_NET, "accept failed with error: %d", WSAGetLastError());
#else
            com_log(COM_NET, "accept failed with error: %d", errno);
#endif
            break;
        }

        int flg = 1;
        setsockopt(client, IPPROTO_TCP, TCP_NODELAY, (char*)&flg, sizeof(flg));

        threadinfo* tinfo = (threadinfo*)malloc(sizeof(threadinfo));
        if (!tinfo) {
#ifdef WIN32
            closesocket(client);
#else
            close(client);
#endif
            continue;
        }
        tinfo->fd        = client;
        tinfo->plist     = NULL; ///< 要等到rpc通讯后才知道具体plist对象
        tinfo->prpc_info = pinfo;

        pthread_create(&tinfo->rd_thread, NULL, nod_read_thread, tinfo);
        pthread_detach(tinfo->rd_thread);
    }

    pthread_rwlock_wrlock(&pinfo->rpc_listen_lk);
    list_del(&listen_nod->node);
    pthread_rwlock_unlock(&pinfo->rpc_listen_lk);
#ifdef WIN32
    closesocket(listen_nod->listen_socket);
#else
    close(listen_nod->listen_socket);
#endif
    com_log(COM_NET, "stop listen , id = %d", listen_nod->id);
    free(listen_nod);

    return NULL;
}
#ifdef ENABLE_UDS
static int create_uds_listen(const char* path, SOCKETFD* listenfd)
{
    int iResult;
#ifdef WIN32
    WSADATA wsaData;
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        com_log(COM_NET, "WSAStartup failed with error: %d", iResult);
        return 1;
    }
#endif
    SOCKETFD ListenSocket = -1;

    // Create a SOCKET for the server to listen for client connections.
    ListenSocket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (ListenSocket == -1) {
#ifdef WIN32
        com_log(COM_NET, "socket failed with error: %ld", WSAGetLastError());
        WSACleanup();
#else
        com_log(COM_NET, "socket failed with error: %d", errno);
#endif
        return 2;
    }
#ifdef WIN32
    SOCKADDR_UN ServerSocket = { 0 };
#else
    struct sockaddr_un ServerSocket;
#endif

    ServerSocket.sun_family = AF_UNIX;
#ifdef WIN32
    strncpy_s(ServerSocket.sun_path, sizeof(ServerSocket.sun_path), path, strlen(path));
#else
    memset(ServerSocket.sun_path, 0, sizeof(ServerSocket.sun_path));
    strncpy(ServerSocket.sun_path, path, strlen(path));
#endif
    // Setup the TCP listening socket
    iResult = bind(ListenSocket, (struct sockaddr*)&ServerSocket, sizeof(ServerSocket));
    if (iResult == -1) {
#ifdef WIN32
        com_log(COM_NET, "uds bind failed with error: %d", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
#else
        com_log(COM_NET, "uds bind failed with error: %d", errno);
        close(ListenSocket);
#endif
        return 3;
    }

    *listenfd = ListenSocket;
    return 0;
}
#endif

rpc_info* rpc_init(int port[], int portlen, const char* udsname)
{
    rpc_info* pinfo = (rpc_info*)malloc(sizeof(rpc_info));

    if (!pinfo) {
        com_log(COM_NET, "malloc rpc_info fail");
        return NULL;
    }
    pinfo->nxt_working_id    = 0;
    pinfo->rpc_listen_nxt_id = 0;
    // 注册的平台
    INIT_LIST_HEAD(&pinfo->head);
    pthread_rwlock_init(&pinfo->head_lk, NULL);

    // 通知上下线
    INIT_LIST_HEAD(&pinfo->head_hotplug_cb);
    pthread_rwlock_init(&pinfo->head_hotplug_lk, NULL);

    // 监听列表
    INIT_LIST_HEAD(&pinfo->rpc_listen_head);
    pthread_rwlock_init(&pinfo->rpc_listen_lk, NULL);

    for (int i = 0; i < portlen; i++) {
        char buff[10];
#ifdef WIN32
        sprintf_s(buff, 10, "%d", port[i]);
#else
        sprintf(buff, "%d", port[i]);
#endif
        rpc_listen_node* nxt_listen = malloc(sizeof(rpc_listen_node));
        int              ret        = create_tcp_listen(buff, &nxt_listen->listen_socket);
        if (ret) {
            free(nxt_listen);
            com_log(COM_NET, "listen tcp error = %d @port = %d", ret, port[i]);
        } else {
            nxt_listen->id = pinfo->rpc_listen_nxt_id++;
            list_add_tail(&nxt_listen->node, &pinfo->rpc_listen_head);
            com_log(COM_NET, "listen add tcp @port = %d ,id = %d ", port[i], nxt_listen->id);
            nxt_listen->pinfo = pinfo;
            pthread_create(&nxt_listen->listen_thread, NULL, rpc_listen_thread, nxt_listen);
            pthread_detach(nxt_listen->listen_thread);
        }
    }

    if (list_empty(&pinfo->rpc_listen_head)) {
        com_log(COM_NET, "no listen port , rpc exit!!");
        pthread_rwlock_destroy(&pinfo->head_lk);
        pthread_rwlock_destroy(&pinfo->head_hotplug_lk);
        pthread_rwlock_destroy(&pinfo->rpc_listen_lk);
        free(pinfo);
        exit(-1);
    }

    return pinfo;
}

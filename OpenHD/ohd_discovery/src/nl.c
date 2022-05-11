#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <linux/types.h>
#include <linux/netlink.h>

#include <netlink/socket.h>
#include <netlink/netlink.h>

#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <net/if.h>

#include "nl80211.h"


typedef struct lookup_data {
    char *name;
    int phyidx;
    bool supports_2ghz;
    bool supports_5ghz;
} lookup_data;


static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg) {
    int *ret = (int*)arg;
    *ret = err->error;
    return NL_STOP;
}


static int finish_handler(struct nl_msg *msg, void *arg) {
    int *ret = (int*)arg;
    *ret = 0;

    return NL_SKIP;
}


static int ack_handler(struct nl_msg *msg, void *arg) {
    int *ret = (int*)arg;
    *ret = 0;
    return NL_SKIP;
}


static int lookup_handler(struct nl_msg *msg, void *arg) {
    struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
    struct nlattr *tb_band[NL80211_BAND_ATTR_MAX + 1];

    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    lookup_data *data = (lookup_data*)arg;


    struct nlattr *nl_band;

    nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);

    int rem_band;
    static int64_t phy_id = -1;

    if (tb_msg[NL80211_ATTR_WIPHY]) {
        int current_phy = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY]);
        if (current_phy != data->phyidx) {
            return NL_SKIP;
        }
    }

    /* needed for split dump */
    if (tb_msg[NL80211_ATTR_WIPHY_BANDS]) {
        nla_for_each_nested(nl_band, tb_msg[NL80211_ATTR_WIPHY_BANDS], rem_band) {
            if (nl_band->nla_type + 1 == 1) {
                //fprintf(stderr, "Found 2ghz band\n");
                data->supports_2ghz = true;
            } else if (nl_band->nla_type + 1 == 2) {
                //fprintf(stderr, "Found 5ghz band\n");
                data->supports_5ghz = true;
            }
        }
    }

    return NL_SKIP;
}


int phy_lookup(char *name, int phyidx, bool *supports_2ghz, bool* supports_5ghz) {    
    struct nl_cb *cb;
    struct nl_msg *msg;
    lookup_data data;
    data.supports_2ghz = false;
    data.supports_5ghz = false;
    int err;

    struct nl_sock* socket = nl_socket_alloc();

    genl_connect(socket);

    data.name = name;
    data.phyidx = phyidx;

    msg = nlmsg_alloc();
    if (!msg) {
        return -ENOMEM;
    }

    cb = nl_cb_alloc(NL_CB_DEFAULT);
    if (!cb) {
        err = -ENOMEM;
        nlmsg_free(msg);
        return err;
    }

    int driverId = genl_ctrl_resolve(socket, "nl80211");

    genlmsg_put(msg, 0, 0, driverId, 0, NLM_F_DUMP, NL80211_CMD_GET_WIPHY, 0);

    err = nl_send_auto_complete(socket, msg);
    if (err < 0) {
        nl_cb_put(cb);
        nlmsg_free(msg);
        return err;
    }

    err = 1;

    nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, lookup_handler, &data);
    nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);
    nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
    nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);

    while (err > 0) {
        nl_recvmsgs(socket, cb);
    }

    nl_cb_put(cb);

    nlmsg_free(msg);

    *supports_2ghz = data.supports_2ghz;
    *supports_5ghz = data.supports_5ghz;

    return err;
}


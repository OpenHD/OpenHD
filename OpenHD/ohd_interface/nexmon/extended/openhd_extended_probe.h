/* Experimental OpenHD receive-channel diagnostics for bcm43455c0/7_45_206.
 * This file is compiled into Nexmon under its GPL-3.0-or-later license.
 * Uses the original firmware setter; does not guess PHY register values.
 */
static int
openhd_extended_probe(struct wlc_info *wlc, char *arg, int len, void *wlc_if)
{
    unsigned int requested, current = 0;
    unsigned int reply[7];
    unsigned int channel;
    char var[13] = "chanspec";
    int frequency = 0, lookup_ret, set_ret = IOCTL_ERROR, get_ret;
    void *channel_info = 0;

    if (len != sizeof(reply)) return IOCTL_ERROR;
    memcpy(&requested, arg, sizeof(requested));
    channel = requested & 0xff;
    /* Restrict this initial experiment to 20 MHz, including a known baseline. */
    if (requested != (0xd000 | channel) ||
        (channel != 165 && channel != 169 && channel != 173 &&
         channel != 177 && channel != 181 && channel != 185))
        return IOCTL_ERROR;

    lookup_ret = wlc_phy_chan2freq_acphy(wlc->band->pi, channel,
                                      &frequency, &channel_info);
    if (frequency == 5000 + 5 * channel && channel_info) {
        memcpy(var + 9, &requested, sizeof(requested));
        set_ret = wlc_ioctl(wlc, WLC_SET_VAR, var, sizeof(var), wlc_if);
    }
    memset(var, 0, sizeof(var));
    memcpy(var, "chanspec", 9);
    get_ret = wlc_ioctl(wlc, WLC_GET_VAR, var, sizeof(var), wlc_if);
    if (!get_ret) memcpy(&current, var, sizeof(current));

    reply[0] = 0x4f484450; /* OHDP protocol magic, version 1 has seven words. */
    reply[1] = requested;
    reply[2] = lookup_ret;
    reply[3] = frequency;
    reply[4] = set_ret;
    reply[5] = get_ret;
    reply[6] = current;
    memcpy(arg, reply, sizeof(reply));
    /* Return the result inside the reply: legacy netlink can hide ioctl errors. */
    return IOCTL_SUCCESS;
}

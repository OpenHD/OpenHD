1) OpenHD service: automatically starts (and restarts in case of a crash) the openhd main executable
2) custom unmanaged camera service: For development / custom camera scripting. By default, does nothing.
    Started by openhd if custom unmanaged camera(s) are selected via sysutils hardware settings
3) crash dump capture service: stores previous boot logs in /Config/openhd/logs for debugging

Optional Pi 4 ZeroCD recovery: `openhd-zerocd-recovery.timer` runs
   `scripts/openhd-zerocd-recovery.py` every 15 seconds. Install the script in
   `/usr/local/sbin/` and both recovery units in `/etc/systemd/system/`, then run
   `systemctl daemon-reload` and `systemctl enable --now openhd-zerocd-recovery.timer`.
   Requires `python3` and `uhubctl`; this is opt-in and is not enabled by image
   builds. Pi 4 USB power is ganged, so recovery only runs with a lone Realtek
   `0bda:1a2b` device behind the onboard hub and no other external USB devices.
   After 90 seconds in CD mode it stops OpenHD/SysUtils, cuts all external USB
   power for 10 seconds, lets normal mode switching run, and starts OpenHD after
   enumeration. Healthy radios do not trigger a reset. Attempts are limited to
   three per boot, at least 120 seconds apart. Inspect
   `journalctl -u openhd-zerocd-recovery.service` for results; disable with
   `systemctl disable --now openhd-zerocd-recovery.timer`.
   Live Pi 4/CU testing showed a stalled CD-mode adapter switch to `0bda:c812`
   after a full-hub cycle; a single-port cycle had not cleared the failure.

4) Artosyn service: `openhd-artosyn.service` owns the L4/P401 daemon when an
   Artosyn-enabled package is built. The package includes the daemon and tunnel
   helper produced from https://github.com/KUTIAN-VT/L4_Linux_SDK. Output is
   discarded because the vendor daemon can log continuously; OpenHD and
   SysUtils expose the useful link state separately. The service is deliberately
   disabled at boot: SysUtils detects supported Artosyn USB/SDIO hardware and
   starts it on demand.

   USB is the default SDK transport (`ARTOSYN_INTERFACE=0`). Override it in
   `/etc/default/openhd-artosyn` when building an image for another transport:

       ARTOSYN_INTERFACE=1
       ARTOSYN_PORT=50000

   The SDK resolver stores clones, extracted archives and build products below
   `out/build/artosyn-sdk/<architecture>`. Set `OPENHD_ARTOSYN_WORK_ROOT` to use
   another persistent build directory. No runtime configuration or build step
   depends on `/tmp`.

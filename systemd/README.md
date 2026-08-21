1) OpenHD service: automatically starts (and restarts in case of a crash) the openhd main executable
2) custom unmanaged camera service: For development / custom camera scripting. By default, does nothing.
    Started by openhd if custom unmanaged camera(s) are selected via sysutils hardware settings
3) crash dump capture service: stores previous boot logs in /Config/openhd/logs for debugging

4) Artosyn service: `openhd-artosyn.service` owns the L4/P401 daemon when an
   Artosyn-enabled package is built. The package includes the daemon and tunnel
   helper produced from https://github.com/KUTIAN-VT/L4_Linux_SDK. Output is
   discarded because the vendor daemon can log continuously; OpenHD and
   SysUtils expose the useful link state separately.

   USB is the default SDK transport (`ARTOSYN_INTERFACE=0`). Override it in
   `/etc/default/openhd-artosyn` when building an image for another transport:

       ARTOSYN_INTERFACE=1
       ARTOSYN_PORT=50000

   The SDK resolver stores clones, extracted archives and build products below
   `out/build/artosyn-sdk/<architecture>`. Set `OPENHD_ARTOSYN_WORK_ROOT` to use
   another persistent build directory. No runtime configuration or build step
   depends on `/tmp`.

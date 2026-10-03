# WiFi broadcast repair and AI handoff — 2026-10-03

## Scope and status

User authorized fixes, builds, deployment and low-power testing on both units. Prioritize 20/40 MHz; 5 MHz is explicitly deferred. Work is committed locally and unpushed. A repaired armhf binary is deployed on both units. 20 MHz passed the final concurrent video/bidirectional diagnostic test. 40 MHz now transports decrypted video and bidirectional traffic, but the final simultaneous UDP test still had loss; do not describe it as fully validated or finished.

The optional UDP bridge is a diagnostic payload stream, not a requirement for OpenHD video/telemetry. The probe injects numbered 512-byte packets at localhost:5602 and reads localhost:5603, so packet delivery proves RF transport independently of Ethernet. It was disabled on both units after testing. Video/telemetry have their own wifibroadcast streams. MAVLink TCP access over Ethernet alone does not prove RF telemetry delivery.

## Workspace and hardware

Repository: C:/Users/Raphael/Documents/GitHub/drivers_/OpenHD (not the esp-sdr cwd). Parent starting HEAD b62d420f; nested Devourer dc6c493d; nested wifibroadcast 66a6fe12. Changes inside both nested repositories must be preserved/published separately before updating parent pins. Preserve unrelated untracked OpenHD-KernelBuilder/, devourer/, rtl8812au/, rtl88x2eu/ directories.

- Air 192.168.1.42: Pi4, Bullseye armv7l, kernel 6.1.29-v7l+, USB 0bda:a81a RTL8812EU/8822E Jaguar3, VEYE_GX_IMX662 1080p30. External card supply added by user.
- Ground 192.168.1.124: same OS, USB 0bda:b812 RTL8822BU Jaguar2.
- SSH account openhd; use the credentials supplied in the conversation (also present in local diagnostic scripts). No credentials should be published.
- Both reported get_throttled=0x50000 after the supply change: historical flags, no current undervoltage. Recheck rather than treating this as permanent.
- Original binary and settings backups: /root/wfb-backup-20261003/openhd and wifibroadcast_settings.json, on each unit. Never overwrite them.
- Temporary journal drop-in: /run/systemd/system/openhd.service.d/wfb-diagnostics.conf, both units. Remove it and daemon-reload/restart when diagnostics are complete.

## Root causes and implemented fixes

1. Jaguar3 firmware download could hit a 512-byte USB transfer boundary and fail reserved-page transfer. Use 972-byte chunks and wire padding at exact boundaries; DDMA uses original payload length. Throw on firmware/MAC initialization failure instead of reporting false readiness.
2. USB TX used short timeouts and treated isolated timeout as fatal, causing repeated radio resets. Minimum synchronous data timeout is 200 ms; classify partial writes as fatal. Transport escalates actual disconnect/pipe/I/O errors or persistent two-second stalls, not every isolated timeout.
3. Jaguar3 primary-20 TX subchannel mapping and Jaguar2 primary-20 RX BB mapping were reversed. Correct lower/upper mapping. This matters for session/management frames at 20 MHz while the video radio is tuned to 40 MHz; full-width standalone injection alone missed this failure.
4. Ground Devourer TX header remained 20 MHz after tuning RX to 40 MHz. Normalize ground TX width to RX width centrally, including management, scan, rollback and initialization callers; synchronize ground management/session headers.
5. Startup forcibly enabled STBC=1 and LDPC, overriding manual settings. Remove forced override and expose coding controls for Devourer. Tested plain coding succeeds; standalone STBC+LDPC 40 MHz failed with this pair. Do not claim that combination fixed. Persisted settings must explicitly set STBC=0 and LDPC=false for this tested configuration.
6. Optional UDP stream constructed a separate default 20 MHz/MCS3 header. Share the main TX header so width/MCS/coding updates apply. This fix concerns the bridge, not normal video transport.
7. Dropped-frame rate reduction was disabled. Restore it; make VEYE eligible for dynamic bitrate updates and configure RPi V4L2 encoder for CBR (video_bitrate_mode=1, verified on the target menu).
8. Source-level 5 MHz validation/rate-budget/scan plumbing added, but hardware 5/10 MHz experiments failed in both directions. These source changes are not proof of narrow-width support.

CCA-disable experiments were reverted: they worsened loss. Narrow IQK/retune/DAC experiments were also reverted. Current deployed binary uses normal carrier sense. No speculative calibration fixes remain.

## Measured evidence

At 5785 MHz, Air MCS2 / Ground MCS0, plain coding, Air power20 / Ground power30, PIT disabled, concurrent VEYE video:

| Test | Uplink received at Air | Downlink received at Ground | Ground RF video FEC |
| --- | --- | --- | --- |
| 20 MHz, 30 sec, 25 packets/sec each | 742/750 | 748/750 | total 165 to 504; lost 1 to 1 (no new lost blocks) |
| 40 MHz after live width switch, same test | 693/750 | 664/750 | total 1934 to 2315; lost 2 to 4 |

20 MHz evidence: out/wfb-probe-1791042140.json and out/wfb-telemetry-1791042147.json. 40 MHz evidence: out/wfb-probe-1791042301.json and out/wfb-telemetry-1791042299.json. Counts include startup/end buffering effects; no guarantee of zero packet loss. 40 MHz measured video around 3.8 Mbps, Air dropped frames zero but TX injection error counters increased. Ground system100 video link_index0 is RF; link_index1 is Ethernet. A successfully connected UI over Ethernet is insufficient acceptance evidence.

Earlier isolated 40 MHz raw tests delivered 1000/1000 frames in each direction. After primary-subchannel correction an earlier video-only sample delivered 11–12.5 Mbps with no new lost FEC blocks over 15 sec. These are separate tests, not a substitute for final sustained integrated validation. No rendered/decoded video proof was captured.

Armhf cross-build succeeded; target ldd -r passed both units. test_wb_link_rate_helper and TxQueueSelftest passed on both real units. git diff --check passed parent and both nested repositories. Build/ABI/tests are distinct from hardware acceptance.

## Reproduction and next work

Local ignored out/ contains Python diagnostics, binaries, logs, JSON results and build-wfb-armhf.sh. Handoff bundle described below preserves scripts and patches. Run Python tools from repository root. Python requires paramiko and pymavlink. Read each helper before use: config and settings restart both devices; standalone tests suspend OpenHD temporarily. Some standalone tools use channel6, others157; do not assume identical frequencies. Never restart while a MAVLink collector is awaiting data, and wait for deployment and radio initialization before probes.

- python out/wfb-mav.py HOST WB_CHANNEL_W [20|40]: binary MAVLink PARAM_EXT int32, component191; do not send ASCII integers. Air system101, Ground100.
- python out/wfb-camera-mav.py HOST STREAMING_E [0|1]: component100; restore 1 after isolating video.
- python out/wfb-telemetry.py 30: capture both units, decode and save RF stats.
- python out/wfb-live.py probe 30 25: bridge must be enabled, /tmp/wfb-probe.py uploaded; compare unique packet counts, not just bytes.
- python out/wfb-live.py deploy: uploads out/openhd-wfb-armhf, preflights target ABI, installs /usr/local/bin/openhd and restarts both.
- python out/wfb-run.py tests: upload/run target regression executables.

WSL Ubuntu22.04 root, GCC10 arm-linux-gnueabihf; Bullseye sysroot /opt/openhd-cross/sysroot-bullseye-armhf; build /opt/openhd-cross/build-wfb-armhf. Invoke out/build-wfb-armhf.sh; targets openhd, test_wb_link_rate_helper, TxQueueSelftest. Strip only after build completes. No source change after the latest build has been deployed.

Next priorities: capture sustained 40 MHz native video/telemetry with bridge disabled; verify cold start at40 and live 40→20→40 following on Ground. If normal video is stable but bridge load causes loss, investigate contention/USB queue scheduling and rate reservation rather than disabling CCA or increasing power blindly. Prove telemetry RF independently of Ethernet if claiming complete two-way native acceptance. Recheck coding permutations separately; plain coding is the validated configuration. Defer5 per user.

Before final completion: disable bridge, leave camera STREAMING_E=1, choose stable width20 and keep low power/PIT=false, remove temporary journal drop-ins, restart and verify RF video/service health. Keep original backups and record exact final settings. Do not claim all20/40 scenarios fixed while40 loaded loss remains.

Upstream references consulted: https://github.com/OpenIPC/devourer ; https://github.com/OpenIPC/wiki/blob/master/en/fpv.md ; https://docs.openipc.org/use-cases/fpv/wfb-ng/-quick-start/ . Kernel-driver restrictions do not establish userspace Devourer restrictions.

## Final checkpoint for continuation

Native40 cold restart with UDP disabled: out/wfb-telemetry-1791042366.json, Ground RF total28→395, lost0→0 over30 sec, incoming video ~3.5–4.1 Mbps. TX injection hints still rose Air8→38 and Ground0→6: retain as an unresolved performance signal.

Then live Air40→20 was acknowledged and Ground followed20. Removed diagnostic journal drop-ins, daemon-reloaded and restarted both; both services active. Final out/wfb-telemetry-1791042425.json includes startup: Ground RF total0→120, lost0→1, video reaches~3.6 Mbps; this startup-inclusive sample is not a zero-loss steady-state claim.

Final active radio settings: 5785 MHz, width20, AirMCS2/GroundMCS0, Airpower20/Groundpower30, STBC0/LDPCfalse/shortguardfalse, PITfalse, UDPbridgefalse, camera streaming enabled. The role-irrelevant persisted width fields remain40 (Air ground-width field; Ground air-width field); active role fields and telemetry confirm20. No current undervoltage. Source changes are committed locally; no deployment or build claims for other hardware.

Private continuation bundle: out/wfb-handoff-2026-10-03.zip contains patches for parent and both nested repositories, HEAD/status snapshots, helper scripts, logs and raw JSON evidence. Helpers contain device credentials: keep bundle private; the Markdown handoff itself contains no password. Existing out/ is ignored, so archive it explicitly before moving machines. Apply nested patches in their respective directories; do not expect a parent-only patch to contain their code. Binary remains out/openhd-wfb-armhf and deployed on both units; reproduce using the preserved build script/sysroot.


## Commit checkpoint

Devourer repair commit: e7e96e4. Wifibroadcast repair commit: a8617c5. The parent commit containing this document records both submodule references and the OpenHD integration fixes. No commits have been pushed. The private ZIP preserves the pre-commit patches/status; use Git for the current committed state.

# Updating OpenHD packages over WiFiBroadcast

This path supports the Debian packages for OpenHD, QOpenHD, SysUtils and WebUI. It uses a dedicated encrypted bidirectional stream. Air requests missing chunks, saves each complete window to disk and resumes from that checkpoint after an interruption. Initial provisioning requires an OpenHD build with OTA support on each end.

## One-time provisioning

Set the following in an OpenHD systemd drop-in on both Ground and Air and restart their OpenHD services.

```ini
[Service]
Environment=OPENHD_WB_OTA=1
```

Generate an Ed25519 signing key on the release machine and copy only its public key to `/etc/openhd/ota-trusted.pem` on Air. Keep the staging directory and trust configuration writable only by the administrator.

```sh
openssl genpkey -algorithm ED25519 -out ota-release-private.pem
openssl pkey -in ota-release-private.pem -pubout -out ota-trusted.pem
```

Enable `openhd-wb-ota.service` on Air to receive signed bundles and install them. Use the `listen` command without `--install` to stage signed bundles for review. The radio bridge pauses OTA traffic when OpenHD reports armed.

## Prepare and send a bundle

Run `openhd-wb-ota identify` on Air during provisioning and keep the returned Debian architecture and model. Pass those exact values when building its bundle. Include one or more packages for that image.

```sh
openhd-wb-ota pack update.zip openhd.deb qopenhd.deb openhd-sys-utils.deb open-hd-web-ui.deb \
  --architecture arm64 --model 'Raspberry Pi 4 Model B Rev 1.4'
openhd-wb-ota sign update.zip ota-release-private.pem update.sig
openhd-wb-ota send update.zip --signature update.sig
```

Run `send` on Ground. At its default 160 kbps pacing, transferring large bundles can take hours. Use `--rate-kbps` to tune the rate for the maintenance link. Re-run the same command after a timeout to resume. Sender completion confirms verified staging, while the Air service journal records the separate installation result.

Air validates the signed bundle digest and rejects a mismatched architecture or board model. Before installation it compares each Debian package's metadata against the signed bundle, rejects downgrades, then uses APT for an offline local install. It uses `dpkg --install` if APT is unavailable. Include only package builds for the installed distro and make sure their dependencies are installed or available in the local APT cache. Debian package scripts may restart services and an interrupted package install follows the platform's normal Debian recovery process.

For manual transfers, start `openhd-wb-ota receive PATH --sha256 DIGEST --size BYTES` on Air using a digest obtained from a trusted source, run `openhd-wb-ota send BUNDLE` on Ground, and then run `sudo openhd-wb-ota install PATH --sha256 DIGEST` on Air. Use a local staging filesystem that supports hard links, such as ext4.

# Artosyn Private SDK Build Notes

Artosyn (`ArtLink`) support is mandatory in this tree.
The SDK source remains private and is **not** stored in this repository.

## Supported injection methods

1. Set explicit paths:

```bash
export ARTOSYN_SDK_ROOT=/opt/openhd-private/artosyn_sdk
export ARTOSYN_SDK_LIB=/opt/openhd-private/artosyn_sdk/host_drv/app/ar8030/libar8030_client.a
```

2. Set only `ARTOSYN_SDK_ROOT` and let CMake/scripts auto-detect `libar8030_client`.

3. Reuse kernel-builder secret contract:
   - `DOWNLOAD_URL` (+ optional `DOWNLOAD_KEY`) for secured archive download.
   - If `DOWNLOAD_URL` is a git URL (not an archive), it is treated as ArtLink repo URL.
   - or `OPENHD_SUBMODULE_TOKEN` / `ARTLINK_GIT_AUTH` for private GitHub clone.

4. (Optional) Set `ARTOSYN_SDK_ARCHIVE` to a local `.tar/.tar.gz/.tgz` archive path.
   Builder scripts extract it and auto-resolve paths.

5. Reuse local KernelBuilder output directly by setting:
   - `OPENHD_KERNEL_BUILDER_DIR` (for `<dir>/workdir/mods/OpenHD-ArtLink`)
   - or `ARTLINK_SOURCE_DIR` (direct path to the ArtLink source tree)

If `libar8030_client` is not prebuilt in that source tree, the resolver will
attempt to build `ar8030_client` from `host_drv` automatically.

## Builder defaults

OpenHD build scripts also search these default SDK roots when env vars are not set:

- `/opt/openhd-private/artosyn_sdk`
- `/opt/openhd/artosyn_sdk`
- `/opt/artosyn_sdk`
- `/usr/local/share/openhd/artosyn_sdk`

## CI/Builder recommendation

Keep SDK in private storage (artifact store, private package, or mounted secret volume),
then inject via environment variables at build time.
For GitHub Actions, prefer reusing existing secrets already used by
`OpenHD-KernelBuilder`:

- `DOWNLOAD_URL`
- `DOWNLOAD_KEY`
- `OPENHD_SUBMODULE_TOKEN`

You can control fetch strategy with `ARTLINK_FETCH_MODE`:

- `git-only` (recommended for private GitHub repo)
- `git-first` (default auto behavior)
- `download-first`
- `download-only`

# Shared OpenHD settings

The authoring source is `catalog.json`. It supplies IDs, labels, descriptions,
enum values, UI ranges, and optional initial-provisioning metadata. OpenHD's
registered backend settings determine which entries a device actually exports.
Application-only preferences such as HUD layout remain owned by their application.

SysUtils-owned role, debug, platform information and supported display settings
are registered through a cached owner adapter. The bindings live in the same JSON
catalog; OpenHD uses internal SysUtils IPC, while clients use MAVLink. An owner
that is absent contributes no controls; a disconnected owner disables writes.
Record-only mode also runs the parameter service on TCP port 5760, without RF or
flight-controller telemetry.
Role choices come from SysUtils, including build restrictions: record-only is
currently disabled in the existing builds, and fixed Air platforms expose only Air.

## Editing definitions

Edit `catalog.json`, then run:

```sh
python3 OpenHD/settings/tools/generate.py
python3 OpenHD/settings/tools/sync_clients.py --qopenhd QOpenHD --glide openhd-glide --webui OpenHD-SysUtils/OpenHD-WebUI --imagewriter /path/to/OpenHD-ImageWriter
```

For FleetControl, pass `--fleetcontrol /path/to/OpenHD-FleetControl` to the sync
command too. Use the actual active checkout: a workspace clone may be older than
the checkout used for deployment.

Generated files are checked during the OpenHD build. The sync command has a
`--check` mode for release automation. Consumers do not need a new release to
discover ordinary new runtime settings; their bundled JSON is only a legacy
firmware fallback. ImageWriter must consume a manifest from its selected image.

Register the real implementation through `openhd::Setting`. The trailing
`metadata` callback can provide hardware-derived `choices`, `available`,
`writable`, `reason`, and constraints. Use `enforce: true` for audited shared
constraints; the setter still performs backend-specific validation. Imported
legacy descriptions/ranges remain advisory until audited, avoiding accidental
rejection of valid existing configurations.

Use `create_read_only_*`, or `Setting::read_only`, for information parameters.
Do not register controls with a no-op setter. Unsupported hardware/backend
features must either not register a setting or resolve `available: false`.
Temporarily unavailable controls resolve `writable: false` with a reason.

Numeric enum values and MAVLink IDs are compatibility contracts: never renumber
an existing choice, reuse a removed ID, or exceed the 16-byte parameter-ID limit.
Generated C++ constants are in `settings/generated/catalog.h`.


## Shared platforms

`platforms.json` defines concrete board IDs, labels, backend platform-type IDs,
camera/SBC families, aliases, and supported roles. Generated choices are shared
by ImageWriter's configuration and craft editors and FleetControl's Hangar and
update editor. There are currently 31 board choices covering all known SysUtils
platform types. With a SysUtils checkout available, generation checks complete
platform-type coverage. Legacy combined board names require a concrete selection.

ImageWriter can refresh this list from the authenticated FleetControl
`GET /api/imagewriter/platforms` endpoint, retaining its generated offline fallback.
Choosing a board does not create an image artifact for it. Actual image availability,
flashing transport and live backend capabilities remain separate checks.

## Initial files saved on a craft

In ImageWriter's FleetControl page, select a craft, configure its image options,
then use **Save configuration**. **Export JSON** saves the corresponding initial
`settings.json` locally. The snapshot uses the same C++ builder as image writing.
FleetControl stores it in that craft's `settings.imagewriter.initialConfiguration`,
alongside the editable ImageWriter selections. Its Hangar editor shows the saved
file and provides a download. Renaming a craft or editing other settings preserves
the captured initial file; saving a new configuration replaces it explicitly.

- `PUT /api/imagewriter/profiles/:id/initial-configuration` saves a version-1
  snapshot containing `settings` and `imageWriterSettings`.
- `GET /api/imagewriter/profiles/:id/initial-configuration` downloads its initial
  `settings.json`.

Both routes enforce account ownership and ImageWriter-profile permission. Plans
validate component IDs, parameter IDs, int32 values and 128-byte strings. Reusable
snapshots exclude FleetControl login credentials and enrollment tokens; current
image-writing authentication remains session-owned. Loading another craft resets
an absent plan, refreshes open initial-default controls, and preserves typed values.
Ground controls remove camera-component defaults.

FleetControl's runtime editor now downloads device metadata through MAVLink FTP
and intersects it with actual PARAM_EXT enumeration. It starts with an empty
inventory, scopes IDs by craft/system/component, sends binary int32/custom UTF-8
values, waits for ACKs and reports readback availability separately. Requests to
Air and Ground use their corresponding relay or VPN route. The legacy `hidden`
hint is a QOpenHD presentation hint, not a capability restriction.

SysUtils currently retains internal owner IPC. A future native SysUtils MAVLink
component can reuse this catalog and parameter/metadata contract to remain
configurable while OpenHD is stopped. Administrative actions need command semantics
and should be migrated separately from ordinary settings.

## MAVLink transport

Every runtime client uses MAVLink. No new OpenHD-specific message ID is required.

| Operation | Wire protocol |
|---|---|
| Discover standard metadata | `MAV_CMD_REQUEST_MESSAGE`, requesting `COMPONENT_METADATA` (397) or legacy `COMPONENT_INFORMATION` (395) |
| Read metadata | `FILE_TRANSFER_PROTOCOL`, OpenFileRO / ReadFile / CalcFileCRC32 |
| Enumerate and read values | `PARAM_EXT_REQUEST_LIST`, `PARAM_EXT_REQUEST_READ`, `PARAM_EXT_VALUE` |
| Write and obtain result | `PARAM_EXT_SET`, `PARAM_EXT_ACK` |

Each settings component exports these read-only virtual files:

- `/component_metadata/general.json`: current upstream general metadata schema.
- `/component_metadata/parameters.json`: current upstream numeric parameter schema, version 3.
- `/openhd/settings.json`: versioned OpenHD metadata, including string choices and runtime availability.
- `/openhd/file-results.json`: most recent explicit runtime-file import results, when an import occurred.
- `/openhd/initial-results.json`: initial provisioning results, when a plan was present.

The general document advertises only the upstream-compatible numeric metadata.
OpenHD clients additionally fetch the OpenHD registry at its documented virtual
path. The standard document contains no proprietary fields. String metadata is
kept separate because the current upstream parameter schema does not accept it.
The upstream proposal is in `upstream/README.md`.

The FTP service serves only in-memory metadata. It cannot read arbitrary device
files, write files, delete files, or reboot a system. Sessions are scoped to the
requesting system/component, bounded, and expire after 30 seconds. Open files
are immutable snapshots even when hardware capabilities change during transfer.
Clients bound transfers to 1 MiB and publish only complete metadata.

QOpenHD fetches metadata with each parameter refresh. Glide refreshes after
discovery and every 30 seconds. WebUI refreshes for each explicit refresh and
after applying a setting. Backend capability validation also runs for writes
equal to a cached value. Invalid or absent extended parameters receive a rejection.

Runtime values are scoped by system/component/parameter, never by ID alone.
Air is system 101, Ground is 100; general settings use component 191, camera 1
uses 100, camera 2 uses 101. QOpenHD, Glide, and WebUI use distinct source
component IDs to avoid FTP-session collisions. Air/Ground coordinated radio
changes remain explicit operations rather than automatic value mirroring.

## Editing runtime values with JSON

Create `openhd/runtime-settings.json` on the configuration partition
(`/Config/openhd/runtime-settings.json` on current images; legacy
`/boot/openhd/runtime-settings.json` is also supported):

```json
{
  "version": 1,
  "systemId": 101,
  "components": {
    "100": {"RESOLUTION_FPS": "1920x1080@30"}
  }
}
```

The provider checks for changes once per second, validates every entry, and calls
the same backend setter used by MAVLink. Unregistered, unsupported, read-only,
out-of-range, and wrong-type requests are rejected. Results are available through
MAVLink FTP. Imports are per entry, not atomic transactions; other accepted entries
can apply when one is rejected. Replace the file atomically when editing it.

This is an explicit runtime override: unchanged content is applied once per
provider lifetime, and remaining content can apply on subsequent starts. Remove
the file to stop requesting those overrides. Definitions stay separate from
values; editing labels or adding an ID never creates a backend implementation.

`OPENHD_RUNTIME_SETTINGS_FILE` can select another local file for development.

## ImageWriter and initial defaults

ImageWriter keeps its existing initial `settings.json`, boot files, and selection
workflow. Its camera-name-to-ID lookup now reads the generated map rather than
maintaining another C++ enum table.

ImageBuilder copies the packaged generated map to
`openhd/settings-manifest.json` on the boot partition. When inspecting a mounted
image, ImageWriter prefers this manifest over its bundled legacy map. The mounted-device editor enables generic initial controls from a supported image
manifest. The image-preparation editor can stage a plan from its bundled catalog;
the target firmware validates it against its registered owners on first boot.
Older firmware may ignore this extension, so provisioning results must be checked.

To make a setting provisionable, add a `provision` object to its definition:

```json
{"componentId": 100, "roles": ["air"]}
```

The generated `initialFields` are rendered automatically. Initial selections are
stored inside the existing `settings.json`:

```json
{
  "role": "air",
  "parameter_defaults": {
    "version": 1,
    "components": {"100": {"AIR_RECORDING_E": 2}}
  }
}
```

The backend owner opts into initial seeding by setting `Setting::seed_allowed`
from `PersistentSettings::created_defaults()`. Camera and wifibroadcast settings implement this
binding. Existing persistent settings are never overwritten by this plan.
Owners of additional provisionable settings must add the same ownership binding;
merely advertising a field does not authorize overwriting persistent data.

Initial values still pass through runtime capability and backend validation.
Offline manifests describe planned configuration, not detected hardware. Check
the initial-results file for rejected or skipped entries. Do not use a runtime
override file to implement initial defaults.

## Client integration and verification

- QOpenHD's `DocumentedParam` API now loads generated JSON and per-component
  device metadata. Existing QML controls retain their handlers. The camera sidebar
  prefers server-provided choices; absent metadata entries are removed after a
  successful refresh.
- Glide's controller downloads metadata and sends setting/value/result lines to
  the LVGL UI. Camera, radio, and recording controls use that metadata. The System
  page provides a generic editor; View's video renderer is unchanged.
- WebUI's legacy descriptions and camera choices are also generated from the
  shared source. Its Connected OpenHD settings editor communicates through the existing
  MAVLink TCP endpoint, default `127.0.0.1:5760`. Deployment can set
  `OPENHD_MAVLINK_HOST` and `OPENHD_MAVLINK_PORT`. A write is followed by a refresh;
  accepted does not claim a delayed hardware operation has completed.
  The settings page replaces its duplicate camera, record, SysUtils and raw
  persistent-file editors with the device registry. The Hardware page routes
  hotspot configuration to this same editor. Network and system
  administration APIs remain separate; old API routes are retained for compatibility.
- ImageWriter's initial configuration remains offline. Application-only controls
  and system administration actions are outside the shared parameter registry.

Tests: `test_metadata_ftp`, `test_settings_registry`, `test_parameter_reapply`.
`test_settings_registry_tcp` is an optional localhost integration fixture for
clients. `test_settings_server` exercises the actual parameter-only TCP service
with WebUI's `tests/SettingsMavlinkIntegration` client and normal shutdown.
Builds and these tests do not prove camera/radio operation on every
supported board. Validate advertised choices on actual hardware when migrating
capability providers.

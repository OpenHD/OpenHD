# MAVLink upstream proposal: extended parameter metadata

Status: draft for review; no upstream PR or new message ID has been submitted.
The `baseline/` schemas were downloaded from MAVLink master on 2026-10-12.
`parameter-string-metadata.patch` and `parameter-v4.schema.json` are generated
from that baseline with `tools/draft_upstream.py`; schema fixtures verify both
the current numeric format and the proposed string extension.

OpenHD exposes integer and string parameters through the existing extended
parameter protocol. Numeric metadata uses the current upstream parameter JSON
schema unchanged. The separate OpenHD registry carries information that the
current schema cannot represent, especially string choices.

The proposed first upstream change is a generic schema extension:

1. Add `String` to the parameter `type` enumeration.
2. Permit a string `default` and string enum `values[].value` when type is String.
3. Add `maxLength` in bytes, bounded by the extended protocol's 128-byte field.
4. Require the parameter type to agree with default and enum-value types.
5. Version this expanded document as version 4. Continue serving a numeric-only
   version 3 document to existing clients until implementations negotiate/support it.

String is a textual interpretation of `MAV_PARAM_EXT_TYPE_CUSTOM`; this proposal
does not redefine arbitrary binary custom data as text. The examples use UTF-8,
with no embedded NUL and explicit byte-length validation. Existing numeric wire
encoding, parameter IDs, set/read messages, and ACK semantics stay unchanged.

Dynamic availability needs a separate protocol discussion. The existing extended
parameter enumeration cannot safely change its count/index mapping mid-transfer.
OpenHD keeps its legacy parameter list stable and exports resolved availability
in fresh metadata; every write is validated against current capabilities. New
clients intersect observed parameters with resolved metadata. A generic future
standard should define refresh/cache invalidation and immutable enumeration
generations before adding messages or assigning IDs.

The implementation supports standard COMPONENT_METADATA (397) and legacy
COMPONENT_INFORMATION (395) discovery, and read-only MAVLink FTP delivery. The
current bundled OpenHD dialect predates COMPONENT_METADATA, so its published
standard layout is packed locally; updating the dialect later can replace that
compatibility code with the generated pack helper.

An upstream submission should include schema changes, numeric compatibility
fixtures, string enum/default/length fixtures, reference implementations, and
agreement on cache refresh semantics. OpenHD-specific boot files, platform IDs,
UI layout hints, and application provisioning do not belong in the common dialect.

References:

- https://mavlink.io/en/services/component_metadata.html
- https://github.com/mavlink/mavlink/blob/master/component_metadata/parameter.schema.json
- https://mavlink.io/en/services/parameter_ext.html
- https://mavlink.io/en/services/ftp.html

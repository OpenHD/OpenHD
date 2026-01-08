#!/usr/bin/env python3
import argparse
import json
from pathlib import Path


def escape_cpp_string(value: str) -> str:
    out = []
    for ch in value:
        if ch == "\\":
            out.append("\\\\")
        elif ch == '"':
            out.append('\\"')
        elif ch == "\n":
            out.append("\\n")
        elif ch == "\r":
            out.append("\\r")
        elif ch == "\t":
            out.append("\\t")
        else:
            out.append(ch)
    return "".join(out)


def load_registry(path: Path):
    data = json.loads(path.read_text(encoding="utf-8"))
    camera_types = data.get("camera_types")
    if not isinstance(camera_types, list):
        raise ValueError("camera_types must be a list")

    manufacturer_groups = data.get("manufacturer_groups")
    if not isinstance(manufacturer_groups, list):
        raise ValueError("manufacturer_groups must be a list")

    platform_choices = data.get("platform_choices")
    if not isinstance(platform_choices, list):
        raise ValueError("platform_choices must be a list")

    secondary_choices = data.get("secondary_choices", [])
    if not isinstance(secondary_choices, list):
        raise ValueError("secondary_choices must be a list")

    fallback_choices = data.get("fallback_choices", [])
    if not isinstance(fallback_choices, list):
        raise ValueError("fallback_choices must be a list")

    return (
        camera_types,
        manufacturer_groups,
        platform_choices,
        secondary_choices,
        fallback_choices,
    )


def validate_registry(
    camera_types,
    manufacturer_groups,
    platform_choices,
    secondary_choices,
    fallback_choices,
):
    camera_by_key = {}
    ids = set()
    for entry in camera_types:
        if not isinstance(entry, dict):
            raise ValueError("camera_types entry must be an object")
        key = entry.get("key")
        cid = entry.get("id")
        name = entry.get("name")
        if not key or cid is None or name is None:
            raise ValueError("camera_types entry requires key, id, and name")
        resolutions = entry.get("resolutions")
        if resolutions is not None:
            if not isinstance(resolutions, list):
                raise ValueError(f"resolutions for {key} must be a list")
            for resolution in resolutions:
                if not isinstance(resolution, dict):
                    raise ValueError(f"resolution entry for {key} must be an object")
                width = resolution.get("width")
                height = resolution.get("height")
                fps = resolution.get("fps")
                if width is None or height is None or fps is None:
                    raise ValueError(f"resolution entry for {key} requires width, height, fps")
                if not isinstance(width, int) or not isinstance(height, int) or not isinstance(fps, int):
                    raise ValueError(f"resolution entry for {key} must use integer width/height/fps")
        if key in camera_by_key:
            raise ValueError(f"duplicate camera key: {key}")
        if cid in ids:
            raise ValueError(f"duplicate camera id: {cid}")
        camera_by_key[key] = entry
        ids.add(cid)

    manufacturer_by_key = {}
    for entry in manufacturer_groups:
        if not isinstance(entry, dict):
            raise ValueError("manufacturer_groups entry must be an object")
        key = entry.get("key")
        name = entry.get("name")
        cameras = entry.get("cameras")
        if not key or not name or not isinstance(cameras, list):
            raise ValueError("manufacturer_groups entry requires key, name, and cameras list")
        if key in manufacturer_by_key:
            raise ValueError(f"duplicate manufacturer group key: {key}")
        for cam_key in cameras:
            cam = camera_by_key.get(cam_key)
            if cam is None:
                raise ValueError(f"unknown camera key in manufacturer group {key}: {cam_key}")
            if cam.get("ui_label") is None:
                raise ValueError(
                    f"camera key {cam_key} in manufacturer group {key} requires ui_label"
                )
        manufacturer_by_key[key] = entry

    for entry in platform_choices:
        if not isinstance(entry, dict):
            raise ValueError("platform_choices entry must be an object")
        platform = entry.get("platform")
        manufacturers = entry.get("manufacturers")
        if not platform or not isinstance(manufacturers, list):
            raise ValueError("platform_choices entry requires platform and manufacturers list")
        for group_key in manufacturers:
            if group_key not in manufacturer_by_key:
                raise ValueError(
                    f"unknown manufacturer group {group_key} for platform {platform}"
                )

    for group_key in secondary_choices:
        if group_key not in manufacturer_by_key:
            raise ValueError(f"unknown manufacturer group in secondary_choices: {group_key}")

    for group_key in fallback_choices:
        if group_key not in manufacturer_by_key:
            raise ValueError(f"unknown manufacturer group in fallback_choices: {group_key}")

    return camera_by_key, manufacturer_by_key


def render_header(
    camera_types,
    manufacturer_groups,
    platform_choices,
    secondary_choices,
    fallback_choices,
    camera_by_key,
):
    lines = []
    lines.append("// Generated by tools/gen_camera_registry.py. Do not edit by hand.")
    lines.append("#pragma once")
    lines.append("")
    lines.append("#include <cstddef>")
    lines.append("#include <string>")
    lines.append("")
    lines.append("#include \"openhd_platform.h\"")
    lines.append("")
    lines.append("namespace openhd {")
    lines.append("namespace camera_registry {")
    lines.append("")
    for entry in camera_types:
        key = entry["key"]
        cid = entry["id"]
        lines.append(f"static constexpr int {key} = {cid};")
    lines.append("")
    lines.append("struct CameraTypeNameEntry {")
    lines.append("  int id;")
    lines.append("  const char* name;")
    lines.append("};")
    lines.append("")
    lines.append("inline constexpr CameraTypeNameEntry kCameraTypeNameEntries[] = {")
    for entry in camera_types:
        name = escape_cpp_string(entry["name"])
        cid = entry["id"]
        lines.append(f'  {{{cid}, "{name}"}},')
    lines.append("};")
    lines.append("")
    lines.append("inline std::string camera_type_to_string(int camera_type) {")
    lines.append("  for (const auto& entry : kCameraTypeNameEntries) {")
    lines.append("    if (entry.id == camera_type) {")
    lines.append("      return entry.name;")
    lines.append("    }")
    lines.append("  }")
    lines.append('  return "UNKNOWN (" + std::to_string(camera_type) + ")";')
    lines.append("}")
    lines.append("")
    lines.append("struct CameraUiEntry {")
    lines.append("  int type;")
    lines.append("  const char* label;")
    lines.append("};")
    lines.append("")
    lines.append("struct ResolutionEntry {")
    lines.append("  int width_px;")
    lines.append("  int height_px;")
    lines.append("  int fps;")
    lines.append("};")
    lines.append("")
    lines.append("struct CameraResolutionEntry {")
    lines.append("  int type;")
    lines.append("  const ResolutionEntry* resolutions;")
    lines.append("  std::size_t resolution_count;")
    lines.append("};")
    lines.append("")
    lines.append("struct ManufacturerEntry {")
    lines.append("  const char* name;")
    lines.append("  const CameraUiEntry* cameras;")
    lines.append("  std::size_t camera_count;")
    lines.append("};")
    lines.append("")
    lines.append("struct PlatformCameraChoicesEntry {")
    lines.append("  int platform_type;")
    lines.append("  const ManufacturerEntry* manufacturers;")
    lines.append("  std::size_t manufacturer_count;")
    lines.append("};")
    lines.append("")

    for entry in camera_types:
        resolutions = entry.get("resolutions")
        if not resolutions:
            continue
        key = entry["key"]
        lines.append(f"inline constexpr ResolutionEntry kCameraResolutions_{key}[] = {{")
        for resolution in resolutions:
            width = resolution["width"]
            height = resolution["height"]
            fps = resolution["fps"]
            lines.append(f"  {{{width}, {height}, {fps}}},")
        lines.append("};")
        lines.append("")

    lines.append("inline constexpr CameraResolutionEntry kCameraResolutionEntries[] = {")
    for entry in camera_types:
        resolutions = entry.get("resolutions")
        if not resolutions:
            continue
        key = entry["key"]
        lines.append(
            f"  {{{key}, kCameraResolutions_{key}, "
            f"sizeof(kCameraResolutions_{key}) / sizeof(kCameraResolutions_{key}[0])}},"
        )
    lines.append("};")
    lines.append("")
    lines.append("inline const CameraResolutionEntry* find_camera_resolutions(int camera_type) {")
    lines.append("  for (const auto& entry : kCameraResolutionEntries) {")
    lines.append("    if (entry.type == camera_type) {")
    lines.append("      return &entry;")
    lines.append("    }")
    lines.append("  }")
    lines.append("  return nullptr;")
    lines.append("}")
    lines.append("")

    for entry in manufacturer_groups:
        group_key = entry["key"]
        group_symbol = escape_cpp_string(group_key).upper()
        cameras = entry["cameras"]
        lines.append(f"inline constexpr CameraUiEntry kManufacturer_{group_symbol}_Cameras[] = {{")
        for cam_key in cameras:
            cam = camera_by_key[cam_key]
            label = escape_cpp_string(cam["ui_label"])
            lines.append(f"  {{{cam_key}, \"{label}\"}},")
        lines.append("};")
        lines.append("")
        group_name = escape_cpp_string(entry["name"])
        lines.append(
            f"inline constexpr ManufacturerEntry kManufacturer_{group_symbol} = "
            f"{{\"{group_name}\", kManufacturer_{group_symbol}_Cameras, "
            f"sizeof(kManufacturer_{group_symbol}_Cameras) / sizeof(kManufacturer_{group_symbol}_Cameras[0])}};"
        )
        lines.append("")

    for entry in platform_choices:
        platform = entry["platform"]
        symbol = escape_cpp_string(platform).upper()
        manufacturers = entry["manufacturers"]
        lines.append(f"inline constexpr ManufacturerEntry kPlatform_{symbol}_Manufacturers[] = {{")
        for group_key in manufacturers:
            group_symbol = escape_cpp_string(group_key).upper()
            lines.append(f"  kManufacturer_{group_symbol},")
        lines.append("};")
        lines.append("")

    lines.append("inline constexpr PlatformCameraChoicesEntry kPlatformCameraChoices[] = {")
    for entry in platform_choices:
        platform = entry["platform"]
        symbol = escape_cpp_string(platform).upper()
        lines.append(
            f"  {{{platform}, kPlatform_{symbol}_Manufacturers, "
            f"sizeof(kPlatform_{symbol}_Manufacturers) / sizeof(kPlatform_{symbol}_Manufacturers[0])}},"
        )
    lines.append("};")
    lines.append("")

    if secondary_choices:
        lines.append("inline constexpr ManufacturerEntry kSecondaryManufacturers[] = {")
        for group_key in secondary_choices:
            group_symbol = escape_cpp_string(group_key).upper()
            lines.append(f"  kManufacturer_{group_symbol},")
        lines.append("};")
        lines.append("")
        lines.append(
            "inline constexpr std::size_t kSecondaryManufacturerCount = "
            "sizeof(kSecondaryManufacturers) / sizeof(kSecondaryManufacturers[0]);"
        )
        lines.append("")
    else:
        lines.append("inline constexpr ManufacturerEntry kSecondaryManufacturers[] = {};")
        lines.append("inline constexpr std::size_t kSecondaryManufacturerCount = 0;")
        lines.append("")

    if fallback_choices:
        lines.append("inline constexpr ManufacturerEntry kFallbackManufacturers[] = {")
        for group_key in fallback_choices:
            group_symbol = escape_cpp_string(group_key).upper()
            lines.append(f"  kManufacturer_{group_symbol},")
        lines.append("};")
        lines.append("")
        lines.append(
            "inline constexpr std::size_t kFallbackManufacturerCount = "
            "sizeof(kFallbackManufacturers) / sizeof(kFallbackManufacturers[0]);"
        )
        lines.append("")
    else:
        lines.append("inline constexpr ManufacturerEntry kFallbackManufacturers[] = {};")
        lines.append("inline constexpr std::size_t kFallbackManufacturerCount = 0;")
        lines.append("")

    lines.append("inline const PlatformCameraChoicesEntry* find_platform_camera_choices(int platform_type) {")
    lines.append("  for (const auto& entry : kPlatformCameraChoices) {")
    lines.append("    if (entry.platform_type == platform_type) {")
    lines.append("      return &entry;")
    lines.append("    }")
    lines.append("  }")
    lines.append("  return nullptr;")
    lines.append("}")
    lines.append("")
    lines.append("}  // namespace camera_registry")
    lines.append("}  // namespace openhd")
    lines.append("")
    for entry in camera_types:
        key = entry["key"]
        lines.append(f"using openhd::camera_registry::{key};")
    lines.append("")
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()

    (
        camera_types,
        manufacturer_groups,
        platform_choices,
        secondary_choices,
        fallback_choices,
    ) = load_registry(Path(args.input))
    camera_by_key, manufacturer_by_key = validate_registry(
        camera_types,
        manufacturer_groups,
        platform_choices,
        secondary_choices,
        fallback_choices,
    )
    header = render_header(
        camera_types,
        manufacturer_groups,
        platform_choices,
        secondary_choices,
        fallback_choices,
        camera_by_key,
    )
    with open(args.output, "w", encoding="utf-8", newline="\n") as output_file:
        output_file.write(header)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

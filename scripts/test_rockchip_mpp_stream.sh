#!/usr/bin/env bash
set -euo pipefail
repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
for tool in "${CXX:-g++}" python3 ffmpeg ffprobe; do command -v "$tool" >/dev/null; done
"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pthread \
  "$repo_root/OpenHD/ohd_video/test/test_rockchip_stream_utils.cpp" -o "$work/test_stream_utils"
"$work/test_stream_utils"
"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror \
  -I"$repo_root/OpenHD/ohd_video/inc" \
  "$repo_root/OpenHD/ohd_video/test/test_matroska_recorder.cpp" \
  "$repo_root/OpenHD/ohd_video/src/matroska_recorder.cpp" -o "$work/test_matroska"
for codec in h264 h265; do
  if [[ "$codec" == h264 ]]; then
    encoder=libx264
    params=(-x264-params aud=1:bframes=0)
  else
    encoder=libx265
    params=(-x265-params aud=1:pools=1:bframes=0:log-level=error)
  fi
  ffmpeg -hide_banner -loglevel error -f lavfi -i testsrc2=size=128x96:rate=10 \
    -frames:v 5 -c:v "$encoder" "${params[@]}" "$work/input.$codec"
  "$work/test_matroska" "$work/input.$codec" "$work/timestamps-$codec.mkv" "$codec" 128 96 10 timestamped
  ffprobe -v error -select_streams v -show_entries frame=best_effort_timestamp_time \
    -of csv=p=0 "$work/timestamps-$codec.mkv" > "$work/pts-$codec.txt"
  ffmpeg -v error -xerror -i "$work/timestamps-$codec.mkv" -f null -
  "$work/test_matroska" "$work/input.$codec" "$work/baseline-$codec.mkv" "$codec" 128 96 10
  ffmpeg -v error -xerror -i "$work/baseline-$codec.mkv" -f null -
done
python3 - "$work" <<'PY'
import pathlib, sys
root = pathlib.Path(sys.argv[1])
for codec in ('h264', 'h265'):
    pts = []
    for line in (root / ('pts-' + codec + '.txt')).read_text().splitlines():
        try:
            pts.append(float(line.split(',')[0]))
        except ValueError:
            pass
    assert len(pts) == 5, pts
    assert all(abs(x-y) < .001 for x, y in zip(pts, [0, .1, .7, .8, .9])), pts
    print(codec + ' decode and preserved timestamp gap passed: ' + str(pts))
PY

# Bundled H.264 lab sample

Generated locally from FFmpeg's synthetic `testsrc2` pattern, with no camera
footage or third-party media. 320x180, 30 access units, one second at 30 fps.

Generation command:

```sh
ffmpeg -f lavfi -i testsrc2=size=320x180:rate=30 -t 1 \
  -c:v libx264 -preset ultrafast -tune zerolatency -b:v 800k \
  -x264-params keyint=30:aud=1:repeat-headers=1 -f h264 sample.h264
```

Access-unit delimiters allow deterministic looping. The tool records the
SHA-256 of the exact embedded asset, not a claim of bit-identical FFmpeg
output across encoder versions.

# NXP V4L2 runtime controls

This directory contains the NXP i.MX8 V4L2 pipeline helpers. Runtime control
support here is **NXP-specific** because it relies on the encoder being exposed
as a V4L2 device (`/dev/video0`) that accepts the NXP control set.

* Supported pipeline: NXP i.MX8 V4L2 encoder only.
* Runtime bitrate and IDR: bitrate overrides use `VIDIOC_S_CTRL` on the active
  encoder FD and force-IDR requests use either the vendor IDR control or the
  generic `V4L2_CID_MPEG_VIDEO_FORCE_KEY_FRAME` fallback.
* No pipeline restart is needed for these controls; the encoder FD is tracked
  while streaming so IOCTL updates apply in-place.

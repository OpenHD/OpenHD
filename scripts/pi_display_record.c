// FKMS/DispmanX investigation prototype. See docs/pi-ground-display-recording.md.
// Build on the Pi; not intended for full KMS or Pi 5.
#define _POSIX_C_SOURCE 200809L
#include <bcm_host.h>
#include <gst/app/gstappsrc.h>
#include <gst/gst.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static volatile sig_atomic_t stopping;
static gint encoded;
static void stop(int sig) { (void)sig; stopping = 1; }
static guint64 now_ns(void) {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return (guint64)t.tv_sec * GST_SECOND + t.tv_nsec;
}
static GstPadProbeReturn count_frame(GstPad *p, GstPadProbeInfo *i, gpointer u) {
  (void)p; (void)i; (void)u;
  g_atomic_int_inc(&encoded);
  return GST_PAD_PROBE_OK;
}
static int report_error(GstMessage *m) {
  GError *e = NULL; gchar *debug = NULL;
  gst_message_parse_error(m, &e, &debug);
  fprintf(stderr, "%s: %s\n%s\n", GST_OBJECT_NAME(m->src), e->message,
          debug ? debug : "");
  g_error_free(e); g_free(debug);
  return 1;
}
int main(int argc, char **argv) {
  if (argc != 6) {
    fprintf(stderr, "Usage: %s OUTPUT.mkv WIDTH HEIGHT FPS SECONDS (0=until Ctrl-C)\n", argv[0]);
    return 2;
  }
  int w = atoi(argv[2]), h = atoi(argv[3]), fps = atoi(argv[4]), seconds = atoi(argv[5]);
  if (w < 32 || w > 1920 || w % 32 || h < 32 || h > 1080 || h % 2 ||
      fps < 1 || fps > 60 || seconds < 0) return 2;
  signal(SIGINT, stop); signal(SIGTERM, stop);
  const int rgb565 = getenv("OHD_CAPTURE_RGB565") != NULL;
  const int bpp = rgb565 ? 2 : 3;
  gst_init(NULL, NULL); bcm_host_init();
  DISPMANX_DISPLAY_HANDLE_T display = vc_dispmanx_display_open(0);
  DISPMANX_MODEINFO_T info;
  if (!display || vc_dispmanx_display_get_info(display, &info)) return 1;
  uint32_t native;
  DISPMANX_RESOURCE_HANDLE_T resource = vc_dispmanx_resource_create(
    rgb565 ? VC_IMAGE_RGB565 : VC_IMAGE_RGB888, w, h, &native);
  if (!resource) { vc_dispmanx_display_close(display); return 1; }
  VC_RECT_T rect; vc_dispmanx_rect_set(&rect, 0, 0, w, h);
  GError *error = NULL;
  GstElement *pipeline = gst_parse_launch(
    "appsrc name=screen is-live=true format=time block=true "
    "! queue max-size-buffers=2 max-size-bytes=0 max-size-time=0 leaky=downstream "
    "! v4l2h264enc extra-controls=\"controls,video_bitrate_mode=0,video_bitrate=10000000,repeat_sequence_header=1\" "
    "! video/x-h264,profile=high,level=(string)4.2 "
    "! h264parse name=parsed ! matroskamux ! filesink name=output sync=false", &error);
  if (error) {
    fprintf(stderr, "%s\n", error->message); g_error_free(error);
    if (pipeline) gst_object_unref(pipeline);
    vc_dispmanx_resource_delete(resource); vc_dispmanx_display_close(display); return 1;
  }
  GstElement *src = gst_bin_get_by_name(GST_BIN(pipeline), "screen");
  GstElement *out = gst_bin_get_by_name(GST_BIN(pipeline), "output");
  GstElement *parsed = gst_bin_get_by_name(GST_BIN(pipeline), "parsed");
  GstPad *pad = gst_element_get_static_pad(parsed, "src");
  gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, count_frame, NULL, NULL);
  gst_object_unref(pad); gst_object_unref(parsed);
  g_object_set(out, "location", argv[1], NULL); gst_object_unref(out);
  const gsize bytes = (gsize)w * h * bpp;
  g_object_set(src, "max-bytes", (guint64)bytes * 2, NULL);
  GstCaps *caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, rgb565 ? "RGB16" : "RGB",
    "width", G_TYPE_INT, w, "height", G_TYPE_INT, h, "framerate", GST_TYPE_FRACTION, fps, 1,
    "colorimetry", G_TYPE_STRING, "sRGB", "interlace-mode", G_TYPE_STRING, "progressive",
    "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1, NULL);
  gst_app_src_set_caps(GST_APP_SRC(src), caps); gst_caps_unref(caps);
  GstBufferPool *pool = gst_buffer_pool_new();
  GstStructure *pool_config = gst_buffer_pool_get_config(pool);
  gst_buffer_pool_config_set_params(pool_config, NULL, bytes, 4, 8);
  if (!gst_buffer_pool_set_config(pool, pool_config) ||
      !gst_buffer_pool_set_active(pool, TRUE)) {
    gst_object_unref(pool); gst_object_unref(src); gst_object_unref(pipeline);
    vc_dispmanx_resource_delete(resource); vc_dispmanx_display_close(display); return 1;
  }
  GstBus *bus = gst_element_get_bus(pipeline);
  int failed = gst_element_set_state(pipeline, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE;
  guint64 start = now_ns(), tick = GST_SECOND / fps, next = start, captures = 0, skipped = 0;
  fprintf(stderr, "Display %ux%u -> %dx%d, target %d fps; hardware H264 -> %s\n",
          info.width, info.height, w, h, fps, argv[1]);
  while (!failed && !stopping && (!seconds || now_ns() - start < (guint64)seconds * GST_SECOND)) {
    GstMessage *m = gst_bus_pop_filtered(bus, GST_MESSAGE_ERROR);
    if (m) { failed = report_error(m); gst_message_unref(m); break; }
    if (gst_app_src_get_current_level_bytes(GST_APP_SRC(src)) < bytes) {
      GstBuffer *b = NULL;
      GstBufferPoolAcquireParams acquire = {0};
      acquire.flags = GST_BUFFER_POOL_ACQUIRE_FLAG_DONTWAIT;
      if (gst_buffer_pool_acquire_buffer(pool, &b, &acquire) != GST_FLOW_OK) {
        skipped++; goto pace;
      }
      GstMapInfo map;
      if (!b || !gst_buffer_map(b, &map, GST_MAP_WRITE)) { if (b) gst_buffer_unref(b); failed = 1; break; }
      guint64 timestamp = now_ns() - start;
      int rc = vc_dispmanx_snapshot(display, resource, 0);
      if (!rc) rc = vc_dispmanx_resource_read_data(resource, &rect, map.data, w * bpp);
      gst_buffer_unmap(b, &map);
      if (rc) { gst_buffer_unref(b); failed = 1; break; }
      GST_BUFFER_PTS(b) = timestamp; GST_BUFFER_DURATION(b) = tick;
      if (gst_app_src_push_buffer(GST_APP_SRC(src), b) != GST_FLOW_OK) { failed = 1; break; }
      captures++;
    } else { skipped++; }
pace:
    next += tick;
    guint64 current = now_ns();
    if (current > next && current - next >= tick) {
      guint64 missed = (current - next) / tick;
      skipped += missed; next += missed * tick;
    }
    struct timespec deadline = {(time_t)(next / GST_SECOND), (long)(next % GST_SECOND)};
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &deadline, NULL);
  }
  double elapsed = (now_ns() - start) / (double)GST_SECOND;
  if (!failed) {
    gst_app_src_end_of_stream(GST_APP_SRC(src));
    GstMessage *m = gst_bus_timed_pop_filtered(bus, 15 * GST_SECOND, GST_MESSAGE_EOS | GST_MESSAGE_ERROR);
    if (!m) { fprintf(stderr, "Timed out finalizing recording\n"); failed = 1; }
    else { if (GST_MESSAGE_TYPE(m) == GST_MESSAGE_ERROR) failed = report_error(m); gst_message_unref(m); }
  }
  fprintf(stderr, "wall=%.3fs captures=%llu encoded=%d skipped_ticks=%llu encoded_fps=%.2f\n",
    elapsed, (unsigned long long)captures, g_atomic_int_get(&encoded),
    (unsigned long long)skipped, g_atomic_int_get(&encoded) / elapsed);
  gst_element_set_state(pipeline, GST_STATE_NULL);
  gst_object_unref(bus); gst_object_unref(src); gst_object_unref(pipeline);
  gst_buffer_pool_set_active(pool, FALSE); gst_object_unref(pool);
  vc_dispmanx_resource_delete(resource); vc_dispmanx_display_close(display);
  return failed;
}

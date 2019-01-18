#include <stdbool.h>
#include <EGL/egl.h>
#include "eglstate.h"
#include <bcm_host.h>
#include <assert.h>

// setWindowParams sets the window's position, adjusting if need be to
// prevent it from going fully off screen. Also sets the dispman rects
// for displaying.
static void setWindowParams(STATE_T * state, int32_t x, int32_t y, VC_RECT_T * src_rect, VC_RECT_T * dst_rect) {
	uint32_t dx, dy, w, h, sx, sy;

	// Set source & destination rectangles so that the image is
	// clipped if it goes off screen (else dispman won't show it properly)
	if (x < (1 - (int)state->window_width)) {	   // Too far off left
		x = 1 - (int)state->window_width;
		dx = 0;
		sx = state->window_width - 1;
		w = 1;
	} else if (x < 0) {				   // Part of left is off
		dx = 0;
		sx = -x;
		w = state->window_width - sx;
	} else if (x < (int)(state->screen_width - state->window_width)) {	// On
		dx = x;
		sx = 0;
		w = state->window_width;
	} else if (x < (int)state->screen_width) {	   // Part of right is off
		dx = x;
		sx = 0;
		w = state->screen_width - x;
	} else {					   // Too far off right
		x = state->screen_width - 1;
		dx = state->screen_width - 1;
		sx = 0;
		w = 1;
	}

	if (y < (1 - (int)state->window_height)) {	   // Too far off top
		y = 1 - (int)state->window_height;
		dy = 0;
		sy = state->window_height - 1;
		h = 1;
	} else if (y < 0) {				   // Part of top is off
		dy = 0;
		sy = -y;
		h = state->window_height - sy;
	} else if (y < (int)(state->screen_height - state->window_height)) {	// On
		dy = y;
		sy = 0;
		h = state->window_height;
	} else if (y < (int)state->screen_height) {	   // Part of bottom is off
		dy = y;
		sy = 0;
		h = state->screen_height - y;
	} else {					   // Wholly off bottom
		y = state->screen_height - 1;
		dy = state->screen_height - 1;
		sy = 0;
		h = 1;
	}

	state->window_x = x;
	state->window_y = y;

	vc_dispmanx_rect_set(dst_rect, dx, dy, w, h);
	vc_dispmanx_rect_set(src_rect, sx << 16, sy << 16, w << 16, h << 16);
}

// oglinit sets the display, OpenVGL context and screen information
// state holds the display information
void oglinit(STATE_T * state) {
	int32_t success = 0;
	EGLBoolean result;
	EGLint num_config;

	static EGL_DISPMANX_WINDOW_T nativewindow;

	DISPMANX_ELEMENT_HANDLE_T dispman_element;
	DISPMANX_DISPLAY_HANDLE_T dispman_display;
	DISPMANX_UPDATE_HANDLE_T dispman_update;
	VC_RECT_T dst_rect;
	VC_RECT_T src_rect;
	static VC_DISPMANX_ALPHA_T alpha = {
//              DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS,
		DISPMANX_FLAGS_ALPHA_FROM_SOURCE,
		255, 0
	};

	static const EGLint attribute_list[] = {
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_NONE
	};

	EGLConfig config;

	// get an EGL display connection
	state->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	assert(state->display != EGL_NO_DISPLAY);

	// initialize the EGL display connection
	result = eglInitialize(state->display, NULL, NULL);
	assert(EGL_FALSE != result);

	// bind OpenVG API
	eglBindAPI(EGL_OPENVG_API);

	// get an appropriate EGL frame buffer configuration
	result = eglChooseConfig(state->display, attribute_list, &config, 1, &num_config);
	assert(EGL_FALSE != result);

	// create an EGL rendering context
	state->context = eglCreateContext(state->display, config, EGL_NO_CONTEXT, NULL);
	assert(state->context != EGL_NO_CONTEXT);

	// create an EGL window surface
	success = graphics_get_display_size(0 /* LCD */ , &state->screen_width,
					    &state->screen_height);
	assert(success >= 0);

	if ((state->window_width == 0) || (state->window_width > state->screen_width))
		state->window_width = state->screen_width;
	if ((state->window_height == 0) || (state->window_height > state->screen_height))
		state->window_height = state->screen_height;

	// adjust position so that at least one pixel is on screen and
	// set up the dispman rects
	setWindowParams(state, state->window_x, state->window_y, &src_rect, &dst_rect);

	dispman_display = vc_dispmanx_display_open(0 /* LCD */ );
	state->dmx_display = dispman_display;
	dispman_update = vc_dispmanx_update_start(0);

	dispman_element = vc_dispmanx_element_add(dispman_update, dispman_display, 1 /*layer */ , &dst_rect, 0 /*src */ ,
						  &src_rect, DISPMANX_PROTECTION_NONE, &alpha, 0 /*clamp */ ,
						  0 /*transform */ );

	state->element = dispman_element;
	nativewindow.element = dispman_element;
	nativewindow.width = state->window_width;
	nativewindow.height = state->window_height;
	vc_dispmanx_update_submit_sync(dispman_update);

	state->surface = eglCreateWindowSurface(state->display, config, &nativewindow, NULL);
	assert(state->surface != EGL_NO_SURFACE);

	// preserve the buffers on swap
	result = eglSurfaceAttrib(state->display, state->surface, EGL_SWAP_BEHAVIOR, EGL_BUFFER_PRESERVED);
	assert(EGL_FALSE != result);

	// connect the context to the surface
	result = eglMakeCurrent(state->display, state->surface, state->surface, state->context);
	assert(EGL_FALSE != result);
}

// dispmanMoveWindow repositions the openVG window to given coords
// -ve coords are allowed upto (1-width,1-height),
// max (screen_width-1,screen_height-1). i.e. at least one pixel must be
// on the screen.
void dispmanMoveWindow(STATE_T * state, int32_t x, int32_t y) {
	VC_RECT_T src_rect, dst_rect;
	DISPMANX_UPDATE_HANDLE_T dispman_update;

	setWindowParams(state, x, y, &src_rect, &dst_rect);
	dispman_update = vc_dispmanx_update_start(0);
	vc_dispmanx_element_change_attributes(dispman_update, state->element, 0, 0, 0, &dst_rect, &src_rect, 0, DISPMANX_NO_ROTATE);
	vc_dispmanx_update_submit_sync(dispman_update);
}

// dispmanChangeWindowOpacity changes the window's opacity
// 0 = transparent, 255 = opaque
void dispmanChangeWindowOpacity(STATE_T * state, uint32_t alpha) {
	DISPMANX_UPDATE_HANDLE_T dispman_update;

	if (alpha > 255)
		alpha = 255;

	dispman_update = vc_dispmanx_update_start(0);
	// The 1<<1 below means update the alpha value
	vc_dispmanx_element_change_attributes(dispman_update, state->element, 1 << 1, 0, (uint8_t) alpha, 0, 0, 0,
					      DISPMANX_NO_ROTATE);
	vc_dispmanx_update_submit_sync(dispman_update);
}

// Custom cursor pointer code.

static inline int align_up(int x, int y) {
	return (x + y - 1) & ~(y - 1);
}

// Create a cursor
cursor_t *createCursor(STATE_T * state, const uint32_t * data, uint32_t w, uint32_t h, uint32_t hx, uint32_t hy, bool upsidedown) {
	if (w == 0 || w > state->screen_width || h == 0 || h > state->screen_height || hx >= w || hy >= h)
		return NULL;
	int32_t pitch = align_up(w * 4, 64);
	cursor_t *cursor = calloc(1, sizeof *cursor);
	if (cursor == NULL)
		return NULL;

	// Set the size of cursor and position it at top-left for now.
	cursor->state.window_width = w;
	cursor->state.window_height = h;
	cursor->state.screen_width = state->window_width;
	cursor->state.screen_height = state->window_height;
	cursor->hot_x = (int32_t) hx;
	cursor->hot_y = (int32_t) hy;
	// Grab a copy of the dispman display
	cursor->display = state->dmx_display;

	// Copy image data
	char *image = calloc(1, pitch * h);
	if (image == NULL) {
		free(cursor);
		return NULL;
	}
	uint32_t img_p;
	cursor->resource = vc_dispmanx_resource_create(VC_IMAGE_RGBA32, w, h, &img_p);
	if (cursor->resource == 0) {
		free(image);
		free(cursor);
		return NULL;
	}

	uint32_t row;
	int32_t incr = (int32_t) w;
	if (upsidedown) {
		data += w * (h - 1);
		incr = -incr;
	}
	for (row = 0; row < h; row++) {
		memcpy(image + (row * pitch), data, w * 4);
		data += incr;
	}

	VC_RECT_T dst_rect = {.width = w, h };
	vc_dispmanx_resource_write_data(cursor->resource, VC_IMAGE_RGBA32, pitch, image, &dst_rect);
	free(image);
	cursor->state.element = 0;
	return cursor;
}

// Show the cursor on screen
void showCursor(cursor_t * cursor) {
	if (cursor && !cursor->state.element) {
		VC_RECT_T src_rect, dst_rect;
		DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
		setWindowParams(&cursor->state, cursor->state.window_x, cursor->state.window_y, &src_rect, &dst_rect);
		cursor->state.element = vc_dispmanx_element_add(update, cursor->display,
								1, &dst_rect,
								cursor->resource,
								&src_rect, DISPMANX_PROTECTION_NONE, NULL, NULL, VC_IMAGE_ROT0);
		vc_dispmanx_update_submit_sync(update);
	}
}

// Hide the cursor
void hideCursor(cursor_t * cursor) {
	if (cursor && cursor->state.element) {
		DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
		vc_dispmanx_element_remove(update, cursor->state.element);
		vc_dispmanx_update_submit_sync(update);
		cursor->state.element = 0;
	}
}

// Move the cursor
void moveCursor(STATE_T * state, cursor_t * cursor, int32_t x, int32_t y) {
	if (cursor) {
		if (x < 0)
			x = 0;
		if (x >= (int32_t) state->window_width)
			x = (int32_t) state->window_width - 1;
		if (y < 0)
			y = 0;
		if (y >= (int32_t) state->window_height)
			y = (int32_t) state->window_height - 1;
		VC_RECT_T src_rect, dst_rect;
		setWindowParams(&cursor->state, x - cursor->hot_x, y - cursor->hot_y, &src_rect, &dst_rect);
		dst_rect.x += state->window_x;
		dst_rect.y += state->window_y;
		if (cursor->state.element) {
			DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
			vc_dispmanx_element_change_attributes(update,
							      cursor->state.element,
							      0, 0, 0, &dst_rect, &src_rect, 0, DISPMANX_NO_ROTATE);
			vc_dispmanx_update_submit_sync(update);
		}
	}
}

// Delete a cursor
void deleteCursor(cursor_t * cursor) {
	if (cursor) {
		if (cursor->state.element) {
			DISPMANX_UPDATE_HANDLE_T update;
			update = vc_dispmanx_update_start(0);
			vc_dispmanx_element_remove(update, cursor->state.element);
			vc_dispmanx_update_submit_sync(update);
		}
		vc_dispmanx_resource_delete(cursor->resource);
		free(cursor);
	}
}

// Dim screen
// level goes from 0 = black -> 255 = Normal. Over 255 is clamped.
void screenBrightness(STATE_T * state, uint32_t level) {
	static uint32_t brightnessLevel = 255;
	static DISPMANX_RESOURCE_HANDLE_T brightnessLayer = 0;
	static DISPMANX_ELEMENT_HANDLE_T brightnessElement = 0;
	int ret;

	if (level > 255)
		level = 255;
	if (brightnessLevel == level)
		return;

	VC_RECT_T src_rect, dst_rect;
	if (!brightnessLayer) {
		uint32_t img_p;
		brightnessLayer = vc_dispmanx_resource_create(VC_IMAGE_RGBA32, 1, 1, &img_p);
		if (!brightnessLayer)
			return;
		uint32_t image = 0;
		dst_rect = (VC_RECT_T) {
		.width = 1, 1};
		ret = vc_dispmanx_resource_write_data(brightnessLayer, VC_IMAGE_RGBA32, sizeof image, &image, &dst_rect);
		if (!ret) {
			vc_dispmanx_resource_delete(brightnessLayer);
			brightnessLayer = 0;
			return;
		}
	}

	brightnessLevel = level;

	DISPMANX_UPDATE_HANDLE_T update;
	if (level == 255) {
		update = vc_dispmanx_update_start(0);
		vc_dispmanx_element_remove(update, brightnessElement);
		vc_dispmanx_update_submit_sync(update);
		vc_dispmanx_resource_delete(brightnessLayer);
		brightnessLayer = 0;
		brightnessElement = 0;
	} else {
		if (brightnessElement) {
			update = vc_dispmanx_update_start(0);
			vc_dispmanx_element_change_attributes(update, brightnessElement, 1 << 1, 0, (uint8_t) (255 - level), 0, 0,
							      0, DISPMANX_NO_ROTATE);
			vc_dispmanx_update_submit_sync(update);
		} else {
			static VC_DISPMANX_ALPHA_T alpha = {
				DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS,
				255, 0
			};
			alpha.opacity = 255 - level;

			update = vc_dispmanx_update_start(0);
			src_rect = (VC_RECT_T) {
			.width = 1 << 16, 1 << 16};
			dst_rect = (VC_RECT_T) {
			.width = state->screen_width, state->screen_height};
			brightnessElement = vc_dispmanx_element_add(update,
								    state->dmx_display,
								    255, &dst_rect,
								    brightnessLayer,
								    &src_rect,
								    DISPMANX_PROTECTION_NONE, &alpha, NULL, VC_IMAGE_ROT0);
			vc_dispmanx_update_submit_sync(update);
		}
	}
}

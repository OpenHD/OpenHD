//
// libshapes: high-level OpenVG API
// Anthony Starks (ajstarks@gmail.com)
//
// Additional outline / windowing functions
// Paeryn (github.com/paeryn)
//
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <wchar.h>
#include <termios.h>
#include <assert.h>
#include <jpeglib.h>
#include <png.h>
#include "VG/openvg.h"
#include "VG/vgu.h"
#include "EGL/egl.h"
#include "bcm_host.h"
#include "DejaVuSans.inc"				   // font data
#include "DejaVuSansMono.inc"
#include "eglstate.h"					   // data structures for graphics state
#include "fontinfo.h"					   // font data structure
#include "shapes.h"					   // Needed to check prototypes
#include "fontsystem.h"

static STATE_T _state, *state = &_state;	// global graphics state
static int32_t init_x = 0;	// Initial window position and size
static int32_t init_y = 0;
static uint32_t init_w = 0;
static uint32_t init_h = 0;
static bool check_errors = true;
static uint32_t vg_error_code = VG_NO_ERROR;

// hold the current glyph representation of a string to print. Memory
// is allocated by the stringToGlyph function, will grow when a longer
// string is converted.
// glyph_kern holds the kerning tables of the string (2*float per glyph)
static VGuint *glyph_string = NULL;
static VGfloat *glyph_kern = NULL;

// hold the paths for basic shapes. Shapes that need a variable number
// of segments are handled by common_path.
static VGPath common_path = VG_INVALID_HANDLE;
static VGPath cbezier_path = VG_INVALID_HANDLE;
static VGPath qbezier_path = VG_INVALID_HANDLE;
static VGPath rect_path = VG_INVALID_HANDLE;
static VGPath line_path = VG_INVALID_HANDLE;
static VGPath roundrect_path = VG_INVALID_HANDLE;
static VGPath ellipse_path = VG_INVALID_HANDLE;
static VGPath dot_smooth_path = VG_INVALID_HANDLE;
static VGPath dot_rough_path = VG_INVALID_HANDLE;
static VGPaint fill_paint = VG_INVALID_HANDLE;
static VGPaint stroke_paint = VG_INVALID_HANDLE;

// Pointer cursor
static cursor_t *priv_cursor = NULL;

//
// Terminal settings
//

// terminal settings structures
static struct termios new_term_attr;
static struct termios orig_term_attr;

// saveterm saves the current terminal settings
void SaveTerm() {
	tcgetattr(fileno(stdin), &orig_term_attr);
}

// Deprecated
void saveterm() {
	SaveTerm();
}

// rawterm sets the terminal to raw mode
void RawTerm() {
	memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
	new_term_attr.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);
	new_term_attr.c_cc[VTIME] = 0;
	new_term_attr.c_cc[VMIN] = 0;
	tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);
}

// Deprecated
void rawterm() {
	RawTerm();
}

// restore resets the terminal to the previously saved setting
void RestoreTerm() {
	tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);
}

// Deprecated
void restoreterm() {
	RestoreTerm();
}

//
// Font functions
//

// loadfont loads font path data
// derived from http://web.archive.org/web/20070808195131/http://developer.hybrid.fi/font2openvg/renderFont.cpp.txt
Fontinfo LoadFont(const int *Points,
		  const int *PointIndices,
		  const unsigned char *Instructions,
		  const int *InstructionIndices, const int *InstructionCounts,
		  const int *adv, const short *cmap, int ng, int descender, int ascender) {
	Fontinfo f = malloc(sizeof *f);
	if (!f)
		return NULL;
	f->face = NULL;
	VGFont font = f->vgfont = vgCreateFont(ng);
	if (font == VG_INVALID_HANDLE) {
		free(f);
		return NULL;
	}

	VGint i;
	for (i = 0; i < ng; i++) {
		const int *p = &Points[PointIndices[i] * 2];
		const unsigned char *instructions = &Instructions[InstructionIndices[i]];
		VGfloat origin[2] = { 0.0f, 0.0f };
		VGfloat escapement[2] = { (VGfloat) (adv[i]) / 65536.0f, 0.0f };
		VGPath path = VG_INVALID_HANDLE;
		int ic = InstructionCounts[i];
		if (ic) {
			int p_count;
			if (i < ng - 1)
				p_count = PointIndices[i + 1];
			else
				p_count = sizeof Points / sizeof *Points;
			p_count -= PointIndices[i];

			path = vgCreatePath(VG_PATH_FORMAT_STANDARD,
					    VG_PATH_DATATYPE_S_32,
					    1.0f / 65536.0f, 0.0f, ic, p_count, VG_PATH_CAPABILITY_APPEND_TO);
			if (path != VG_INVALID_HANDLE)
				vgAppendPathData(path, ic, instructions, p);
		}
		vgSetGlyphToPath(font, (VGuint) i, path, VG_FALSE, origin, escapement);
		if (path != VG_INVALID_HANDLE)
			vgDestroyPath(path);
	}

	f->CharacterMap = cmap;
	f->GlyphAdvances = adv;
	f->Count = (unsigned int)ng;
	f->DescenderHeight = (VGfloat) descender / 65536.0f;
	f->AscenderHeight = (VGfloat) ascender / 65536.0f;
	f->Height = f->AscenderHeight - f->DescenderHeight;	// Guesstimate
	f->Name = "unknown";
	f->Style = "unknown";
	f->Kerning = 0;
	return f;
}

// Deprecated
Fontinfo loadfont(const int *Points,
		  const int *PointIndices,
		  const unsigned char *Instructions,
		  const int *InstructionIndices, const int *InstructionCounts,
		  const int *adv, const short *cmap, int ng, int descender, int ascender) {
	return LoadFont(Points, PointIndices, Instructions, InstructionIndices,
			InstructionCounts, adv, cmap, ng, descender, ascender);
}

// unloadfont frees font path data
void UnloadFont(Fontinfo f) {
	if (f) {
		if (f->face)
			return UnloadTTF(f);
		else {
			vgDestroyFont(f->vgfont);
			free(f);
		}

	}
}

// Deprecated
void unloadfont(Fontinfo f) {
	UnloadFont(f);
}

// createImageFromJpeg decompresses a JPEG image to the standard image format
// source: https://github.com/ileben/ShivaVG/blob/master/examples/test_image.c
VGImage CreateImageFromJpeg(const char *filename) {
	FILE *infile;
	struct jpeg_decompress_struct jdc;
	struct jpeg_error_mgr jerr;
	JSAMPARRAY buffer;
	unsigned int bstride;
	unsigned int bbpp;

	VGImage img;
	VGubyte *data;
	unsigned int width;
	unsigned int height;
	unsigned int dstride;
	unsigned int dbpp;

	VGubyte *brow;
	VGubyte *drow;
	unsigned int x;
	unsigned int lilEndianTest = 1;
	VGImageFormat rgbaFormat;

	// Check for endianness
	if (((unsigned char *)&lilEndianTest)[0] == 1)
		rgbaFormat = VG_sABGR_8888;
	else
		rgbaFormat = VG_sRGBA_8888;

	// Try to open image file
	infile = fopen(filename, "rb");
	if (infile == NULL) {
		fprintf(stderr, "Failed opening '%s' for reading!\n", filename);
		return VG_INVALID_HANDLE;
	}
	// Setup default error handling
	jdc.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&jdc);

	// Set input file
	jpeg_stdio_src(&jdc, infile);

	// Read header and start
	jpeg_read_header(&jdc, TRUE);
	jpeg_start_decompress(&jdc);
	width = jdc.output_width;
	height = jdc.output_height;

	// Allocate buffer using jpeg allocator
	bbpp = (unsigned int)jdc.output_components;
	bstride = width * bbpp;
	buffer = (*jdc.mem->alloc_sarray)
	    ((j_common_ptr) & jdc, JPOOL_IMAGE, bstride, 1);

	// Allocate image data buffer
	dbpp = 4;
	dstride = width * dbpp;
	data = (VGubyte *) malloc(dstride * height);

	// Iterate until all scanlines processed
	while (jdc.output_scanline < height) {

		// Read scanline into buffer
		jpeg_read_scanlines(&jdc, buffer, 1);
		drow = data + (height - jdc.output_scanline) * dstride;
		brow = buffer[0];
		// Expand to RGBA
		for (x = 0; x < width; ++x, drow += dbpp, brow += bbpp) {
			switch (bbpp) {
			case 4:
				drow[0] = brow[0];
				drow[1] = brow[1];
				drow[2] = brow[2];
				drow[3] = brow[3];
				break;
			case 3:
				drow[0] = brow[0];
				drow[1] = brow[1];
				drow[2] = brow[2];
				drow[3] = 255;
				break;
			}
		}
	}

	// Create VG image
	img = vgCreateImage(rgbaFormat, (VGint) width, (VGint) height, VG_IMAGE_QUALITY_BETTER);
	if (img != VG_INVALID_HANDLE)
		vgImageSubData(img, data, (VGint) dstride, rgbaFormat, 0, 0, (VGint) width, (VGint) height);
	else
		vg_error_code = vgGetError();

	// Cleanup
	jpeg_destroy_decompress(&jdc);
	fclose(infile);
	free(data);

	return img;
}

// Deprecated
VGImage createImageFromJpeg(const char *filename) {
	return CreateImageFromJpeg(filename);
}

// makeimage makes an image from a raw raster of red, green, blue, alpha values
void MakeImage(VGfloat x, VGfloat y, VGint w, VGint h, VGubyte * data) {
	VGint dstride = w * 4;
	VGImageFormat rgbaFormat = VG_sABGR_8888;
	VGImage img = vgCreateImage(rgbaFormat, w, h, VG_IMAGE_QUALITY_BETTER);
	if (img != VG_INVALID_HANDLE) {
		vgImageSubData(img, (void *)data, dstride, rgbaFormat, 0, 0, w, h);
		vgSetPixels((VGint) x, (VGint) y, img, 0, 0, w, h);
		vgDestroyImage(img);
	} else
		vg_error_code = vgGetError();
}

// Deprecated
void makeimage(VGfloat x, VGfloat y, VGint w, VGint h, VGubyte * data) {
	MakeImage(x, y, w, h, data);
}

// Image places an image at the specifed location
void Image(VGfloat x, VGfloat y, VGint w, VGint h, const char *filename) {
	VGImage img = CreateImageFromJpeg(filename);
	if (img != VG_INVALID_HANDLE) {
		vgSetPixels((VGint) x, (VGint) y, img, 0, 0, w, h);
		vgDestroyImage(img);
	} else
		vg_error_code = vgGetError();
}

// Deprecated
void image(VGfloat x, VGfloat y, VGint w, VGint h, const char *filename) {
	Image(x, y, w, h, filename);
}

// dumpscreen writes the raster
static void dumpscreen(VGuint w, VGuint h, FILE * fp) {
	if (w > state->window_width)
		w = state->window_width;
	if (h > state->window_height)
		h = state->window_height;
	void *ScreenBuffer = malloc(w * h * 4);
	if (ScreenBuffer) {
		vgReadPixels(ScreenBuffer, (VGint) (w * 4), VG_sABGR_8888, 0, 0, (VGint) w, (VGint) h);
		fwrite(ScreenBuffer, 1, w * h * 4, fp);
		free(ScreenBuffer);
	}
}

//Fontinfo SansTypeface, SerifTypeface, MonoTypeface;
Fontinfo SansTypeface, MonoTypeface;

// initWindowSize requests a specific window size & position, if not called
// then init() will open a full screen window.
// Done this way to preserve the original init() behaviour.
void InitWindowSize(int32_t x, int32_t y, int32_t w, int32_t h) {
	init_x = x;
	init_y = y;
	init_w = w < 0 ? 0 : (uint32_t) w;
	init_h = h < 0 ? 0 : (uint32_t) h;
}

// Deprecated
void initWindowSize(int32_t x, int32_t y, int32_t w, int32_t h) {
	InitWindowSize(x, y, w, h);
}

// init sets the system to its initial state, returns false if an
// error occured.
bool InitShapes(int32_t * w, int32_t * h) {
	int err_state = 0;
	check_errors = false;
	bcm_host_init();
	memset(state, 0, sizeof(*state));
	state->window_x = init_x;
	state->window_y = init_y;
	state->window_width = init_w;
	state->window_height = init_h;
	oglinit(state);
	SansTypeface = LoadFont(DejaVuSans_glyphPoints,
				DejaVuSans_glyphPointIndices,
				DejaVuSans_glyphInstructions,
				DejaVuSans_glyphInstructionIndices,
				DejaVuSans_glyphInstructionCounts,
				DejaVuSans_glyphAdvances,
				DejaVuSans_characterMap,
				DejaVuSans_glyphCount, DejaVuSans_descender_height, DejaVuSans_ascender_height);
	if (SansTypeface) {
		SansTypeface->Name = "DejaVu Sans";
		SansTypeface->Style = "Book";
	}
	MonoTypeface = LoadFont(DejaVuSansMono_glyphPoints,
				DejaVuSansMono_glyphPointIndices,
				DejaVuSansMono_glyphInstructions,
				DejaVuSansMono_glyphInstructionIndices,
				DejaVuSansMono_glyphInstructionCounts,
				DejaVuSansMono_glyphAdvances,
				DejaVuSansMono_characterMap,
				DejaVuSansMono_glyphCount, DejaVuSansMono_descender_height, DejaVuSansMono_ascender_height);
	if (MonoTypeface) {
		MonoTypeface->Name = "DejaVu Sans Mono";
		MonoTypeface->Style = "Book";
	}
	*w = (int32_t) state->window_width;
	*h = (int32_t) state->window_height;

	if (!SansTypeface || !MonoTypeface) {
		fputs("libshapes failed to load the default fonts.", stderr);
	}

	fill_paint = vgCreatePaint();
	err_state |= fill_paint == VG_INVALID_HANDLE;

	stroke_paint = vgCreatePaint();
	err_state |= stroke_paint == VG_INVALID_HANDLE;

	common_path =
	    vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, 0, 0,
			 VG_PATH_CAPABILITY_APPEND_TO | VG_PATH_CAPABILITY_MODIFY);
	err_state |= common_path == VG_INVALID_HANDLE;

	VGubyte segments[] = { VG_MOVE_TO_ABS, VG_CUBIC_TO };
	VGfloat coords[] = { 0.0f, 0.0f, 1.0f, -1.0f, 2.0f, 1.0f, 3.0f, 0.0f };

	cbezier_path =
	    vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, 2, 8,
			 VG_PATH_CAPABILITY_APPEND_TO | VG_PATH_CAPABILITY_MODIFY);
	if (cbezier_path == VG_INVALID_HANDLE)
		err_state |= 1;
	else
		vgAppendPathData(cbezier_path, 2, segments, coords);

	qbezier_path =
	    vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, 2, 6,
			 VG_PATH_CAPABILITY_APPEND_TO | VG_PATH_CAPABILITY_MODIFY);
	if (qbezier_path == VG_INVALID_HANDLE)
		err_state |= 1;
	else {
		segments[1] = VG_QUAD_TO;
		vgAppendPathData(qbezier_path, 2, segments, coords);
	}

	rect_path =
	    vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, 5, 5,
			 VG_PATH_CAPABILITY_APPEND_TO | VG_PATH_CAPABILITY_MODIFY);
	if (rect_path == VG_INVALID_HANDLE)
		err_state |= 1;
	else
		vguRect(rect_path, 0.0f, 0.0f, 1.0f, 1.0f);

	line_path =
	    vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, 2, 4,
			 VG_PATH_CAPABILITY_APPEND_TO | VG_PATH_CAPABILITY_MODIFY);
	if (line_path == VG_INVALID_HANDLE)
		err_state |= 1;
	else
		vguLine(line_path, 0.0f, 0.0f, 1.0f, 1.0f);

	roundrect_path =
	    vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, 10, 26,
			 VG_PATH_CAPABILITY_APPEND_TO | VG_PATH_CAPABILITY_MODIFY);
	if (roundrect_path == VG_INVALID_HANDLE)
		err_state |= 1;
	else
		vguRoundRect(roundrect_path, 0.0f, 0.0f, 2.0f, 2.0f, 1.0f, 1.0f);

	ellipse_path =
	    vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, 4, 12,
			 VG_PATH_CAPABILITY_APPEND_TO | VG_PATH_CAPABILITY_MODIFY);
	if (ellipse_path == VG_INVALID_HANDLE)
		err_state |= 1;
	else
		vguEllipse(ellipse_path, 0.0f, 0.0f, 1.0f, 1.0f);

	dot_smooth_path =
	    vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, 4, 12, VG_PATH_CAPABILITY_APPEND_TO);
	if (dot_smooth_path == VG_INVALID_HANDLE)
		err_state |= 1;
	else
		vguEllipse(dot_rough_path, 0.5f, 0.5f, 1.0f, 1.0f);

	dot_rough_path = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, 4, 12, VG_PATH_CAPABILITY_APPEND_TO);
	if (dot_rough_path == VG_INVALID_HANDLE)
		err_state |= 1;
	else
		vguRect(dot_smooth_path, 0.0f, 0.0f, 1.0f, 1.0f);

	if (err_state) {
		fputs("Failed initialising libshapes paths.\n", stderr);
		FinishShapes();
		return false;
	}

	vg_error_code = vgGetError();
	if (vg_error_code != VG_NO_ERROR) {
		fprintf(stderr, "OpenVG gave error %x whilst initilising libshapes.\n", vg_error_code);
		FinishShapes();
		return false;
	}
	return true;
}

// Deprecated
bool init(int32_t * w, int32_t * h) {
	return InitShapes(w, h);
}

// Turn on/off checking and reporting of OpenVG errors
// For functions that already know an OpenVG error happened they will
// always set vg_error_code regardless of this setting.
void EnableOpenVGErrorCheck(bool check) {
	check_errors = check;
}

// Check status of OpenVG errors, if none had been reported then check
uint32_t CheckErrorStatus() {
	uint32_t err = vg_error_code;
	// If no current error then check if one has happened
	if (err == VG_NO_ERROR)
		err = vgGetError();
	// Reset the state as the error has now been reported
	vg_error_code = VG_NO_ERROR;
	return err;
}

// finish cleans up
void FinishShapes() {
	ScreenBrightness(255);
	DeleteCursor();
	eglSwapBuffers(state->display, state->surface);
	vgDestroyPath(dot_rough_path);
	vgDestroyPath(dot_smooth_path);
	vgDestroyPath(ellipse_path);
	vgDestroyPath(roundrect_path);
	vgDestroyPath(line_path);
	vgDestroyPath(rect_path);
	vgDestroyPath(qbezier_path);
	vgDestroyPath(cbezier_path);
	vgDestroyPaint(stroke_paint);
	vgDestroyPaint(fill_paint);
	vgDestroyPath(common_path);
	UnloadFont(SansTypeface);
	SansTypeface = NULL;
//      UnloadFont(SerifTypeface);
//      SerifTypeface = NULL;
	UnloadFont(MonoTypeface);
	MonoTypeface = NULL;
	font_CloseFontSystem();
	eglMakeCurrent(state->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroySurface(state->display, state->surface);
	eglDestroyContext(state->display, state->context);
	eglTerminate(state->display);
}

// Deprecated
void finish() {
	FinishShapes();
}

//
// Transformations
//

// Translate the coordinate system to x,y
void Translate(VGfloat x, VGfloat y) {
	vgTranslate(x, y);
}

// Rotate around angle r
void Rotate(VGfloat r) {
	vgRotate(r);
}

// Shear shears the x coordinate by x degrees, the y coordinate by y degrees
void Shear(VGfloat x, VGfloat y) {
	vgShear(x, y);
}

// Scale scales by  x, y
void Scale(VGfloat x, VGfloat y) {
	vgScale(x, y);
}

//
// Style functions
//

// setfill sets the fill color
void SetFill(VGfloat color[4]) {
	vgSetParameteri(fill_paint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
	vgSetParameterfv(fill_paint, VG_PAINT_COLOR, 4, color);
	vgSetPaint(fill_paint, VG_FILL_PATH);
}

// Deprecated
void setfill(VGfloat color[4]) {
	SetFill(color);
}

// setstroke sets the stroke color
void SetStroke(VGfloat color[4]) {
	vgSetParameteri(stroke_paint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
	vgSetParameterfv(stroke_paint, VG_PAINT_COLOR, 4, color);
	vgSetPaint(stroke_paint, VG_STROKE_PATH);
}

// Deprecated
void setstroke(VGfloat color[4]) {
	SetStroke(color);
}

// StrokeWidth sets the stroke width
void StrokeWidth(VGfloat width) {
	vgSetf(VG_STROKE_LINE_WIDTH, width);
	vgSeti(VG_STROKE_CAP_STYLE, VG_CAP_BUTT);
	vgSeti(VG_STROKE_JOIN_STYLE, VG_JOIN_MITER);
}

//
// Color functions
//
//

// RGBA fills a color vectors from a RGBA quad.
void RGBA(VGuint r, VGuint g, VGuint b, VGfloat a, VGfloat color[4]) {
	if (r > 255) {
		r = 0;
	}
	if (g > 255) {
		g = 0;
	}
	if (b > 255) {
		b = 0;
	}
	if (a < 0.0f || a > 1.0f) {
		a = 1.0f;
	}
	color[0] = (VGfloat) r / 255.0f;
	color[1] = (VGfloat) g / 255.0f;
	color[2] = (VGfloat) b / 255.0f;
	color[3] = a;
}

// RGB returns a solid color from a RGB triple
void RGB(VGuint r, VGuint g, VGuint b, VGfloat color[4]) {
	RGBA(r, g, b, 1.0f, color);
}

// Stroke sets the stroke color, defined as a RGB triple.
void Stroke(VGuint r, VGuint g, VGuint b, VGfloat a) {
	VGfloat color[4];
	RGBA(r, g, b, a, color);
	SetStroke(color);
}

// Fill sets the fillcolor, defined as a RGBA quad.
void Fill(VGuint r, VGuint g, VGuint b, VGfloat a) {
	VGfloat color[4];
	RGBA(r, g, b, a, color);
	SetFill(color);
}

// setstops sets color stops for gradients
static void setstop(VGPaint paint, VGfloat * stops, VGint n) {
	VGboolean multmode = VG_FALSE;
	VGColorRampSpreadMode spreadmode = VG_COLOR_RAMP_SPREAD_REPEAT;
	vgSetParameteri(paint, VG_PAINT_COLOR_RAMP_SPREAD_MODE, spreadmode);
	vgSetParameteri(paint, VG_PAINT_COLOR_RAMP_PREMULTIPLIED, multmode);
	vgSetParameterfv(paint, VG_PAINT_COLOR_RAMP_STOPS, 5 * n, stops);
	vgSetPaint(paint, VG_FILL_PATH);
}

// LinearGradient fills with a linear gradient
void FillLinearGradient(VGfloat x1, VGfloat y1, VGfloat x2, VGfloat y2, VGfloat * stops, VGint ns) {
	VGfloat lgcoord[4] = { x1, y1, x2, y2 };
	vgSetParameteri(fill_paint, VG_PAINT_TYPE, VG_PAINT_TYPE_LINEAR_GRADIENT);
	vgSetParameterfv(fill_paint, VG_PAINT_LINEAR_GRADIENT, 4, lgcoord);
	setstop(fill_paint, stops, ns);
}

// RadialGradient fills with a linear gradient
void FillRadialGradient(VGfloat cx, VGfloat cy, VGfloat fx, VGfloat fy, VGfloat radius, VGfloat * stops, VGint ns) {
	VGfloat radialcoord[5] = { cx, cy, fx, fy, radius };
	vgSetParameteri(fill_paint, VG_PAINT_TYPE, VG_PAINT_TYPE_RADIAL_GRADIENT);
	vgSetParameterfv(fill_paint, VG_PAINT_RADIAL_GRADIENT, 5, radialcoord);
	setstop(fill_paint, stops, ns);
}

// ClipRect limits the drawing area to specified rectangle
void ClipRect(VGint x, VGint y, VGint w, VGint h) {
	vgSeti(VG_SCISSORING, VG_TRUE);
	VGint coords[4] = { x, y, w, h };
	vgSetiv(VG_SCISSOR_RECTS, 4, coords);
}

// ClipEnd stops limiting drawing area to specified rectangle
void ClipEnd() {
	vgSeti(VG_SCISSORING, VG_FALSE);
}

// Text Functions

// stringToGlyphs converts a string into a list of glyphs
//   It auto allocates memory to store the glyph list and reuses it
//   for future convertions, growing it if need be.
//   Cache: If we are passed glyph_string as the string then it's
//   because TextMid & TextEnd have already called here when
//   calculating the width and we don't want to parse it again.
static unsigned int stringToGlyphs(const char *s, Fontinfo f) {
	static unsigned int glyph_length = 0;	// number of valid glyphs in string
	static unsigned int glyph_string_len = 0;	// size of allocated buffer
	if (s == (char *)glyph_string) {
		return glyph_length;
	}

	mbstate_t mbstate;
	memset(&mbstate, 0, sizeof mbstate);
	size_t str_length = mbsrtowcs(NULL, &s, 0, &mbstate);
	if ((str_length == 0) || (str_length == (size_t) - 1))
		return 0;

	if (str_length > glyph_string_len) {
		if (glyph_string)
			free(glyph_string);
		if (glyph_kern)
			free(glyph_kern);

		glyph_string_len = str_length;
		glyph_string = malloc(glyph_string_len * sizeof *glyph_string);
		glyph_kern = malloc(glyph_string_len * 2 * sizeof *glyph_kern);
	}
	wchar_t wstr[str_length];
	mbsrtowcs(wstr, &s, str_length, &mbstate);
	glyph_length = 0;
	unsigned int i;
	if (f->CharacterMap) {				   // libshapes classic fonts
		for (i = 0; i < str_length; i++) {
			if (wstr[i] < f->Count) {
				short glyph = f->CharacterMap[wstr[i]];
				if (glyph != -1)
					glyph_string[glyph_length++] = (VGuint) glyph;
			}
		}
	} else {					   // fontsystem fonts
		VGuint prev = 0xffffffff;
		VGfloat *kernX = glyph_kern;
		VGfloat *kernY = glyph_kern + str_length;
		for (i = 0; i < str_length; i++) {
			VGuint glyph = font_CharToGlyph(f->face, wstr[i]);
			glyph_string[i] = glyph;
			if (f->Kerning) {
				font_KernData(f->face, glyph, prev, kernX++, kernY++);
				prev = glyph;
			}
		}
		glyph_length = str_length;
	}
	return glyph_length;
}

// Text renders a string of text at a specified location, size, using the specified font glyphs
void Text(VGfloat x, VGfloat y, const char *s, Fontinfo f, VGint pointsize) {
	VGfloat size = (VGfloat) pointsize;
	VGfloat matrix[9];
	vgGetMatrix(matrix);
	VGint mm = vgGeti(VG_MATRIX_MODE);
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_GLYPH_USER_TO_SURFACE);
	vgLoadMatrix(matrix);
	vgTranslate(x, y);
	vgScale(size, size);
	vgSeti(VG_MATRIX_MODE, mm);
	VGfloat pos[2] = { 0.0f, 0.0f };
	vgSetfv(VG_GLYPH_ORIGIN, 2, pos);
	VGfloat strokew = vgGetf(VG_STROKE_LINE_WIDTH);
	if (strokew != 0.0f) {
		vgSetf(VG_STROKE_LINE_WIDTH, strokew / size);
	}
	unsigned int count = stringToGlyphs(s, f);
	if (count) {
		VGfloat *kernX, *kernY;
		if (f->Kerning) {
			kernX = &glyph_kern[0];
			kernY = &glyph_kern[count];
		} else {
			kernX = kernY = NULL;
		}
		vgDrawGlyphs(f->vgfont, (VGint) count, glyph_string, kernX, kernY, VG_FILL_PATH | VG_STROKE_PATH, VG_FALSE);
	}
	if (strokew != 0.0f) {
		vgSetf(VG_STROKE_LINE_WIDTH, strokew);
	}
}

// TextWidth returns the width of a text string at the specified font and size.
VGfloat TextWidth(const char *s, Fontinfo f, VGint pointsize) {
	VGfloat pos[2] = { 0.0f, 0.0f };
	unsigned int count = stringToGlyphs(s, f);
	if (count) {
		vgSetfv(VG_GLYPH_ORIGIN, 2, pos);
		VGfloat *kernX, *kernY;
		if (f->Kerning) {
			kernX = &glyph_kern[0];
			kernY = &glyph_kern[count];
		} else {
			kernX = kernY = NULL;
		}
		vgDrawGlyphs(f->vgfont, (VGint) count, glyph_string, kernX, kernY, 0, VG_FALSE);
		vgGetfv(VG_GLYPH_ORIGIN, 2, pos);
	}
	return pos[0] * (VGfloat) pointsize;
}

// In TextMid and TextEnd we pass glyph_string directly to Text as
// TextWidth has alreay parsed the string. Saves parsing it twice when we
// know that it cannot have changed.

// TextMid draws text, centered on (x,y)
void TextMid(VGfloat x, VGfloat y, const char *s, Fontinfo f, VGint pointsize) {
	VGfloat tw = TextWidth(s, f, pointsize);
	Text(x - (tw / 2.0f), y, (char *)glyph_string, f, pointsize);
}

// TextEnd draws text, with its end aligned to (x,y)
void TextEnd(VGfloat x, VGfloat y, const char *s, Fontinfo f, VGint pointsize) {
	VGfloat tw = TextWidth(s, f, pointsize);
	Text(x - tw, y, (char *)glyph_string, f, pointsize);
}

// TextHeight reports a font's height above baseline
VGfloat TextHeight(Fontinfo f, VGint pointsize) {
	return f->AscenderHeight * (VGfloat) pointsize;
}

// TextDepth reports a font's depth (how far under the baseline it goes)
VGfloat TextDepth(Fontinfo f, VGint pointsize) {
	return -f->DescenderHeight * (VGfloat) pointsize;
}

// TextLineHeight reports how far between baselines is recommended
VGfloat TextLineHeight(Fontinfo f, VGint pointsize) {
	return f->Height * (VGfloat) pointsize;
}

//
// Shape functions
//

// newpath creates path data
// Changed capabilities as others not needed at the moment - allows possible
// driver optimisations.
// Changed again to just clear out and re-use a common path.
static inline VGPath newpath() {
	vgClearPath(common_path, VG_PATH_CAPABILITY_APPEND_TO);
	return common_path;
}

// Shapes that have constant segment sizes are drawn by just modifying
// the coordinates of the shape's global path. This saves the constant
// allocation/deallocation which seems to periodically stall the RPi's
// gpu. These routines rely on knowledge of how the RPi's vgu routines
// create paths - other implementations my differ!

// CBezier makes a quadratic bezier curve
void Cbezier(VGfloat sx, VGfloat sy, VGfloat cx, VGfloat cy, VGfloat px, VGfloat py, VGfloat ex, VGfloat ey) {
	const VGfloat coords[8] = { sx, sy, cx, cy, px, py, ex, ey };
	vgModifyPathCoords(cbezier_path, 0, 2, coords);
	vgDrawPath(cbezier_path, VG_FILL_PATH | VG_STROKE_PATH);
}

// QBezier makes a quadratic bezier curve
void Qbezier(VGfloat sx, VGfloat sy, VGfloat cx, VGfloat cy, VGfloat ex, VGfloat ey) {
	const VGfloat coords[6] = { sx, sy, cx, cy, ex, ey };
	vgModifyPathCoords(qbezier_path, 0, 2, coords);
	vgDrawPath(qbezier_path, VG_FILL_PATH | VG_STROKE_PATH);
}

// interleave interleaves arrays of x, y into a single array
static void interleave(VGfloat * x, VGfloat * y, VGint n, VGfloat * points) {
	while (n--) {
		*points++ = *x++;
		*points++ = *y++;
	}
}

// poly makes either a polygon or polyline
static void poly(VGfloat * x, VGfloat * y, VGint n, VGbitfield flag) {
	VGfloat points[n * 2];
	VGPath path = newpath();
	interleave(x, y, n, points);
	vguPolygon(path, points, n, VG_FALSE);
	vgDrawPath(path, flag);
}

// Polygon makes a filled polygon with vertices in x, y arrays
void Polygon(VGfloat * x, VGfloat * y, VGint n) {
	poly(x, y, n, VG_FILL_PATH);
}

// Polyline makes a polyline with vertices at x, y arrays
void Polyline(VGfloat * x, VGfloat * y, VGint n) {
	poly(x, y, n, VG_STROKE_PATH);
}

// Rect makes a rectangle at the specified location and dimensions
void Rect(VGfloat x, VGfloat y, VGfloat w, VGfloat h) {
	const VGfloat coords[5] = { x, y, w, h, -w };
	vgModifyPathCoords(rect_path, 0, 4, coords);
	vgDrawPath(rect_path, VG_FILL_PATH | VG_STROKE_PATH);
}

// Line makes a line from (x1,y1) to (x2,y2)
void Line(VGfloat x1, VGfloat y1, VGfloat x2, VGfloat y2) {
	const VGfloat coords[4] = { x1, y1, x2, y2 };
	vgModifyPathCoords(line_path, 0, 2, coords);
	vgDrawPath(line_path, VG_STROKE_PATH);
}

// Roundrect makes an rounded rectangle at the specified location and dimensions
void Roundrect(VGfloat x, VGfloat y, VGfloat w, VGfloat h, VGfloat rw, VGfloat rh) {
	const VGfloat coords[26] = { x + rw / 2.0f, y,
		w - rw,
		rw / 2.0f, rh / 2.0f, 0.0f, rw / 2.0f, rh / 2.0f,
		h - rh,
		rw / 2.0f, rh / 2.0f, 0.0f, -rw / 2.0f, rh / 2.0f,
		-(w - rw),
		rw / 2.0f, rh / 2.0f, 0.0f, -rw / 2.0f, -rh / 2.0f,
		-(h - rh),
		rw / 2.0f, rh / 2.0f, 0.0f, rw / 2.0f, -rh / 2.0f
	};
	vgModifyPathCoords(roundrect_path, 0, 9, coords);
	vgDrawPath(roundrect_path, VG_FILL_PATH | VG_STROKE_PATH);
}

// Ellipse makes an ellipse at the specified location and dimensions
void Ellipse(VGfloat x, VGfloat y, VGfloat w, VGfloat h) {
	const VGfloat coords[12] = { x + w / 2.0f, y,
		w / 2.0f, h / 2.0f, 0.0f, -w, 0.0f,
		w / 2.0f, h / 2.0f, 0.0f, w, 0.0f
	};
	vgModifyPathCoords(ellipse_path, 0, 3, coords);
	vgDrawPath(ellipse_path, VG_FILL_PATH | VG_STROKE_PATH);
}

// Circle makes a circle at the specified location and dimensions
void Circle(VGfloat x, VGfloat y, VGfloat r) {
	Ellipse(x, y, r, r);
}

// Arc makes an elliptical arc at the specified location and dimensions
void Arc(VGfloat x, VGfloat y, VGfloat w, VGfloat h, VGfloat sa, VGfloat aext) {
	VGPath path = newpath();
	vguArc(path, x, y, w, h, sa, aext, VGU_ARC_OPEN);
	vgDrawPath(path, VG_FILL_PATH | VG_STROKE_PATH);
}

// Start begins the picture, clearing a rectangular region with a specified color
void Start(VGint width, VGint height) {
	VGfloat color[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
	vgSetfv(VG_CLEAR_COLOR, 4, color);
	vgClear(0, 0, width, height);
	color[0] = color[1] = color[2] = 0.0f;
	SetFill(color);
	SetStroke(color);
	StrokeWidth(0.0f);
	vgLoadIdentity();
}

// End checks for errors (if enabled) and renders to the display,
// returns false if an error was detected. 
bool End() {
	bool success = true;
	if (check_errors) {
		vg_error_code = vgGetError();
		if (vg_error_code != VG_NO_ERROR) {
			font_error(vg_error_code, "***End()***");
			success = false;
		}
	}

	eglSwapBuffers(state->display, state->surface);
	if (check_errors && eglGetError() != EGL_SUCCESS)
		success = false;
	return success;
}

// SaveEnd dumps the raster before rendering to the display, returns
// false if an error was detected (if enabled).
bool SaveEnd(const char *filename) {
	bool success = true;
	if (check_errors) {
		vg_error_code = vgGetError();
		if (vg_error_code != VG_NO_ERROR) {
			font_error(vg_error_code, "***End()***");
			success = false;
		}
	}
	FILE *fp;
	if (strlen(filename) == 0) {
		dumpscreen(state->screen_width, state->screen_height, stdout);
	} else {
		fp = fopen(filename, "wb");
		if (fp != NULL) {
			dumpscreen(state->screen_width, state->screen_height, fp);
			fclose(fp);
		}
	}
	eglSwapBuffers(state->display, state->surface);
	if (check_errors && eglGetError() != EGL_SUCCESS)
		success = false;
	return success;
}

// Backgroud clears the screen to a solid background color
void Background(VGuint r, VGuint g, VGuint b) {
	VGfloat colour[4];
	RGB(r, g, b, colour);
	vgSetfv(VG_CLEAR_COLOR, 4, colour);
	vgClear(0, 0, (VGint) state->window_width, (VGint) state->window_height);
}

// BackgroundRGBA clears the screen to a background color with alpha
void BackgroundRGBA(VGuint r, VGuint g, VGuint b, VGfloat a) {
	VGfloat colour[4];
	RGBA(r, g, b, a, colour);
	vgSetfv(VG_CLEAR_COLOR, 4, colour);
	vgClear(0, 0, (VGint) state->window_width, (VGint) state->window_height);
}

// Old name, doesn't match the parameters so deprecated in favour of
// BackgroundRGBA()
void BackgroundRGB(VGuint r, VGuint g, VGuint b, VGfloat a) {
	BackgroundRGBA(r, g, b, a);
}

// WindowClear clears the window to previously set background colour
void WindowClear() {
	vgClear(0, 0, (VGint) state->window_width, (VGint) state->window_height);
}

// AreaClear clears a given rectangle in window coordinates (not affected by
// transformations)
void AreaClear(VGint x, VGint y, VGint w, VGint h) {
	vgClear(x, y, w, h);
}

// WindowOpacity sets the  window opacity
void WindowOpacity(uint32_t a) {
	dispmanChangeWindowOpacity(state, a);
}

// WindowPosition moves the window to given position
void WindowPosition(int32_t x, int32_t y) {
	dispmanMoveWindow(state, x, y);
}

// Outlined shapes
// Hollow shapes, because filling still happens even with a fill of 0,0,0,0
// unlike where using a strokewidth of 0 disables the stroke.

// CBezier makes a quadratic bezier curve, stroked
void CbezierOutline(VGfloat sx, VGfloat sy, VGfloat cx, VGfloat cy, VGfloat px, VGfloat py, VGfloat ex, VGfloat ey) {
	const VGfloat coords[8] = { sx, sy, cx, cy, px, py, ex, ey };
	vgModifyPathCoords(cbezier_path, 0, 2, coords);
	vgDrawPath(cbezier_path, VG_STROKE_PATH);
}

// QBezierOutline makes a quadratic bezier curve, outlined 
void QbezierOutline(VGfloat sx, VGfloat sy, VGfloat cx, VGfloat cy, VGfloat ex, VGfloat ey) {
	const VGfloat coords[6] = { sx, sy, cx, cy, ex, ey };
	vgModifyPathCoords(qbezier_path, 0, 2, coords);
	vgDrawPath(qbezier_path, VG_STROKE_PATH);
}

// RectOutline makes a rectangle at the specified location and dimensions, outlined 
void RectOutline(VGfloat x, VGfloat y, VGfloat w, VGfloat h) {
	const VGfloat coords[5] = { x, y, w, h, -w };
	vgModifyPathCoords(rect_path, 0, 4, coords);
	vgDrawPath(rect_path, VG_STROKE_PATH);
}

// RoundrectOutline  makes an rounded rectangle at the specified location and dimensions, outlined 
void RoundrectOutline(VGfloat x, VGfloat y, VGfloat w, VGfloat h, VGfloat rw, VGfloat rh) {
	const VGfloat coords[26] = { x + rw / 2.0f, y,
		w - rw,
		rw / 2.0f, rh / 2.0f, 0.0f, rw / 2.0f, rh / 2.0f,
		h - rh,
		rw / 2.0f, rh / 2.0f, 0.0f, -rw / 2.0f, rh / 2.0f,
		-(w - rw),
		rw / 2.0f, rh / 2.0f, 0.0f, -rw / 2.0f, -rh / 2.0f,
		-(h - rh),
		rw / 2.0f, rh / 2.0f, 0.0f, rw / 2.0f, -rh / 2.0f
	};
	vgModifyPathCoords(roundrect_path, 0, 9, coords);
	vgDrawPath(roundrect_path, VG_STROKE_PATH);
}

// EllipseOutline makes an ellipse at the specified location and dimensions, outlined
void EllipseOutline(VGfloat x, VGfloat y, VGfloat w, VGfloat h) {
	const VGfloat coords[12] = { x + w / 2.0f, y,
		w / 2.0f, h / 2.0f, 0.0f, -w, 0.0f,
		w / 2.0f, h / 2.0f, 0.0f, w, 0.0f
	};
	vgModifyPathCoords(ellipse_path, 0, 3, coords);
	vgDrawPath(ellipse_path, VG_STROKE_PATH);
}

// CircleOutline makes a circle at the specified location and dimensions, outlined
void CircleOutline(VGfloat x, VGfloat y, VGfloat r) {
	EllipseOutline(x, y, r, r);
}

// ArcOutline makes an elliptical arc at the specified location and dimensions, outlined
void ArcOutline(VGfloat x, VGfloat y, VGfloat w, VGfloat h, VGfloat sa, VGfloat aext) {
	VGPath path = newpath();
	vguArc(path, x, y, w, h, sa, aext, VGU_ARC_OPEN);
	vgDrawPath(path, VG_STROKE_PATH);
}

// Path returning functions - these don't destroy the path, rather
// they return it so you can draw it many times. Otherwise equivalent
// to the same shape functions.

// newpath_ext creates new a new path, this one has all capabilities
// as the user may want to join paths together.
static inline VGPath newpath_ext() {
	return vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, 0, 0, VG_PATH_CAPABILITY_ALL);
}

VGPath CbezierPath(VGfloat sx, VGfloat sy, VGfloat cx, VGfloat cy, VGfloat px, VGfloat py, VGfloat ex, VGfloat ey) {
	VGubyte segments[] = { VG_MOVE_TO_ABS, VG_CUBIC_TO };
	VGfloat coords[] = { sx, sy, cx, cy, px, py, ex, ey };
	VGPath path = newpath_ext();
	if (path != VG_INVALID_HANDLE)
		vgAppendPathData(path, 2, segments, coords);
	return path;
}

VGPath QbezierPath(VGfloat sx, VGfloat sy, VGfloat cx, VGfloat cy, VGfloat ex, VGfloat ey) {
	VGubyte segments[] = { VG_MOVE_TO_ABS, VG_QUAD_TO };
	VGfloat coords[] = { sx, sy, cx, cy, ex, ey };
	VGPath path = newpath_ext();
	if (path != VG_INVALID_HANDLE)
		vgAppendPathData(path, 2, segments, coords);
	return path;
}

VGPath PolygonPath(VGfloat * x, VGfloat * y, VGint n) {
	VGfloat points[n * 2];
	VGPath path = newpath_ext();
	if (path != VG_INVALID_HANDLE) {
		interleave(x, y, n, points);
		vguPolygon(path, points, n, VG_FALSE);
	}
	return path;
}

VGPath RectPath(VGfloat x, VGfloat y, VGfloat w, VGfloat h) {
	VGPath path = newpath_ext();
	if (path != VG_INVALID_HANDLE)
		vguRect(path, x, y, w, h);
	return path;
}

VGPath LinePath(VGfloat x1, VGfloat y1, VGfloat x2, VGfloat y2) {
	VGPath path = newpath_ext();
	if (path != VG_INVALID_HANDLE)
		vguLine(path, x1, y1, x2, y2);
	return path;
}

VGPath RoundrectPath(VGfloat x, VGfloat y, VGfloat w, VGfloat h, VGfloat rw, VGfloat rh) {
	VGPath path = newpath_ext();
	if (path != VG_INVALID_HANDLE)
		vguRoundRect(path, x, y, w, h, rw, rh);
	return path;
}

VGPath EllipsePath(VGfloat x, VGfloat y, VGfloat w, VGfloat h) {
	VGPath path = newpath_ext();
	if (path != VG_INVALID_HANDLE)
		vguEllipse(path, x, y, w, h);
	return path;
}

VGPath CirclePath(VGfloat x, VGfloat y, VGfloat r) {
	return EllipsePath(x, y, r, r);
}

VGPath ArcPath(VGfloat x, VGfloat y, VGfloat w, VGfloat h, VGfloat sa, VGfloat aext) {
	VGPath path = newpath_ext();
	if (path != VG_INVALID_HANDLE)
		vguArc(path, x, y, w, h, sa, aext, VGU_ARC_OPEN);
	return path;
}

// DrawPath draws the path with both fill and stroke
void DrawPath(VGPath path) {
	vgDrawPath(path, VG_FILL_PATH | VG_STROKE_PATH);
}

// DrawPathOutline draws the path with stroke only
void DrawPathOutline(VGPath path) {
	vgDrawPath(path, VG_STROKE_PATH);
}

// DrawPathAt draws the path after translating the origin to x,y
void DrawPathAt(VGfloat x, VGfloat y, VGPath path) {
	VGfloat matrix[9];
	VGMatrixMode mode = vgGeti(VG_MATRIX_MODE);
	if (mode != VG_MATRIX_PATH_USER_TO_SURFACE)
		vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
	vgGetMatrix(matrix);
	vgTranslate(x, y);
	vgDrawPath(path, VG_FILL_PATH | VG_STROKE_PATH);
	vgLoadMatrix(matrix);
	if (mode != VG_MATRIX_PATH_USER_TO_SURFACE)
		vgSeti(VG_MATRIX_MODE, mode);
}

// DrawPathOutlineAt draws the outline path after translating the origin to x,y
void DrawPathOutlineAt(VGfloat x, VGfloat y, VGPath path) {
	VGfloat matrix[9];
	VGMatrixMode mode = vgGeti(VG_MATRIX_MODE);
	if (mode != VG_MATRIX_PATH_USER_TO_SURFACE)
		vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
	vgGetMatrix(matrix);
	vgTranslate(x, y);
	vgDrawPath(path, VG_STROKE_PATH);
	vgLoadMatrix(matrix);
	if (mode != VG_MATRIX_PATH_USER_TO_SURFACE)
		vgSeti(VG_MATRIX_MODE, mode);
}

// DeletePath frees the given path
void DeletePath(VGPath path) {
	vgDestroyPath(path);
}

// Dot draws a 1 unit dot with lower left extent at x, y either as a
// cicle (smooth==true) or rectangle (smooth=false).
// Only draws using fill (not stroke).
void Dot(VGfloat x, VGfloat y, bool smooth) {
	VGfloat matrix[9];
	VGMatrixMode mode = vgGeti(VG_MATRIX_MODE);
	if (mode != VG_MATRIX_PATH_USER_TO_SURFACE)
		vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
	vgGetMatrix(matrix);
	vgTranslate(x, y);
	vgDrawPath(smooth ? dot_smooth_path : dot_rough_path, VG_FILL_PATH);
	vgLoadMatrix(matrix);
	if (mode != VG_MATRIX_PATH_USER_TO_SURFACE)
		vgSeti(VG_MATRIX_MODE, mode);
}

// Paint returns a paint of the specified colour
VGPaint Paint(VGuint r, VGuint g, VGuint b, VGfloat a) {
	VGfloat colour[4];
	RGBA(r, g, b, a, colour);
	VGPaint paint = vgCreatePaint();
	if (paint != VG_INVALID_HANDLE) {
		vgSetParameteri(paint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
		vgSetParameterfv(paint, VG_PAINT_COLOR, 4, colour);
	}
	return paint;
}

VGPaint LinearGradientPaint(VGfloat x1, VGfloat y1, VGfloat x2, VGfloat y2, VGfloat * stops, VGint ns) {
	VGfloat lgcoord[4] = { x1, y1, x2, y2 };
	VGPaint paint = vgCreatePaint();
	if (paint != VG_INVALID_HANDLE) {
		vgSetParameteri(paint, VG_PAINT_TYPE, VG_PAINT_TYPE_LINEAR_GRADIENT);
		vgSetParameterfv(paint, VG_PAINT_LINEAR_GRADIENT, 4, lgcoord);
		setstop(paint, stops, ns);
	}
	return paint;
}

// RadialGradient fills with a linear gradient
VGPaint RadialGradientPaint(VGfloat cx, VGfloat cy, VGfloat fx, VGfloat fy, VGfloat radius, VGfloat * stops, VGint ns) {
	VGfloat radialcoord[5] = { cx, cy, fx, fy, radius };
	VGPaint paint = vgCreatePaint();
	if (paint != VG_INVALID_HANDLE) {
		vgSetParameteri(paint, VG_PAINT_TYPE, VG_PAINT_TYPE_RADIAL_GRADIENT);
		vgSetParameterfv(paint, VG_PAINT_RADIAL_GRADIENT, 5, radialcoord);
		setstop(paint, stops, ns);
	}
	return paint;
}

// Deletes a paint
void DeletePaint(VGPaint paint) {
	vgDestroyPaint(paint);
}

// Sets the fill to be paint
void FillPaint(VGPaint paint) {
	vgSetPaint(paint, VG_FILL_PATH);
}

// Sets the stroke to be paint
void StrokePaint(VGPaint paint) {
	vgSetPaint(paint, VG_STROKE_PATH);
}

// Take a copy of an area of the window ready for saving
static char *grabWindow(VGint x, VGint y, VGint * w, VGint * h) {
	// If either x,y is off screen then set to 0
	if ((x < 0) || (x > (VGint) state->window_width))
		x = 0;
	if ((y < 0) || (y > (VGint) state->window_height))
		y = 0;
	// Now make sure w,h is valid, reducing if need be
	VGint width = *w;
	VGint height = *h;
	if (width <= 0)
		width = (VGint) state->window_width;
	if ((x + width) > (VGint) state->window_width)
		width = (VGint) state->window_width - x;
	if (height <= 0)
		height = (VGint) state->window_height;
	if ((y + height) > (VGint) state->window_height)
		height = (VGint) state->window_height - y;
	*w = width;
	*h = height;
	char *ScreenBuffer = malloc((size_t) (width * height * 4));
	if (ScreenBuffer)
		vgReadPixels(ScreenBuffer, width * 4, VG_sABGR_8888, x, y, width, height);
	return ScreenBuffer;
}

// Save an area of the window from (x,y) at a size of (w,h) to a .png
// file. Returns true on success.
bool WindowSaveAsPng(const char *filename, VGint x, VGint y, VGint w, VGint h, int zlib_level) {
	bool success = false;
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
						      NULL, NULL, NULL);
	if (png_ptr == NULL) {
		return false;
	}
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr) {
		FILE *file = NULL;
		int width = w;
		int height = h;
		char *image = grabWindow(x, y, &width, &height);
		if (image != NULL) {
			file = fopen(filename, "wb");
			if (file) {
				if (!setjmp(png_jmpbuf(png_ptr))) {
					png_init_io(png_ptr, file);
					png_set_IHDR(png_ptr, info_ptr, width, height,
						     8, PNG_COLOR_TYPE_RGB,
						     PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
					png_set_compression_level(png_ptr, zlib_level);
					png_write_info(png_ptr, info_ptr);
					png_set_packing(png_ptr);
					png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);
					png_bytep row_pointer;
					row_pointer = (png_bytep) image + width * 4 * height;
					VGint row;
					for (row = height; row; row--) {
						row_pointer -= width * 4;
						png_write_rows(png_ptr, &row_pointer, 1);
					}
					png_write_end(png_ptr, info_ptr);
					success = true;
				} else
					success = false;
				fclose(file);
			}
			free(image);
		}
	}
	png_destroy_write_struct(&png_ptr, &info_ptr);
	return success;
}

 // Load a PNG from filename into a VGImage, return width & height in
 // pointers. For now we'll use the basic high-level reading.
VGImage CreateImageFromPng(const char *filename) {
	VGImage image = VG_INVALID_HANDLE;
	FILE *file = fopen(filename, "rb");
	if (file == NULL)
		return VG_INVALID_HANDLE;
	const unsigned int sig_bytes = 8;
	char header[sig_bytes];
	if ((fread(header, 1, sig_bytes, file) != sig_bytes) || png_sig_cmp((png_bytep) header, 0, sig_bytes)) {
		fclose(file);
		return VG_INVALID_HANDLE;
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
						     NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fclose(file);
		return VG_INVALID_HANDLE;
	}
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fclose(file);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return VG_INVALID_HANDLE;
	}

	png_uint_32 width, height;
	int bpp, colour, interlace;
	png_bytep buffer = NULL;
	png_bytep *row_ptrs = NULL;
	if (!setjmp(png_jmpbuf(png_ptr))) {
		png_init_io(png_ptr, file);
		png_set_sig_bytes(png_ptr, sig_bytes);
		png_read_info(png_ptr, info_ptr);
		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bpp, &colour, &interlace, NULL, NULL);
		if (bpp == 16)
			png_set_strip_16(png_ptr);
		if (bpp < 8) {
			png_set_packing(png_ptr);
			if (colour == PNG_COLOR_TYPE_GRAY)
				png_set_expand_gray_1_2_4_to_8(png_ptr);
		}
		if (colour == PNG_COLOR_TYPE_PALETTE)
			png_set_palette_to_rgb(png_ptr);
		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) != 0)
			png_set_tRNS_to_alpha(png_ptr);
		if (colour == PNG_COLOR_TYPE_RGB || colour == PNG_COLOR_TYPE_GRAY)
			png_set_add_alpha(png_ptr, 0xffff, PNG_FILLER_AFTER);
		if (colour == PNG_COLOR_TYPE_GRAY || colour == PNG_COLOR_TYPE_GRAY_ALPHA)
			png_set_gray_to_rgb(png_ptr);
		png_read_update_info(png_ptr, info_ptr);
		buffer = malloc(height * width * 4);
		row_ptrs = malloc(height * sizeof *row_ptrs);
		if (buffer == NULL || row_ptrs == NULL)
			longjmp(png_jmpbuf(png_ptr), 1);
		unsigned int i;
		png_bytep rbuf = buffer + 4 * width * height;
		for (i = 0; i < height; i++) {
			rbuf -= width * 4;
			row_ptrs[i] = rbuf;
		}
		png_read_image(png_ptr, row_ptrs);
		png_read_end(png_ptr, info_ptr);
		image = vgCreateImage(VG_sABGR_8888, width, height, VG_IMAGE_QUALITY_BETTER);
		if (image != VG_INVALID_HANDLE)
			vgImageSubData(image, buffer, width * 4, VG_sABGR_8888, 0, 0, width, height);
		else
			vg_error_code = vgGetError();
	}
	if (row_ptrs)
		free(row_ptrs);
	if (buffer)
		free(buffer);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(file);
	return image;
}

// DrawImageAt draws the image at a specified location
void DrawImageAt(VGfloat x, VGfloat y, VGImage image) {
	if (x == 0.0f && y == 0.0f) {
		vgDrawImage(image);
		return;
	}

	VGint mm = vgGeti(VG_MATRIX_MODE);
	if (mm != VG_MATRIX_IMAGE_USER_TO_SURFACE)
		vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
	VGfloat matrix[9];
	vgGetMatrix(matrix);
	vgTranslate(x, y);
	vgDrawImage(image);
	vgLoadMatrix(matrix);
	if (mm != VG_MATRIX_IMAGE_USER_TO_SURFACE)
		vgSeti(VG_MATRIX_MODE, mm);
}

// DrawImageAtFit draws the image scaled to fit in requested size
void DrawImageAtFit(VGfloat x, VGfloat y, VGfloat w, VGfloat h, VGImage image) {
	VGint iw = vgGetParameteri(image, VG_IMAGE_WIDTH);
	VGint ih = vgGetParameteri(image, VG_IMAGE_HEIGHT);
	if (iw == 0 || ih == 0) {
		vg_error_code = vgGetError();
		return;
	}

	VGint mm = vgGeti(VG_MATRIX_MODE);
	if (mm != VG_MATRIX_IMAGE_USER_TO_SURFACE)
		vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
	VGfloat matrix[9];
	vgGetMatrix(matrix);
	VGfloat sx = w / (VGfloat) iw;
	VGfloat sy = h / (VGfloat) ih;
	vgTranslate(x, y);
	vgScale(sx, sy);
	vgDrawImage(image);
	vgLoadMatrix(matrix);
	if (mm != VG_MATRIX_IMAGE_USER_TO_SURFACE)
		vgSeti(VG_MATRIX_MODE, mm);
}

//
// Miscellaneous utility functions.
//

// CopyMatrixPathToImage copies the path matrix to the image matrix.
void CopyMatrixPathToImage() {
	VGint mm = vgGeti(VG_MATRIX_MODE);
	if (mm != VG_MATRIX_PATH_USER_TO_SURFACE)
		vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
	VGfloat matrix[9];
	vgGetMatrix(matrix);
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
	vgLoadMatrix(matrix);
	if (mm != VG_MATRIX_IMAGE_USER_TO_SURFACE)
		vgSeti(VG_MATRIX_MODE, mm);
}

// Create a cursor image above the OpenVG layer.
bool CreateCursor(uint32_t * data, uint32_t width, uint32_t height, uint32_t hot_x, uint32_t hot_y) {
	if (!state->dmx_display || priv_cursor != NULL)
		return false;
	priv_cursor = createCursor(state, data, width, height, hot_x, hot_y, false);
	return (priv_cursor != NULL);
}

// Same as CreateCursor but get the image data directly from an
// existing VGImage.
bool CreateCursorFromVGImage(VGImage img, uint32_t hot_x, uint32_t hot_y) {
	if (!state->dmx_display || priv_cursor != NULL)
		return false;
	VGint w = vgGetParameteri(img, VG_IMAGE_WIDTH);
	VGint h = vgGetParameteri(img, VG_IMAGE_HEIGHT);
	if (w == 0 || h == 0) {
		vg_error_code = vgGetError();
		return false;
	}

	uint32_t *data = malloc(w * 4 * h);
	if (data == NULL)
		return false;

	vgGetImageSubData(img, data, w * 4, VG_sABGR_8888, 0, 0, w, h);
	priv_cursor = createCursor(state, data, w, h, hot_x, hot_y, true);
	free(data);
	return (priv_cursor != NULL);
}

void ShowCursor() {
	showCursor(priv_cursor);
}

void HideCursor() {
	hideCursor(priv_cursor);
}

// Moves cursor to window coordinte (origin top-left)
void MoveHWCursor(int32_t x, int32_t y) {
	moveCursor(state, priv_cursor, x, y);
}

// Moves cursor to OpenVG coordinte (origin bottom-left)
void MoveCursor(int32_t x, int32_t y) {
	moveCursor(state, priv_cursor, x, (int32_t) state->window_height - 1 - y);
}

void DeleteCursor() {
	if (priv_cursor) {
		deleteCursor(priv_cursor);
		priv_cursor = NULL;
	}
}

void ScreenBrightness(uint32_t level) {
	screenBrightness(state, level);
}

#ifndef OPENVG_SHAPES_H
#define OPENVG_SHAPES_H

#include <stdint.h>
#include <stdbool.h>
#include <VG/openvg.h>
#include <VG/vgu.h>
#include "fontinfo.h"

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef __GNUC__
#define DEPRECATED __attribute__((__deprecated__))
#else
#define DEPRECATED
#endif
	extern void Translate(VGfloat, VGfloat);
	extern void Rotate(VGfloat);
	extern void Shear(VGfloat, VGfloat);
	extern void Scale(VGfloat, VGfloat);
	extern void Text(VGfloat, VGfloat, const char *, Fontinfo, VGint);
	extern void TextMid(VGfloat, VGfloat, const char *, Fontinfo, VGint);
	extern void TextEnd(VGfloat, VGfloat, const char *, Fontinfo, VGint);
	extern VGfloat TextWidth(const char *, Fontinfo, VGint);
	extern void Cbezier(VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat);
	extern void Qbezier(VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat);
	extern void Polygon(VGfloat *, VGfloat *, VGint);
	extern void Polyline(VGfloat *, VGfloat *, VGint);
	extern void Rect(VGfloat, VGfloat, VGfloat, VGfloat);
	extern void Line(VGfloat, VGfloat, VGfloat, VGfloat);
	extern void Roundrect(VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat);
	extern void Ellipse(VGfloat, VGfloat, VGfloat, VGfloat);
	extern void Circle(VGfloat, VGfloat, VGfloat);
	extern void Arc(VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat);
	extern void Image(VGfloat, VGfloat, VGint, VGint, const char *);
	extern void Start(VGint, VGint);
	extern bool End();
	extern bool SaveEnd(const char *);
	extern void Background(VGuint, VGuint, VGuint);
	// Deprecate BackgroundRGB, should really be BackgroundRGBA
	extern void BackgroundRGB(VGuint, VGuint, VGuint, VGfloat) DEPRECATED;
	extern void BackgroundRGBA(VGuint, VGuint, VGuint, VGfloat);
	// Deprecate non capitalised function names
	extern bool init(int32_t *, int32_t *) DEPRECATED;
	extern void finish() DEPRECATED;
	extern void setfill(VGfloat[4]) DEPRECATED;
	extern void setstroke(VGfloat[4]) DEPRECATED;
	// New function names
	extern bool InitShapes(int32_t *, int32_t *);
	extern void FinishShapes();
	extern void SetFill(VGfloat[4]);
	extern void SetStroke(VGfloat[4]);

	extern void StrokeWidth(VGfloat);
	extern void Stroke(VGuint, VGuint, VGuint, VGfloat);
	extern void Fill(VGuint, VGuint, VGuint, VGfloat);
	extern void RGBA(VGuint, VGuint, VGuint, VGfloat, VGfloat[4]);
	extern void RGB(VGuint, VGuint, VGuint, VGfloat[4]);
	extern void FillLinearGradient(VGfloat, VGfloat, VGfloat, VGfloat, VGfloat *, VGint);
	extern void FillRadialGradient(VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat *, VGint);
	extern void ClipRect(VGint x, VGint y, VGint w, VGint h);
	extern void ClipEnd();
	// Deprecate non capitalised function names
	extern Fontinfo loadfont(const int *, const int *, const unsigned char *, const int *, const int *, const int *,
				 const short *, int, int, int) DEPRECATED;
	extern void unloadfont(Fontinfo) DEPRECATED;
	extern void makeimage(VGfloat, VGfloat, VGint, VGint, VGubyte *) DEPRECATED;
	extern void saveterm() DEPRECATED;
	extern void restoreterm() DEPRECATED;
	extern void rawterm() DEPRECATED;
	extern VGImage createImageFromJpeg(const char *) DEPRECATED;
	// New function names
	extern Fontinfo LoadFont(const int *, const int *, const unsigned char *, const int *, const int *, const int *,
				 const short *, int, int, int);
	extern void UnloadFont(Fontinfo);
	extern void MakeImage(VGfloat, VGfloat, VGint, VGint, VGubyte *);
	extern void SaveTerm();
	extern void RestoreTerm();
	extern void RawTerm();
	extern VGImage CreateImageFromJpeg(const char *);

	// Added by Paeryn
	extern void initWindowSize(int32_t x, int32_t y, int32_t w, int32_t h) DEPRECATED;
	extern void InitWindowSize(int32_t x, int32_t y, int32_t w, int32_t h);
	extern void EnableOpenVGErrorCheck(bool check);
	extern uint32_t CheckErrorStatus();
	extern Fontinfo LoadTTFFile(const char *fname);
	extern Fontinfo LoadTTF(const char *font_style);
	extern void UnloadTTF(Fontinfo f);
	extern void FontKerning(Fontinfo f, int kerning);
	extern VGfloat TextHeight(Fontinfo f, VGint pointsize);
	extern VGfloat TextDepth(Fontinfo f, VGint pointsize);
	extern VGfloat TextLineHeight(Fontinfo f, VGint pointsize);

	extern void AreaClear(VGint x, VGint y, VGint w, VGint h);
	extern void WindowClear();
	extern void WindowOpacity(uint32_t alpha);
	extern void WindowPosition(int32_t x, int32_t y);
	extern bool WindowSaveAsPng(const char *filename, VGint x, VGint y, VGint w, VGint h, int zlib_level);
	extern VGImage CreateImageFromPng(const char *filename);
	extern void DrawImageAt(VGfloat x, VGfloat y, VGImage image);
	extern void DrawImageAtFit(VGfloat x, VGfloat y, VGfloat w, VGfloat h, VGImage image);
	extern void CopyMatrixPathToImage();

	// Outline shapes
	extern void CbezierOutline(VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat);
	extern void QbezierOutline(VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat);
	extern void RectOutline(VGfloat, VGfloat, VGfloat, VGfloat);
	extern void RoundrectOutline(VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat);
	extern void EllipseOutline(VGfloat, VGfloat, VGfloat, VGfloat);
	extern void CircleOutline(VGfloat, VGfloat, VGfloat);
	extern void ArcOutline(VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat);
	// Path shapes
	extern VGPath CbezierPath(VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat);
	extern VGPath QbezierPath(VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat);
	extern VGPath PolygonPath(VGfloat *, VGfloat *, VGint);
	extern VGPath RectPath(VGfloat, VGfloat, VGfloat, VGfloat);
	extern VGPath LinePath(VGfloat, VGfloat, VGfloat, VGfloat);
	extern VGPath RoundrectPath(VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat);
	extern VGPath EllipsePath(VGfloat x, VGfloat y, VGfloat w, VGfloat h);
	extern VGPath CirclePath(VGfloat x, VGfloat y, VGfloat r);
	extern VGPath ArcPath(VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat);
	extern void DrawPath(VGPath path);
	extern void DrawPathAt(VGfloat x, VGfloat y, VGPath path);
	extern void DrawPathOutline(VGPath path);
	extern void DrawPathOutlineAt(VGfloat x, VGfloat y, VGPath path);
	extern void DeletePath(VGPath path);
	extern void Dot(VGfloat x, VGfloat y, bool smooth);
	extern VGPaint Paint(unsigned int r, unsigned int g, unsigned int b, float a);
	extern VGPaint LinearGradientPaint(VGfloat, VGfloat, VGfloat, VGfloat, VGfloat *, int);
	extern VGPaint RadialGradientPaint(VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat *, int);
	extern void DeletePaint(VGPaint paint);
	extern void FillPaint(VGPaint paint);
	extern void StrokePaint(VGPaint paint);

	// Cursor
	extern bool CreateCursor(uint32_t * data, uint32_t width, uint32_t height, uint32_t hot_x, uint32_t hot_y);
	extern bool CreateCursorFromVGImage(VGImage img, uint32_t hot_x, uint32_t hot_y);
	extern void ShowCursor();
	extern void HideCursor();
	extern void MoveHWCursor(int32_t x, int32_t y);
	extern void MoveCursor(int32_t x, int32_t y);
	extern void DeleteCursor();
	extern void ScreenBrightness(uint32_t level);

#undef DEPRECATED
#if defined(__cplusplus)
}
#endif
// Color name defines for use in functions using RGB triples
#define color_aliceblue		240,248,255
#define color_antiquewhite		250,235,215
#define color_aqua 		0,255,255
#define color_aquamarine		127,255,212
#define color_azure		240,255,255
#define color_beige		245,245,220
#define color_bisque		255,228,196
#define color_black		0,0,0
#define color_blanchedalmond	255,235,205
#define color_blue			0,0,255
#define color_blueviolet		138,43,226
#define color_brown		165,42,42
#define color_burlywood		222,184,135
#define color_cadetblue		95,158,160
#define color_chartreuse		127,255,0
#define color_chocolate		210,105,30
#define color_coral		255,127,80
#define color_cornflowerblue	100,149,237
#define color_cornsilk		255,248,220
#define color_crimson		220,20,60
#define color_cyan			0,255,255
#define color_darkblue		0,0,139
#define color_darkcyan		0,139,139
#define color_darkgoldenrod	184,134,11
#define color_darkgray		169,169,169
#define color_darkgreen		0,100,0
#define color_darkgrey		169,169,169
#define color_darkkhaki		189,183,107
#define color_darkmagenta		139,0,139
#define color_darkolivegreen	85,107,47
#define color_darkorange		255,140,0
#define color_darkorchid		153,50,204
#define color_darkred		139,0,0
#define color_darksalmon		233,150,122
#define color_darkseagreen		143,188,143
#define color_darkslateblue	72,61,139
#define color_darkslategray	47,79,79
#define color_darkslategrey	47,79,79
#define color_darkturquoise	0,206,209
#define color_darkviolet		148,0,211
#define color_deeppink		255,20,147
#define color_deepskyblue		0,191,255
#define color_dimgray		105,105,105
#define color_dimgrey		105,105,105
#define color_dodgerblue		30,144,255
#define color_firebrick		178,34,34
#define color_floralwhite		255,250,240
#define color_forestgreen		34,139,34
#define color_fuchsia		255,0,255
#define color_gainsboro		220,220,220
#define color_ghostwhite		248,248,255
#define color_gold			255,215,0
#define color_goldenrod		218,165,32
#define color_gray			128,128,128
#define color_green		0,128,0
#define color_greenyellow		173,255,47
#define color_grey			128,128,128
#define color_honeydew		240,255,240
#define color_hotpink		255,105,180
#define color_indianred		205,92,92
#define color_indigo		75,0,130
#define color_ivory		255,255,240
#define color_khaki		240,230,140
#define color_lavender		230,230,250
#define color_lavenderblush	255,240,245
#define color_lawngreen		124,252,0
#define color_lemonchiffon		255,250,205
#define color_lightblue		173,216,230
#define color_lightcoral		240,128,128
#define color_lightcyan		224,255,255
#define color_lightgoldenrodyellow	250,250,210
#define color_lightgray		211,211,211
#define color_lightgreen		144,238,144
#define color_lightgrey		211,211,211
#define color_lightpink		255,182,193
#define color_lightsalmon		255,160,122
#define color_lightseagreen	32,178,170
#define color_lightskyblue		135,206,250
#define color_lightslategray	119,136,153
#define color_lightslategrey	119,136,153
#define color_lightsteelblue	176,196,222
#define color_lightyellow		255,255,224
#define color_lime			0,255,0
#define color_limegreen		50,205,50
#define color_linen		250,240,230
#define color_magenta		255,0,255
#define color_maroon		128,0,0
#define color_mediumaquamarine	102,205,170
#define color_mediumblue		0,0,205
#define color_mediumorchid		186,85,211
#define color_mediumpurple		147,112,219
#define color_mediumseagreen	60,179,113
#define color_mediumslateblue	123,104,238
#define color_mediumspringgreen	0,250,154
#define color_mediumturquoise	72,209,204
#define color_mediumvioletred	199,21,133
#define color_midnightblue		25,25,112
#define color_mintcream		245,255,250
#define color_mistyrose		255,228,225
#define color_moccasin		255,228,181
#define color_navajowhite		255,222,173
#define color_navy			0,0,128
#define color_oldlace		253,245,230
#define color_olive		128,128,0
#define color_olivedrab		107,142,35
#define color_orange		255,165,0
#define color_orangered		255,69,0
#define color_orchid		218,112,214
#define color_palegoldenrod	238,232,170
#define color_palegreen		152,251,152
#define color_paleturquoise	175,238,238
#define color_palevioletred	219,112,147
#define color_papayawhip		255,239,213
#define color_peachpuff		255,218,185
#define color_peru			205,133,63
#define color_pink			255,192,203
#define color_plum			221,160,221
#define color_powderblue		176,224,230
#define color_purple		128,0,128
#define color_red			255,0,0
#define color_rosybrown		188,143,143
#define color_royalblue		65,105,225
#define color_saddlebrown		139,69,19
#define color_salmon		250,128,114
#define color_sandybrown		244,164,96
#define color_seagreen		46,139,87
#define color_seashell		255,245,238
#define color_sienna		160,82,45
#define color_silver		192,192,192
#define color_skyblue		135,206,235
#define color_slateblue		106,90,205
#define color_slategray		112,128,144
#define color_slategrey		112,128,144
#define color_snow			255,250,250
#define color_springgreen		0,255,127
#define color_steelblue		70,130,180
#define color_tan			210,180,140
#define color_teal			0,128,128
#define color_thistle		216,191,216
#define color_tomato		255,99,71
#define color_turquoise		64,224,208
#define color_violet		238,130,238
#define color_wheat		245,222,179
#define color_white		255,255,255
#define color_whitesmoke		245,245,245
#define color_yellow		255,255,0
#define color_yellowgreen		154,205,50
#endif				// OPENVG_SHAPES_H

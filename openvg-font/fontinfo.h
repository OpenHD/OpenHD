#ifndef OPENVG_FONTINFO_H
#define OPENVG_FONTINFO_H

#include "VG/openvg.h"

#if defined(__cplusplus)
extern "C" {
#endif
	typedef struct Fontinfo_T {
		const short *CharacterMap;
		const int *GlyphAdvances;
		unsigned int Count;
		VGfloat DescenderHeight;	// How far below baseline (-ve)
		VGfloat AscenderHeight;	// How far above baseline
		VGfloat Height;	// How far between baselines
		const char *Name;	// Name of font family
		const char *Style;	// Name of font style
		int Kerning;	// Apply kerning to text
		VGFont vgfont;	// Private
		void *face;	// Private
	} Fontinfo_T;
	typedef struct Fontinfo_T *Fontinfo;

//      extern Fontinfo SansTypeface, SerifTypeface, MonoTypeface;
	extern Fontinfo SansTypeface, MonoTypeface;
#if defined(__cplusplus)
}
#endif
#endif				// OPENVG_FONTINFO_H

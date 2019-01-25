#ifndef OPENVG_FONTSYSTEM_H
#define OPENVG_FONTSYSTEM_H

#include "VG/openvg.h"

typedef enum 
{
        FONT_NO_ERROR       = 0,
        FONT_NO_FREETYPE    = 1,
        FONT_NO_FILE        = 2,
        FONT_NOT_SCALABLE   = 3,
        FONT_OUT_OF_MEMORY  = 4,
        FONT_NO_VGFONT      = 5,
        FONT_BAD_HANDLE_ERROR = VG_BAD_HANDLE_ERROR,
        FONT_ILLEGAL_ARGUMENT_ERROR,
        FONT_OUT_OF_MEMORY_ERROR,
        FONT_PATH_CAPABILITY_ERROR,
        FONT_UNSUPPORTED_IMAGE_FORMAT_ERROR,
        FONT_UNSUPPORTED_PATH_FORMAT_ERROR,
        FONT_IMAGE_IN_USE_ERROR,
        FONT_NO_CONTEXT_ERROR
} FontErrorCode;

#if defined(__cplusplus)
extern "C" {
#endif
        extern void font_CloseFontSystem();
        extern unsigned int font_CharToGlyph(void *face, unsigned long code);
        extern void font_KernData(void *face, unsigned long curr, unsigned long prev, VGfloat *kernX, VGfloat *kernY);
        extern void font_error(FontErrorCode error, const char *filename);

#if defined(__cplusplus)
}
#endif
#endif // OPENVG_FONTSYSTEM_H

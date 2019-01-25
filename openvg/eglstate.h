typedef struct {
	// Screen dimentions
	uint32_t screen_width;
	uint32_t screen_height;
	// Window dimentions
	int32_t window_x;
	int32_t window_y;
	uint32_t window_width;
	uint32_t window_height;
	// dispman window 
	DISPMANX_ELEMENT_HANDLE_T element;
        // dispman display
        DISPMANX_DISPLAY_HANDLE_T dmx_display;
        
	// EGL data
	EGLDisplay display;

	EGLSurface surface;
	EGLContext context;
} STATE_T;

typedef struct cursor_t {
    STATE_T state;
    DISPMANX_RESOURCE_HANDLE_T resource;
    DISPMANX_DISPLAY_HANDLE_T display;
    int32_t hot_x;
    int32_t hot_y;
} cursor_t;

extern void oglinit(STATE_T *);
extern void dispmanMoveWindow(STATE_T *, int, int);
extern void dispmanChangeWindowOpacity(STATE_T *, unsigned int);
extern cursor_t *createCursor(STATE_T *state, const uint32_t *data, uint32_t w, uint32_t h, uint32_t hx, uint32_t hy, bool upsidedown);
extern void showCursor(cursor_t *cursor);
extern void hideCursor(cursor_t *cursor);
extern void moveCursor(STATE_T *state, cursor_t *cursor, int32_t x, int32_t y);
extern void deleteCursor(cursor_t *cursor);
extern void screenBrightness(STATE_T *state, uint32_t level);

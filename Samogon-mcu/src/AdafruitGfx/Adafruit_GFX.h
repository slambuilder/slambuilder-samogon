#ifndef _ADAFRUIT_GFX_H
#define _ADAFRUIT_GFX_H

#include "gfxfont.h"

typedef struct TAdafruitGfx AdafruitGfx;

// public non-virtual methods
void AdafruitGfx_constructor(AdafruitGfx *_this, int16_t w, int16_t h);

// public virtual methods
typedef void (*vAdafruitGfx_destructor)(AdafruitGfx *_this);
typedef void (*vAdafruitGfx_drawPixel)(AdafruitGfx *_this, int16_t x, int16_t y, uint16_t color);
typedef void (*vAdafruitGfx_startWrite)(AdafruitGfx *_this);
typedef void (*vAdafruitGfx_writePixel)(AdafruitGfx *_this, int16_t x, int16_t y, uint16_t color);
typedef void (*vAdafruitGfx_writeFillRect)(AdafruitGfx *_this, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
typedef void (*vAdafruitGfx_writeFastVLine)(AdafruitGfx *_this, int16_t x, int16_t y, int16_t h, uint16_t color);
typedef void (*vAdafruitGfx_writeFastHLine)(AdafruitGfx *_this, int16_t x, int16_t y, int16_t w, uint16_t color);
typedef void (*vAdafruitGfx_writeLine)(AdafruitGfx *_this, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
typedef void (*vAdafruitGfx_endWrite)(AdafruitGfx *_this);

typedef void (*vAdafruitGfx_setRotation)(AdafruitGfx *_this, uint8_t r);
typedef void (*vAdafruitGfx_invertDisplay)(AdafruitGfx *_this, bool i);
typedef void (*vAdafruitGfx_drawFastVLine)(AdafruitGfx *_this, int16_t x, int16_t y, int16_t h, uint16_t color);
typedef void (*vAdafruitGfx_drawFastHLine)(AdafruitGfx *_this, int16_t x, int16_t y, int16_t w, uint16_t color);
typedef void (*vAdafruitGfx_fillRect)(AdafruitGfx *_this, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
typedef void (*vAdafruitGfx_fillScreen)(AdafruitGfx *_this, uint16_t color);
typedef void (*vAdafruitGfx_drawLine)(AdafruitGfx *_this, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
typedef void (*vAdafruitGfx_drawRect)(AdafruitGfx *_this, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

typedef size_t (*vAdafruitGfx_write)(AdafruitGfx *_this, uint8_t ch);

// typedef for AdafruitGfx Virtual method table
struct AdafruitGfx_VMT
{
	vAdafruitGfx_destructor			pfnDestructor;
	vAdafruitGfx_drawPixel			pfnDrawPixel;
	vAdafruitGfx_startWrite			pfnStartWrite;
	vAdafruitGfx_writePixel			pfnWritePixel;
	vAdafruitGfx_writeFillRect		pfnWriteFillRect;
	vAdafruitGfx_writeFastVLine		pfnWriteFastVLine;
	vAdafruitGfx_writeFastHLine		pfnWriteFastHLine;
	vAdafruitGfx_writeLine			pfnWriteLine;
	vAdafruitGfx_endWrite			pfnEndWrite;
	vAdafruitGfx_setRotation		pfnSetRotation;
	vAdafruitGfx_invertDisplay		pfnInvertDisplay;
	vAdafruitGfx_drawFastVLine		pfnDrawFastVLine;
	vAdafruitGfx_drawFastHLine		pfnDrawFastHLine;
	vAdafruitGfx_fillRect			pfnFillRect;
	vAdafruitGfx_fillScreen			pfnFillScreen;
	vAdafruitGfx_drawLine			pfnDrawLine;
	vAdafruitGfx_drawRect			pfnDrawRect;
	vAdafruitGfx_write				pfnWrite;
};

// public non-virtual methods
void AdafruitGfx_drawCircle(AdafruitGfx *_this, int16_t x0, int16_t y0, int16_t r, uint16_t color);
void AdafruitGfx_drawCircleHelper(AdafruitGfx *_this, int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color);
void AdafruitGfx_fillCircle(AdafruitGfx *_this, int16_t x0, int16_t y0, int16_t r, uint16_t color);
void AdafruitGfx_fillCircleHelper(AdafruitGfx *_this, int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color);
void AdafruitGfx_drawTriangle(AdafruitGfx *_this, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void AdafruitGfx_fillTriangle(AdafruitGfx *_this, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void AdafruitGfx_drawRoundRect(AdafruitGfx *_this, int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);
void AdafruitGfx_fillRoundRect(AdafruitGfx *_this, int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);
void AdafruitGfx_drawBitmap(AdafruitGfx *_this, int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color);
void AdafruitGfx_drawBitmapWithBg(AdafruitGfx *_this, int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color, uint16_t bg);
void AdafruitGfx_drawXBitmap(AdafruitGfx *_this, int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color);
void AdafruitGfx_drawGrayscaleBitmap(AdafruitGfx *_this, int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h);
void AdafruitGfx_drawGrayscaleBitmapWithMask(AdafruitGfx *_this, int16_t x, int16_t y, const uint8_t bitmap[], const uint8_t mask[], int16_t w, int16_t h);
void AdafruitGfx_drawRGBBitmap(AdafruitGfx *_this, int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h);
void AdafruitGfx_drawRGBBitmapWithMask(AdafruitGfx *_this, int16_t x, int16_t y, const uint16_t bitmap[], const uint8_t mask[], int16_t w, int16_t h);
void AdafruitGfx_drawChar(AdafruitGfx *_this, int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);
void AdafruitGfx_setCursor(AdafruitGfx *_this, int16_t x, int16_t y);
void AdafruitGfx_setTextColor(AdafruitGfx *_this, uint16_t c);
void AdafruitGfx_setTextColorWithBg(AdafruitGfx *_this, uint16_t c, uint16_t bg);
void AdafruitGfx_setTextSize(AdafruitGfx *_this, uint8_t s);
void AdafruitGfx_setTextWrap(AdafruitGfx *_this, bool w);
void AdafruitGfx_cp437(AdafruitGfx *_this, bool x);
void AdafruitGfx_setFont(AdafruitGfx *_this, const GFXfont *f);
void AdafruitGfx_getTextBounds(AdafruitGfx *_this, const char *s, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);

int16_t AdafruitGfx_height(const AdafruitGfx *_this);
int16_t AdafruitGfx_width(const AdafruitGfx *_this);
uint8_t AdafruitGfx_getRotation(const AdafruitGfx *_this);
int16_t AdafruitGfx_getCursorX(const AdafruitGfx *_this);
int16_t AdafruitGfx_getCursorY(const AdafruitGfx *_this);

// protected methods
void AdafruitGfx_charBounds(AdafruitGfx *_this, char c, int16_t *x, int16_t *y, int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy);

typedef struct TAdafruitGfx 
{
	const struct AdafruitGfx_VMT *vt;

	int16_t m_WIDTH, m_HEIGHT;   // This is the 'raw' display w/h - never changes
	int16_t m_width, m_height; // Display w/h as modified by current rotation
	int16_t m_cursor_x, m_cursor_y;
	uint16_t m_textcolor, m_textbgcolor;
	uint8_t m_textsize, m_rotation;
	bool m_wrap;   // If set, 'wrap' text at right edge of display
	bool m_cp437; // If set, use correct CP437 charset (default is off)
	GFXfont *m_gfxFont;
} AdafruitGfx;

#endif // _ADAFRUIT_GFX_H

/*
This is the core graphics library for all our displays, providing a common
set of graphics primitives (points, lines, circles, etc.).  It needs to be
paired with a hardware-specific library for each display device we carry
(to handle the lower-level functions).

Adafruit invests time and resources providing this open source code, please
support Adafruit & open-source hardware by purchasing products from Adafruit!

Copyright (c) 2013 Adafruit Industries.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
 */

#include "asf.h"
#include "Adafruit_GFX.h"
#include "glcdfont.inl"

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif

const struct AdafruitGfx_VMT AdafruitGfx_VMT_Instance;

void AdafruitGfx_constructor(AdafruitGfx *_this, int16_t w, int16_t h)
{
	_this->vt			= &AdafruitGfx_VMT_Instance;
	_this->m_WIDTH		= w;
	_this->m_HEIGHT		= h;
	
    _this->m_width		= w;
    _this->m_height		= h;
    _this->m_rotation	= 0;
    _this->m_cursor_y	= 0;
	_this->m_cursor_x   = 0;
    _this->m_textsize	= 1;
    _this->m_textcolor	= 0xFFFF;
	_this->m_textbgcolor= 0xFFFF;
    _this->m_wrap		= true;
    _this->m_cp437		= false;
    _this->m_gfxFont	= NULL;
}
// ****************************************************************************
// *********            public non-virtual methods              ***************
// ****************************************************************************

// Draw a circle outline
void AdafruitGfx_drawCircle(AdafruitGfx *_this, int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    _this->vt->pfnStartWrite(_this);
    _this->vt->pfnWritePixel(_this, x0  , y0+r, color);
    _this->vt->pfnWritePixel(_this, x0  , y0-r, color);
    _this->vt->pfnWritePixel(_this, x0+r, y0  , color);
    _this->vt->pfnWritePixel(_this, x0-r, y0  , color);

    while (x<y) {
	    if (f >= 0) {
		    y--;
		    ddF_y += 2;
		    f += ddF_y;
	    }
	    x++;
	    ddF_x += 2;
	    f += ddF_x;

	    _this->vt->pfnWritePixel(_this, x0 + x, y0 + y, color);
	    _this->vt->pfnWritePixel(_this, x0 - x, y0 + y, color);
	    _this->vt->pfnWritePixel(_this, x0 + x, y0 - y, color);
	    _this->vt->pfnWritePixel(_this, x0 - x, y0 - y, color);
	    _this->vt->pfnWritePixel(_this, x0 + y, y0 + x, color);
	    _this->vt->pfnWritePixel(_this, x0 - y, y0 + x, color);
	    _this->vt->pfnWritePixel(_this, x0 + y, y0 - x, color);
	    _this->vt->pfnWritePixel(_this, x0 - y, y0 - x, color);
    }
    _this->vt->pfnEndWrite(_this);
}

void AdafruitGfx_drawCircleHelper(AdafruitGfx *_this, int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color)
{
    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x<y) {
	    if (f >= 0) {
		    y--;
		    ddF_y += 2;
		    f     += ddF_y;
	    }
	    x++;
	    ddF_x += 2;
	    f     += ddF_x;
	    if (cornername & 0x4) {
		    _this->vt->pfnWritePixel(_this, x0 + x, y0 + y, color);
		    _this->vt->pfnWritePixel(_this, x0 + y, y0 + x, color);
	    }
	    if (cornername & 0x2) {
		    _this->vt->pfnWritePixel(_this, x0 + x, y0 - y, color);
		    _this->vt->pfnWritePixel(_this, x0 + y, y0 - x, color);
	    }
	    if (cornername & 0x8) {
		    _this->vt->pfnWritePixel(_this, x0 - y, y0 + x, color);
		    _this->vt->pfnWritePixel(_this, x0 - x, y0 + y, color);
	    }
	    if (cornername & 0x1) {
		    _this->vt->pfnWritePixel(_this, x0 - y, y0 - x, color);
		    _this->vt->pfnWritePixel(_this, x0 - x, y0 - y, color);
	    }
    }
}

void AdafruitGfx_fillCircle(AdafruitGfx *_this, int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
    _this->vt->pfnStartWrite(_this);
    _this->vt->pfnWriteFastVLine(_this, x0, y0-r, 2*r+1, color);
    AdafruitGfx_fillCircleHelper(_this, x0, y0, r, 3, 0, color);
    _this->vt->pfnEndWrite(_this);
}

// Used to do circles and roundrects
void AdafruitGfx_fillCircleHelper(AdafruitGfx *_this, int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color)
{
    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x<y) {
	    if (f >= 0) {
		    y--;
		    ddF_y += 2;
		    f     += ddF_y;
	    }
	    x++;
	    ddF_x += 2;
	    f     += ddF_x;

	    if (cornername & 0x1) {
		    _this->vt->pfnWriteFastVLine(_this, x0+x, y0-y, 2*y+1+delta, color);
		    _this->vt->pfnWriteFastVLine(_this, x0+y, y0-x, 2*x+1+delta, color);
	    }
	    if (cornername & 0x2) {
		    _this->vt->pfnWriteFastVLine(_this, x0-x, y0-y, 2*y+1+delta, color);
		    _this->vt->pfnWriteFastVLine(_this, x0-y, y0-x, 2*x+1+delta, color);
	    }
    }
}

void AdafruitGfx_drawTriangle(AdafruitGfx *_this, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
    _this->vt->pfnDrawLine(_this, x0, y0, x1, y1, color);
    _this->vt->pfnDrawLine(_this, x1, y1, x2, y2, color);
    _this->vt->pfnDrawLine(_this, x2, y2, x0, y0, color);
}

void AdafruitGfx_fillTriangle(AdafruitGfx *_this, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
    int16_t a, b, y, last;

    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1) {
	    _swap_int16_t(y0, y1); _swap_int16_t(x0, x1);
    }
    if (y1 > y2) {
	    _swap_int16_t(y2, y1); _swap_int16_t(x2, x1);
    }
    if (y0 > y1) {
	    _swap_int16_t(y0, y1); _swap_int16_t(x0, x1);
    }

    _this->vt->pfnStartWrite(_this);
    if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
	    a = b = x0;
	    if(x1 < a)      a = x1;
	    else if(x1 > b) b = x1;
	    if(x2 < a)      a = x2;
	    else if(x2 > b) b = x2;
	    _this->vt->pfnWriteFastHLine(_this, a, y0, b-a+1, color);
	    _this->vt->pfnEndWrite(_this);
	    return;
    }

    int16_t
    dx01 = x1 - x0,
    dy01 = y1 - y0,
    dx02 = x2 - x0,
    dy02 = y2 - y0,
    dx12 = x2 - x1,
    dy12 = y2 - y1;
    int32_t
    sa   = 0,
    sb   = 0;

    // For upper part of triangle, find scanline crossings for segments
    // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
    // is included here (and second loop will be skipped, avoiding a /0
    // error there), otherwise scanline y1 is skipped here and handled
    // in the second loop...which also avoids a /0 error here if y0=y1
    // (flat-topped triangle).
    if(y1 == y2) last = y1;   // Include y1 scanline
    else         last = y1-1; // Skip it

    for(y=y0; y<=last; y++) {
	    a   = x0 + sa / dy01;
	    b   = x0 + sb / dy02;
	    sa += dx01;
	    sb += dx02;
	    // longhand:
	    // a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
	    // b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
	    if(a > b) _swap_int16_t(a,b);
	    _this->vt->pfnWriteFastHLine(_this, a, y, b-a+1, color);
    }

    // For lower part of triangle, find scanline crossings for segments
    // 0-2 and 1-2.  This loop is skipped if y1=y2.
    sa = dx12 * (y - y1);
    sb = dx02 * (y - y0);
    for(; y<=y2; y++) {
	    a   = x1 + sa / dy12;
	    b   = x0 + sb / dy02;
	    sa += dx12;
	    sb += dx02;
	    // longhand:
	    // a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
	    // b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
	    if(a > b) _swap_int16_t(a,b);
	    _this->vt->pfnWriteFastHLine(_this, a, y, b-a+1, color);
    }
    _this->vt->pfnEndWrite(_this);
}

// Draw a rounded rectangle
void AdafruitGfx_drawRoundRect(AdafruitGfx *_this, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color)
{
    // smarter version
    _this->vt->pfnStartWrite(_this);
    _this->vt->pfnWriteFastHLine(_this, x+r  , y    , w-2*r, color); // Top
    _this->vt->pfnWriteFastHLine(_this, x+r  , y+h-1, w-2*r, color); // Bottom
    _this->vt->pfnWriteFastVLine(_this, x    , y+r  , h-2*r, color); // Left
    _this->vt->pfnWriteFastVLine(_this, x+w-1, y+r  , h-2*r, color); // Right
    // draw four corners
    AdafruitGfx_drawCircleHelper(_this, x+r    , y+r    , r, 1, color);
    AdafruitGfx_drawCircleHelper(_this, x+w-r-1, y+r    , r, 2, color);
    AdafruitGfx_drawCircleHelper(_this, x+w-r-1, y+h-r-1, r, 4, color);
    AdafruitGfx_drawCircleHelper(_this, x+r    , y+h-r-1, r, 8, color);
    _this->vt->pfnEndWrite(_this);
}

// Fill a rounded rectangle
void AdafruitGfx_fillRoundRect(AdafruitGfx *_this, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color)
{
    // smarter version
    _this->vt->pfnStartWrite(_this);
    _this->vt->pfnWriteFillRect(_this, x+r, y, w-2*r, h, color);

    // draw four corners
    AdafruitGfx_fillCircleHelper(_this, x+w-r-1, y+r, r, 1, h-2*r-1, color);
    AdafruitGfx_fillCircleHelper(_this, x+r    , y+r, r, 2, h-2*r-1, color);
    _this->vt->pfnEndWrite(_this);
}

void AdafruitGfx_drawBitmap(AdafruitGfx *_this, int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color)
{
    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    _this->vt->pfnStartWrite(_this);
    for(int16_t j=0; j<h; j++, y++) {
	    for(int16_t i=0; i<w; i++ ) {
		    if(i & 7) byte <<= 1;
		    else      byte   = bitmap[j * byteWidth + i / 8];
		    if(byte & 0x80) _this->vt->pfnWritePixel(_this, x+i, y, color);
	    }
    }
    _this->vt->pfnEndWrite(_this);
}

void AdafruitGfx_drawBitmapWithBg(AdafruitGfx *_this, int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color, uint16_t bg)
{
    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    _this->vt->pfnStartWrite(_this);
    for(int16_t j=0; j<h; j++, y++) {
	    for(int16_t i=0; i<w; i++ ) {
		    if(i & 7) byte <<= 1;
		    else      byte   = bitmap[j * byteWidth + i / 8];
		    _this->vt->pfnWritePixel(_this, x+i, y, (byte & 0x80) ? color : bg);
	    }
    }
    _this->vt->pfnEndWrite(_this);
}

// Draw PROGMEM-resident XBitMap Files (*.xbm), exported from GIMP,
// Usage: Export from GIMP to *.xbm, rename *.xbm to *.c and open in editor.
// C Array can be directly used with this function.
// There is no RAM-resident version of this function; if generating bitmaps
// in RAM, use the format defined by drawBitmap() and call that instead.
void AdafruitGfx_drawXBitmap(AdafruitGfx *_this, int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color)
{
    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    _this->vt->pfnStartWrite(_this);
    for(int16_t j=0; j<h; j++, y++) {
	    for(int16_t i=0; i<w; i++ ) {
		    if(i & 7) byte >>= 1;
		    else      byte   = bitmap[j * byteWidth + i / 8];
		    // Nearly identical to drawBitmap(), only the bit order
		    // is reversed here (left-to-right = LSB to MSB):
		    if(byte & 0x01) _this->vt->pfnWritePixel(_this, x+i, y, color);
	    }
    }
    _this->vt->pfnEndWrite(_this);
}

// Draw a PROGMEM-resident 8-bit image (grayscale) at the specified (x,y)
// pos.  Specifically for 8-bit display devices such as IS31FL3731;
// no color reduction/expansion is performed.
void AdafruitGfx_drawGrayscaleBitmap(AdafruitGfx *_this, int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h)
{
    _this->vt->pfnStartWrite(_this);
    for(int16_t j=0; j<h; j++, y++) {
	    for(int16_t i=0; i<w; i++ ) {
		    _this->vt->pfnWritePixel(_this, x+i, y, bitmap[j * w + i]);
	    }
    }
    _this->vt->pfnEndWrite(_this);
}

// Draw a RAM-resident 8-bit image (grayscale) with a 1-bit mask
// (set bits = opaque, unset bits = clear) at the specified (x,y) pos.
// BOTH buffers (grayscale and mask) must be RAM-resident, no mix-and-
// match.  Specifically for 8-bit display devices such as IS31FL3731;
// no color reduction/expansion is performed.
void AdafruitGfx_drawGrayscaleBitmapWithMask(AdafruitGfx *_this, int16_t x, int16_t y, const uint8_t bitmap[], const uint8_t mask[], int16_t w, int16_t h)
{
    int16_t bw   = (w + 7) / 8; // Bitmask scanline pad = whole byte
    uint8_t byte = 0;
    _this->vt->pfnStartWrite(_this);
    for(int16_t j=0; j<h; j++, y++) {
	    for(int16_t i=0; i<w; i++ ) {
		    if(i & 7) byte <<= 1;
		    else      byte   = mask[j * bw + i / 8];
		    if(byte & 0x80) {
			    _this->vt->pfnWritePixel(_this, x+i, y, bitmap[j * w + i]);
		    }
	    }
    }
    _this->vt->pfnEndWrite(_this);
}

// Draw a RAM-resident 16-bit image (RGB 5/6/5) at the specified (x,y)
// position.  For 16-bit display devices; no color reduction performed.
void AdafruitGfx_drawRGBBitmap(AdafruitGfx *_this, int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h)
{
    _this->vt->pfnStartWrite(_this);
    for(int16_t j=0; j<h; j++, y++) {
	    for(int16_t i=0; i<w; i++ ) {
		    _this->vt->pfnWritePixel(_this, x+i, y, bitmap[j * w + i]);
	    }
    }
    _this->vt->pfnEndWrite(_this);
}

// Draw a RAM-resident 16-bit image (RGB 5/6/5) with a 1-bit mask
// (set bits = opaque, unset bits = clear) at the specified (x,y) pos.
// BOTH buffers (color and mask) must be RAM-resident, no mix-and-match.
// For 16-bit display devices; no color reduction performed.
void AdafruitGfx_drawRGBBitmapWithMask(AdafruitGfx *_this, int16_t x, int16_t y, const uint16_t bitmap[], const uint8_t mask[], int16_t w, int16_t h)
{
    int16_t bw   = (w + 7) / 8; // Bitmask scanline pad = whole byte
    uint8_t byte = 0;
    _this->vt->pfnStartWrite(_this);
    for(int16_t j=0; j<h; j++, y++) {
	    for(int16_t i=0; i<w; i++ ) {
		    if(i & 7) byte <<= 1;
		    else      byte   = mask[j * bw + i / 8];
		    if(byte & 0x80) {
			    _this->vt->pfnWritePixel(_this, x+i, y, bitmap[j * w + i]);
		    }
	    }
    }
    _this->vt->pfnEndWrite(_this);
}

void AdafruitGfx_drawChar(AdafruitGfx *_this, int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size)
{
    if(!_this->m_gfxFont) { // 'Classic' built-in font

	    if((x >= _this->m_width)            || // Clip right
	    (y >= _this->m_height)           || // Clip bottom
	    ((x + 6 * size - 1) < 0) || // Clip left
	    ((y + 8 * size - 1) < 0))   // Clip top
	    return;

	    if(!_this->m_cp437 && (c >= 176)) c++; // Handle 'classic' charset behavior

	    _this->vt->pfnStartWrite(_this);
	    for(int8_t i=0; i<5; i++ ) { // Char bitmap = 5 columns
		    uint8_t line = font[c * 5 + i];
		    for(int8_t j=0; j<8; j++, line >>= 1) {
			    if(line & 1) {
				    if(size == 1)
				    _this->vt->pfnWritePixel(_this, x+i, y+j, color);
				    else
				    _this->vt->pfnWriteFillRect(_this, x+i*size, y+j*size, size, size, color);
				    } else if(bg != color) {
				    if(size == 1)
				    _this->vt->pfnWritePixel(_this, x+i, y+j, bg);
				    else
				    _this->vt->pfnWriteFillRect(_this, x+i*size, y+j*size, size, size, bg);
			    }
		    }
	    }
	    if(bg != color) { // If opaque, draw vertical line for last column
		    if(size == 1) _this->vt->pfnWriteFastVLine(_this, x+5, y, 8, bg);
		    else          _this->vt->pfnWriteFillRect(_this, x+5*size, y, size, 8*size, bg);
	    }
	    _this->vt->pfnEndWrite(_this);

	    } else { // Custom font

	    // Character is assumed previously filtered by write() to eliminate
	    // newlines, returns, non-printable characters, etc.  Calling
	    // drawChar() directly with 'bad' characters of font may cause mayhem!

	    c -= _this->m_gfxFont->first;
	    GFXglyph *glyph  = _this->m_gfxFont->glyph + c;
	    uint8_t  *bitmap = _this->m_gfxFont->bitmap;

	    uint16_t bo = glyph->bitmapOffset;
	    uint8_t  w  = glyph->width;
	    uint8_t  h  = glyph->height;
	    int8_t   xo = glyph->xOffset;
	    int8_t	 yo = glyph->yOffset;
	    uint8_t  xx, yy, bits = 0, bit = 0;
	    int16_t  xo16 = 0, yo16 = 0;

	    if(size > 1) {
		    xo16 = xo;
		    yo16 = yo;
	    }

	    // Todo: Add character clipping here

	    // NOTE: THERE IS NO 'BACKGROUND' COLOR OPTION ON CUSTOM FONTS.
	    // THIS IS ON PURPOSE AND BY DESIGN.  The background color feature
	    // has typically been used with the 'classic' font to overwrite old
	    // screen contents with new data.  This ONLY works because the
	    // characters are a uniform size; it's not a sensible thing to do with
	    // proportionally-spaced fonts with glyphs of varying sizes (and that
	    // may overlap).  To replace previously-drawn text when using a custom
	    // font, use the getTextBounds() function to determine the smallest
	    // rectangle encompassing a string, erase the area with fillRect(),
	    // then draw new text.  This WILL unfortunately 'blink' the text, but
	    // is unavoidable.  Drawing 'background' pixels will NOT fix this,
	    // only creates a new set of problems.  Have an idea to work around
	    // this (a canvas object type for MCUs that can afford the RAM and
	    // displays supporting setAddrWindow() and pushColors()), but haven't
	    // implemented this yet.

	    _this->vt->pfnStartWrite(_this);
	    for(yy=0; yy<h; yy++) {
		    for(xx=0; xx<w; xx++) {
			    if(!(bit++ & 7)) {
				    bits = bitmap[bo++];
			    }
			    if(bits & 0x80) {
				    if(size == 1) {
					    _this->vt->pfnWritePixel(_this, x+xo+xx, y+yo+yy, color);
					    } else {
					    _this->vt->pfnWriteFillRect(_this, x+(xo16+xx)*size, y+(yo16+yy)*size,
					    size, size, color);
				    }
			    }
			    bits <<= 1;
		    }
	    }
	    _this->vt->pfnEndWrite(_this);

    } // End classic vs custom font
}

void AdafruitGfx_setCursor(AdafruitGfx *_this, int16_t x, int16_t y)
{
    _this->m_cursor_x = x;
    _this->m_cursor_y = y;
}

void AdafruitGfx_setTextColor(AdafruitGfx *_this, uint16_t c)
{
    // For 'transparent' background, we'll set the bg
    // to the same as fg instead of using a flag
    _this->m_textcolor = _this->m_textbgcolor = c;
}

void AdafruitGfx_setTextColorWithBg(AdafruitGfx *_this, uint16_t c, uint16_t bg)
{
    _this->m_textcolor   = c;
    _this->m_textbgcolor = bg;
}

void AdafruitGfx_setTextSize(AdafruitGfx *_this, uint8_t s)
{
    _this->m_textsize = (s > 0) ? s : 1;
}

void AdafruitGfx_setTextWrap(AdafruitGfx *_this, bool w)
{
    _this->m_wrap = w;
}

void AdafruitGfx_cp437(AdafruitGfx *_this, bool x)
{
    _this->m_cp437 = x;
}

void AdafruitGfx_setFont(AdafruitGfx *_this, const GFXfont *f)
{
    if(f) {            // Font struct pointer passed in?
	    if(!_this->m_gfxFont) { // And no current font struct?
		    // Switching from classic to new font behavior.
		    // Move cursor pos down 6 pixels so it's on baseline.
		    _this->m_cursor_y += 6;
	    }
	    } else if(_this->m_gfxFont) { // NULL passed.  Current font struct defined?
	    // Switching from new to classic font behavior.
	    // Move cursor pos up 6 pixels so it's at top-left of char.
	    _this->m_cursor_y -= 6;
    }
    _this->m_gfxFont = (GFXfont *)f;
}

// Pass string and a cursor position, returns UL corner and W,H.
void AdafruitGfx_getTextBounds(AdafruitGfx *_this, const char *str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
{
    uint8_t c; // Current character

    *x1 = x;
    *y1 = y;
    *w  = *h = 0;

    int16_t minx = _this->m_width, miny = _this->m_height, maxx = -1, maxy = -1;

    while((c = *str++))
    AdafruitGfx_charBounds(_this, c, &x, &y, &minx, &miny, &maxx, &maxy);

    if(maxx >= minx) {
	    *x1 = minx;
	    *w  = maxx - minx + 1;
    }
    if(maxy >= miny) {
	    *y1 = miny;
	    *h  = maxy - miny + 1;
    }
}

int16_t AdafruitGfx_height(const AdafruitGfx *_this)
{
    return _this->m_height;
}

int16_t AdafruitGfx_width(const AdafruitGfx *_this)
{
    return _this->m_width;
}

uint8_t AdafruitGfx_getRotation(const AdafruitGfx *_this)
{
    return _this->m_rotation;
}

int16_t AdafruitGfx_getCursorX(const AdafruitGfx *_this)
{
    return _this->m_cursor_x;
}

int16_t AdafruitGfx_getCursorY(const AdafruitGfx *_this)
{
    return _this->m_cursor_y;
}

// protected methods
void AdafruitGfx_charBounds(AdafruitGfx *_this, char c, int16_t *x, int16_t *y, int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy)
{
    if(_this->m_gfxFont) {

	    if(c == '\n') { // Newline?
		    *x  = 0;    // Reset x to zero, advance y by one line
		    *y += _this->m_textsize * _this->m_gfxFont->yAdvance;
		    } else if(c != '\r') { // Not a carriage return; is normal char
		    uint8_t first = _this->m_gfxFont->first,
		    last  = _this->m_gfxFont->last;
		    if((c >= first) && (c <= last)) { // Char present in this font?
			    GFXglyph *glyph = _this->m_gfxFont->glyph + c - first;
			    uint8_t gw = glyph->width,
			    gh = glyph->height,
			    xa = glyph->xAdvance;
			    int8_t  xo = glyph->xOffset,
			    yo = glyph->yOffset;
			    if(_this->m_wrap && ((*x+(((int16_t)xo+gw)*_this->m_textsize)) > _this->m_width)) {
				    *x  = 0; // Reset x to zero, advance y by one line
				    *y += _this->m_textsize * _this->m_gfxFont->yAdvance;
			    }
			    int16_t ts = (int16_t)_this->m_textsize,
			    x1 = *x + xo * ts,
			    y1 = *y + yo * ts,
			    x2 = x1 + gw * ts - 1,
			    y2 = y1 + gh * ts - 1;
			    if(x1 < *minx) *minx = x1;
			    if(y1 < *miny) *miny = y1;
			    if(x2 > *maxx) *maxx = x2;
			    if(y2 > *maxy) *maxy = y2;
			    *x += xa * ts;
		    }
	    }

	    } else { // Default font

	    if(c == '\n') {                     // Newline?
		    *x  = 0;                        // Reset x to zero,
		    *y += _this->m_textsize * 8;             // advance y one line
		    // min/max x/y unchaged -- that waits for next 'normal' character
		    } else if(c != '\r') {  // Normal char; ignore carriage returns
		    if(_this->m_wrap && ((*x + _this->m_textsize * 6) > _this->m_width)) { // Off right?
			    *x  = 0;                    // Reset x to zero,
			    *y += _this->m_textsize * 8;         // advance y one line
		    }
		    int x2 = *x + _this->m_textsize * 6 - 1, // Lower-right pixel of char
		    y2 = *y + _this->m_textsize * 8 - 1;
		    if(x2 > *maxx) *maxx = x2;      // Track max x, y
		    if(y2 > *maxy) *maxy = y2;
		    if(*x < *minx) *minx = *x;      // Track min x, y
		    if(*y < *miny) *miny = *y;
		    *x += _this->m_textsize * 6;             // Advance x one char
	    }
    }
}

// ****************************************************************************
// ****************            Virtual methods               ******************
// ****************************************************************************

void AdafruitGfx_destructor(AdafruitGfx *_this)
{
}

void AdafruitGfx_startWrite(AdafruitGfx *_this)
{
	// Overwrite in subclasses if desired!
}

void AdafruitGfx_writePixel(AdafruitGfx *_this, int16_t x, int16_t y, uint16_t color)
{
	// Overwrite in subclasses if startWrite is defined!
	_this->vt->pfnDrawPixel(_this, x, y, color);
}

void AdafruitGfx_writeFillRect(AdafruitGfx *_this, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    // Overwrite in subclasses if desired!
    _this->vt->pfnFillRect(_this, x,y,w,h,color);
}

// (x,y) is topmost point; if unsure, calling function
// should sort endpoints or call writeLine() instead
void AdafruitGfx_writeFastVLine(AdafruitGfx *_this, int16_t x, int16_t y, int16_t h, uint16_t color)
{
    // Overwrite in subclasses if startWrite is defined!
    // Can be just writeLine(x, y, x, y+h-1, color);
    // or writeFillRect(x, y, 1, h, color);
    _this->vt->pfnDrawFastVLine(_this, x, y, h, color);
}

// (x,y) is leftmost point; if unsure, calling function
// should sort endpoints or call writeLine() instead
void AdafruitGfx_writeFastHLine(AdafruitGfx *_this, int16_t x, int16_t y, int16_t w, uint16_t color)
{
    // Overwrite in subclasses if startWrite is defined!
    // Example: writeLine(x, y, x+w-1, y, color);
    // or writeFillRect(x, y, w, 1, color);
    _this->vt->pfnDrawFastHLine(_this, x, y, w, color);
}

// Bresenham's algorithm - thx wikpedia
void AdafruitGfx_writeLine(AdafruitGfx *_this, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
	    _swap_int16_t(x0, y0);
	    _swap_int16_t(x1, y1);
    }

    if (x0 > x1) {
	    _swap_int16_t(x0, x1);
	    _swap_int16_t(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
	    ystep = 1;
	    } else {
	    ystep = -1;
    }

    for (; x0<=x1; x0++) {
	    if (steep) {
		    _this->vt->pfnWritePixel(_this, y0, x0, color);
		    } else {
		    _this->vt->pfnWritePixel(_this, x0, y0, color);
	    }
	    err -= dy;
	    if (err < 0) {
		    y0 += ystep;
		    err += dx;
	    }
    }
}

void AdafruitGfx_endWrite(AdafruitGfx *_this)
{
    // Overwrite in subclasses if startWrite is defined!
}

void AdafruitGfx_setRotation(AdafruitGfx *_this, uint8_t x)
{
    _this->m_rotation = (x & 3);
    switch(_this->m_rotation) {
	    case 0:
	    case 2:
	    _this->m_width  = _this->m_WIDTH;
	    _this->m_height = _this->m_HEIGHT;
	    break;
	    case 1:
	    case 3:
	    _this->m_width  = _this->m_HEIGHT;
	    _this->m_height = _this->m_WIDTH;
	    break;
    }
}

void AdafruitGfx_invertDisplay(AdafruitGfx *_this, bool i)
{
    // Do nothing, must be subclassed if supported by hardware
}

// (x,y) is topmost point; if unsure, calling function
// should sort endpoints or call drawLine() instead
void AdafruitGfx_drawFastVLine(AdafruitGfx *_this, int16_t x, int16_t y, int16_t h, uint16_t color)
{
    // Update in subclasses if desired!
    _this->vt->pfnStartWrite(_this);
    _this->vt->pfnWriteLine(_this, x, y, x, y+h-1, color);
    _this->vt->pfnEndWrite(_this);
}

// (x,y) is leftmost point; if unsure, calling function
// should sort endpoints or call drawLine() instead
void AdafruitGfx_drawFastHLine(AdafruitGfx *_this, int16_t x, int16_t y, int16_t w, uint16_t color)
{
    // Update in subclasses if desired!
    _this->vt->pfnStartWrite(_this);
    _this->vt->pfnWriteLine(_this, x, y, x+w-1, y, color);
    _this->vt->pfnEndWrite(_this);
}

void AdafruitGfx_fillRect(AdafruitGfx *_this, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    // Update in subclasses if desired!
    _this->vt->pfnStartWrite(_this);
    for (int16_t i=x; i<x+w; i++) {
	    _this->vt->pfnWriteFastVLine(_this, i, y, h, color);
    }
    _this->vt->pfnEndWrite(_this);
}

void AdafruitGfx_fillScreen(AdafruitGfx *_this, uint16_t color)
{
    // Update in subclasses if desired!
    _this->vt->pfnFillRect(_this, 0, 0, _this->m_width, _this->m_height, color);
}

void AdafruitGfx_drawLine(AdafruitGfx *_this, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    // Update in subclasses if desired!
    if(x0 == x1){
	    if(y0 > y1) _swap_int16_t(y0, y1);
	    _this->vt->pfnDrawFastVLine(_this, x0, y0, y1 - y0 + 1, color);
	    } else if(y0 == y1){
	    if(x0 > x1) _swap_int16_t(x0, x1);
	    _this->vt->pfnDrawFastHLine(_this, x0, y0, x1 - x0 + 1, color);
	    } else {
	    _this->vt->pfnStartWrite(_this);
	    _this->vt->pfnWriteLine(_this, x0, y0, x1, y1, color);
	    _this->vt->pfnEndWrite(_this);
    }
}

void AdafruitGfx_drawRect(AdafruitGfx *_this, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    _this->vt->pfnStartWrite(_this);
    _this->vt->pfnWriteFastHLine(_this, x, y, w, color);
    _this->vt->pfnWriteFastHLine(_this, x, y+h-1, w, color);
    _this->vt->pfnWriteFastVLine(_this, x, y, h, color);
    _this->vt->pfnWriteFastVLine(_this, x+w-1, y, h, color);
    _this->vt->pfnEndWrite(_this);
}

size_t AdafruitGfx_write(AdafruitGfx *_this, uint8_t c)
{
    if(!_this->m_gfxFont) { // 'Classic' built-in font

	    if(c == '\n') {                        // Newline?
		    _this->m_cursor_x  = 0;                     // Reset x to zero,
		    _this->m_cursor_y += _this->m_textsize * 8;          // advance y one line
		    } else if(c != '\r') {                 // Ignore carriage returns
		    if(_this->m_wrap && ((_this->m_cursor_x + _this->m_textsize * 6) > _this->m_width)) { // Off right?
			    _this->m_cursor_x  = 0;                 // Reset x to zero,
			    _this->m_cursor_y += _this->m_textsize * 8;      // advance y one line
		    }
		    AdafruitGfx_drawChar(_this, _this->m_cursor_x, _this->m_cursor_y, c, _this->m_textcolor, _this->m_textbgcolor, _this->m_textsize);
		    _this->m_cursor_x += _this->m_textsize * 6;          // Advance x one char
	    }

	    } else { // Custom font

	    if(c == '\n') {
		    _this->m_cursor_x  = 0;
		    _this->m_cursor_y += (int16_t)_this->m_textsize * _this->m_gfxFont->yAdvance;
		    } else if(c != '\r') {
		    uint8_t first = _this->m_gfxFont->first;
		    if((c >= first) && (c <= _this->m_gfxFont->last)) {
			    GFXglyph *glyph = _this->m_gfxFont->glyph + c - first;
			    uint8_t   w     = glyph->width,
			    h     = glyph->height;
			    if((w > 0) && (h > 0)) { // Is there an associated bitmap?
				    int16_t xo = glyph->xOffset; // sic
				    if(_this->m_wrap && ((_this->m_cursor_x + _this->m_textsize * (xo + w)) > _this->m_width)) {
					    _this->m_cursor_x  = 0;
					    _this->m_cursor_y += (int16_t)_this->m_textsize * _this->m_gfxFont->yAdvance;
				    }
				    AdafruitGfx_drawChar(_this, _this->m_cursor_x, _this->m_cursor_y, c, _this->m_textcolor, _this->m_textbgcolor, _this->m_textsize);
			    }
			    _this->m_cursor_x += glyph->xAdvance * (int16_t)_this->m_textsize;
		    }
	    }

    }
    return 1;
}

const struct AdafruitGfx_VMT AdafruitGfx_VMT_Instance =
{
	pfnDestructor: &AdafruitGfx_destructor,
	pfnDrawPixel: NULL, 
	pfnStartWrite: &AdafruitGfx_startWrite, 
	pfnWritePixel: &AdafruitGfx_writePixel, 
	pfnWriteFillRect: &AdafruitGfx_writeFillRect, 
	pfnWriteFastVLine: &AdafruitGfx_writeFastVLine, 
	pfnWriteFastHLine: &AdafruitGfx_writeFastHLine, 
	pfnWriteLine: &AdafruitGfx_writeLine, 
	pfnEndWrite: &AdafruitGfx_endWrite, 
	pfnSetRotation: &AdafruitGfx_setRotation, 
	pfnInvertDisplay: &AdafruitGfx_invertDisplay, 
	pfnDrawFastVLine: &AdafruitGfx_drawFastVLine, 
	pfnDrawFastHLine: &AdafruitGfx_drawFastHLine, 
	pfnFillRect: &AdafruitGfx_fillRect, 
	pfnFillScreen: &AdafruitGfx_fillScreen, 
	pfnDrawLine: &AdafruitGfx_drawLine, 
	pfnDrawRect: &AdafruitGfx_drawRect, 
	pfnWrite: &AdafruitGfx_write, 
};

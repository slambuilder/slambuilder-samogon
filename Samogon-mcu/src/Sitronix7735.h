#ifndef SITRONIX7735_H_
#define SITRONIX7735_H_

#include "asf.h"
#include "AdafruitGfx/Adafruit_GFX.h"

#define DELAY 0x80

#define ST7735_RGB(r, g, b) ((r & 0x1f) | ((g & 0x3f) << 5) | ((b & 0x1f) << 11))

typedef struct TSitronix7735
{
	// base has to be the first field. This will work similar to C++ inheritance.
	AdafruitGfx m_base;

	// PWM enabled light control. 1 - full brightness, 0 - no light.
	uint8_t m_litePin;
	// High - display data or parameter, low - command data
	uint8_t m_dcSelectorPin;
	// Active low.
	uint8_t m_resetPin;

	struct spi_module spiModule;
	struct spi_slave_inst spiSlave;
	struct spi_slave_inst spiCardSlave;

	uint16_t m_colstart;
	uint16_t m_rowstart;
	uint16_t m_xstart;
	uint16_t m_ystart;

} Sitronix7735;

// non-virtual instance method
void Sitronix7735_Constructor(Sitronix7735 *_this, int16_t w, int16_t h);
void Sitronix7735_Initialize(
	Sitronix7735 *_this,
	enum spi_signal_mux_setting m_spiMuxSetting,
	uint32_t m_pinmux_pad0,
	uint32_t m_pinmux_pad1,
	uint32_t m_pinmux_pad2,
	uint32_t m_pinmux_pad3,
	Sercom const *m_sercomHw,
	uint32_t baudrate,
	uint8_t litePin,
	uint8_t chipSelectPin,
	uint8_t cardChipSelectPin,
	uint8_t dcSelectorPin,
	uint8_t resetPin);
void Sitronix7735_Reset(Sitronix7735 *_this);
void Sitronix7735_LightOn(Sitronix7735 *_this);
void Sitronix7735_LightOff(Sitronix7735 *_this);

// returns number of characters drawn
int Sitronix7735_Text(
	Sitronix7735 *_this, 
	const char *str, 
	int16_t x, 
	int16_t y, 
	uint16_t color, 
	uint16_t bgColor, 
	bool clearBackground);

// virtual methods, overrides for AdafruitGfx library
void Sitronix7735_Destructor(Sitronix7735 *_this);
void Sitronix7735_fillScreen(Sitronix7735 *_this, uint16_t color);
void Sitronix7735_drawPixel(Sitronix7735 *_this, int16_t x, int16_t y, uint16_t color);
void Sitronix7735_drawFastVLine(Sitronix7735 *_this, int16_t x, int16_t y, int16_t h, uint16_t color);
void Sitronix7735_drawFastHLine(Sitronix7735 *_this, int16_t x, int16_t y, int16_t w, uint16_t color);
void Sitronix7735_fillRect(Sitronix7735 *_this, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void Sitronix7735_setRotation(Sitronix7735 *_this, uint8_t r);
void Sitronix7735_invertDisplay(Sitronix7735 *_this, bool i);

#endif /* SITRONIX7735_H_ */
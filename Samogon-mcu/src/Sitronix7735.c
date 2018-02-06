#include "Sitronix7735.h"

struct AdafruitGfx_VMT g_Sitronix7735_VMT = {NULL};

// ST7735 commands
#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13

#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E

#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5

#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD

#define ST7735_PWCTR6  0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

static const uint8_t gf_TftInitCommands_Part1[] = {                 // Init for 7735R, part 1 (red or green tab)
	15,                         // 15 commands in list:
	ST7735_SWRESET,   DELAY,	//  1: Software reset, 0 args, w/delay
	150,						//     150 ms delay
	ST7735_SLPOUT ,   DELAY,	//  2: Out of sleep mode, 0 args, w/delay
	255,						//     500 ms delay
	ST7735_FRMCTR1, 3      ,	//  3: Frame rate ctrl - normal mode, 3 args:
	0x01, 0x2C, 0x2D,			//     Rate = fosc/(1x2+40) * (LINE+2C+2D)
	ST7735_FRMCTR2, 3      ,	//  4: Frame rate control - idle mode, 3 args:
	0x01, 0x2C, 0x2D,			//     Rate = fosc/(1x2+40) * (LINE+2C+2D)
	ST7735_FRMCTR3, 6      ,	//  5: Frame rate ctrl - partial mode, 6 args:
	0x01, 0x2C, 0x2D,			//     Dot inversion mode
	0x01, 0x2C, 0x2D,			//     Line inversion mode
	ST7735_INVCTR , 1      ,	//  6: Display inversion ctrl, 1 arg, no delay:
	0x07,						//     No inversion
	ST7735_PWCTR1 , 3      ,	//  7: Power control, 3 args, no delay:
	0xA2,
	0x02,						//     -4.6V
	0x84,						//     AUTO mode
	ST7735_PWCTR2 , 1      ,	//  8: Power control, 1 arg, no delay:
	0xC5,						//     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
	ST7735_PWCTR3 , 2      ,	//  9: Power control, 2 args, no delay:
	0x0A,						//     Opamp current small
	0x00,						//     Boost frequency
	ST7735_PWCTR4 , 2      ,	// 10: Power control, 2 args, no delay:
	0x8A,						//     BCLK/2, Opamp current small & Medium low
	0x2A,
	ST7735_PWCTR5 , 2      ,	// 11: Power control, 2 args, no delay:
	0x8A, 0xEE,
	ST7735_VMCTR1 , 1      ,	// 12: Power control, 1 arg, no delay:
	0x0E,
	ST7735_INVOFF , 0      ,	// 13: Don't invert display, no args, no delay
	ST7735_MADCTL , 1      ,	// 14: Memory access control (directions), 1 arg:
	0xC8,						//     row addr/col addr, bottom to top refresh
	ST7735_COLMOD , 1      ,	// 15: set color mode, 1 arg, no delay:
	0x05						//     16-bit color
};

static const uint8_t gf_TftInitCommands_Part2[] = {            // Init for 7735R, part 2 (green tab only)
	2,							//  2 commands in list:
	ST7735_CASET  , 4      ,	//  1: Column addr set, 4 args, no delay:
	0x00, 0x02,					//     XSTART = 0
	0x00, 0x7F+0x02,			//     XEND = 127
	ST7735_RASET  , 4      ,	//  2: Row addr set, 4 args, no delay:
	0x00, 0x01,					//     XSTART = 0
	0x00, 0x9F+0x01				//     XEND = 159
};

static const uint8_t gf_TftInitCommands_Part3[] = {                 // Init for 7735R, part 3 (red or green tab)
	4,							//  4 commands in list:
	ST7735_GMCTRP1, 16      ,	//  1: Magical unicorn dust, 16 args, no delay:
	0x02, 0x1c, 0x07, 0x12,
	0x37, 0x32, 0x29, 0x2d,
	0x29, 0x25, 0x2B, 0x39,
	0x00, 0x01, 0x03, 0x10,
	ST7735_GMCTRN1, 16      ,	//  2: Sparkles and rainbows, 16 args, no delay:
	0x03, 0x1d, 0x07, 0x06,
	0x2E, 0x2C, 0x29, 0x2D,
	0x2E, 0x2E, 0x37, 0x3F,
	0x00, 0x00, 0x02, 0x10,
	ST7735_NORON  ,    DELAY,	//  3: Normal display on, no args, w/delay
	10,							//     10 ms delay
	ST7735_DISPON ,    DELAY,	//  4: Main screen turn on, no args w/delay
	100							//     100 ms delay
};

// *****************************************************************
// *********  private methods     **********************************
// *****************************************************************

void Sitronix7735_writeCommand(Sitronix7735 *_this, uint8_t cmd)
{
	port_pin_set_output_level(_this->m_dcSelectorPin, false);
	spi_write_buffer_wait(&_this->spiModule, &cmd, 1);
}

void Sitronix7735_writeData(Sitronix7735 *_this, const uint8_t *pData, size_t size)
{
	if (pData && size > 0)
	{
		port_pin_set_output_level(_this->m_dcSelectorPin, true);
		spi_write_buffer_wait(&_this->spiModule, pData, size);
	}
}

void Sitronix7735_writeCommands(Sitronix7735 *_this, const uint8_t *pCmdBuffer, size_t cmdBufferSize)
{
	if (!pCmdBuffer || cmdBufferSize <= 0)
	{
		return;
	}

	const uint8_t *p = pCmdBuffer;
	uint8_t cmdCount = *p++;
	const uint8_t *pEnd = pCmdBuffer + cmdBufferSize;
	for (uint8_t i = 0; i < cmdCount && p < pEnd; i++)
	{
		uint8_t cmd = *p++;
		uint8_t paramCount = *p++;
		bool delayed = (paramCount & DELAY) != 0;
		paramCount &= ~DELAY;

		spi_select_slave(&_this->spiModule, &_this->spiSlave, true);
		Sitronix7735_writeCommand(_this, cmd);
		Sitronix7735_writeData(_this, p, paramCount);
		spi_select_slave(&_this->spiModule, &_this->spiSlave, false);

		p += paramCount;

		if (delayed)
		{
			uint8_t ms = *p++;
			delay_ms((ms == 255) ? 500 : ms);
		}
	}
}

void Sitronix7735_setAddrWindow(Sitronix7735 *_this, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	x0 += _this->m_xstart;
	y0 += _this->m_ystart;
	x1 += _this->m_xstart;
	y1 += _this->m_ystart;

	spi_select_slave(&_this->spiModule, &_this->spiSlave, true);
	Sitronix7735_writeCommand(_this, ST7735_CASET);
	uint8_t columnAddressData[4] = {(x0 >> 8) & 0xff, x0 & 0xff, (x1 >> 8) & 0xff, x1 & 0xff};
	Sitronix7735_writeData(_this, columnAddressData, sizeof(columnAddressData));
	spi_select_slave(&_this->spiModule, &_this->spiSlave, false);

	spi_select_slave(&_this->spiModule, &_this->spiSlave, true);
	Sitronix7735_writeCommand(_this, ST7735_RASET);
	uint8_t rowAddressData[4] = {(y0 >> 8) & 0xff, y0  & 0xff, (y1 >> 8) & 0xff, y1 & 0xff};
	Sitronix7735_writeData(_this, rowAddressData, sizeof(rowAddressData));
	spi_select_slave(&_this->spiModule, &_this->spiSlave, false);

	spi_select_slave(&_this->spiModule, &_this->spiSlave, true);
	Sitronix7735_writeCommand(_this, ST7735_RAMWR);
}


// *****************************************************************
// *********  public methods     ***********************************
// *****************************************************************


// non-virtual instance method
void Sitronix7735_Constructor(Sitronix7735 *_this, int16_t w, int16_t h)
{
	// call base constructor first
	AdafruitGfx_constructor(&_this->m_base, w, h);

	// adjust VMT
	if (!g_Sitronix7735_VMT.pfnDestructor) 
	{
		memcpy(&g_Sitronix7735_VMT, _this->m_base.vt, sizeof(g_Sitronix7735_VMT));
		g_Sitronix7735_VMT.pfnDestructor = (vAdafruitGfx_destructor)Sitronix7735_Destructor;
		g_Sitronix7735_VMT.pfnFillScreen = (vAdafruitGfx_fillScreen)Sitronix7735_fillScreen;
		g_Sitronix7735_VMT.pfnDrawPixel = (vAdafruitGfx_drawPixel)Sitronix7735_drawPixel;
		g_Sitronix7735_VMT.pfnDrawFastVLine = (vAdafruitGfx_drawFastVLine)Sitronix7735_drawFastVLine;
		g_Sitronix7735_VMT.pfnDrawFastHLine = (vAdafruitGfx_drawFastHLine)Sitronix7735_drawFastHLine;
		g_Sitronix7735_VMT.pfnFillRect = (vAdafruitGfx_fillRect)Sitronix7735_fillRect;
		g_Sitronix7735_VMT.pfnSetRotation = (vAdafruitGfx_setRotation)Sitronix7735_setRotation;
		g_Sitronix7735_VMT.pfnInvertDisplay = (vAdafruitGfx_invertDisplay)Sitronix7735_invertDisplay;
	}
	_this->m_base.vt = &g_Sitronix7735_VMT;

	_this->m_xstart		= 0;
	_this->m_ystart		= 0;
	_this->m_colstart	= 0;
	_this->m_rowstart	= 0;
}

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
	uint8_t resetPin)
{
	_this->m_litePin = litePin;
	_this->m_dcSelectorPin = dcSelectorPin;
	_this->m_resetPin = resetPin;

	struct port_config pin_config;
	port_get_config_defaults(&pin_config);
	pin_config.direction = PORT_PIN_DIR_OUTPUT;

	// set RST pin to high
	port_pin_set_config(resetPin, &pin_config);
	port_pin_set_output_level(resetPin, true);

	// set LITE pin to high
	port_pin_set_config(litePin, &pin_config);
	port_pin_set_output_level(litePin, true);

	// set C/D pin to high (data)
	port_pin_set_config(dcSelectorPin, &pin_config);
	port_pin_set_output_level(dcSelectorPin, true);

	// Configure and initialize software device instance of peripheral slave
	struct spi_slave_inst_config slaveConfig;

	spi_slave_inst_get_config_defaults(&slaveConfig);
	slaveConfig.ss_pin = chipSelectPin;
	spi_attach_slave(&_this->spiSlave, &slaveConfig);

	// Configure and initialize software device instance of peripheral slave SD card device
	struct spi_slave_inst_config cardSlaveConfig;
	spi_slave_inst_get_config_defaults(&cardSlaveConfig);
	slaveConfig.ss_pin = cardChipSelectPin;
	spi_attach_slave(&_this->spiCardSlave, &cardSlaveConfig);

	struct spi_config spiConfig;

	// Configure, initialize and enable SERCOM SPI module
	spi_get_config_defaults(&spiConfig);
	spiConfig.mode_specific.master.baudrate = baudrate;
	spiConfig.mux_setting = CONF_BOARD_TFT_SERCOM_MUX_SETTING;
	spiConfig.pinmux_pad0 = CONF_BOARD_TFT_SERCOM_PAD0;
	spiConfig.pinmux_pad2 = CONF_BOARD_TFT_SERCOM_PAD2;
	spiConfig.pinmux_pad3 = CONF_BOARD_TFT_SERCOM_PAD3;
	spi_init(&_this->spiModule, CONF_BOARD_TFT_SERCOM, &spiConfig);
	spi_enable(&_this->spiModule);

	Sitronix7735_Reset(_this);

#define ST7735_RGB(r, g, b) ((r & 0x1f) | ((g & 0x3f) << 5) | ((b & 0x1f) << 11))

	Sitronix7735_fillRect(_this,  0,  0, 128, 160, ST7735_RGB(10, 30, 15));

	Sitronix7735_fillRect(_this,  5,  5, 10, 10, ST7735_RGB(0x00, 0x00, 0x00));	// black
	Sitronix7735_fillRect(_this, 15, 15, 10, 10, ST7735_RGB(0x00, 0x00, 0x1f));	// blue
	Sitronix7735_fillRect(_this,  5, 25, 10, 10, ST7735_RGB(0x1f, 0x00, 0x00));	// red
	Sitronix7735_fillRect(_this, 15, 35, 10, 10, ST7735_RGB(0x00, 0x3f, 0x00));	// green
	Sitronix7735_fillRect(_this,  5, 45, 10, 10, ST7735_RGB(0x00, 0x3f, 0x1f));	// cyan
	Sitronix7735_fillRect(_this, 15, 55, 10, 10, ST7735_RGB(0x1f, 0x00, 0x1f));	// magenta
	Sitronix7735_fillRect(_this,  5, 65, 10, 10, ST7735_RGB(0x1f, 0x3f, 0x00));	// yellow
	Sitronix7735_fillRect(_this, 15, 75, 10, 10, ST7735_RGB(0x1f, 0x3f, 0x1f));	// white

	Sitronix7735_fillRect(_this, 55,  5, 10, 10, ST7735_RGB(0x00, 0x00, 0x00));	// black
	Sitronix7735_fillRect(_this, 65, 15, 10, 10, ST7735_RGB(0x00, 0x00, 0x0f));	// blue
	Sitronix7735_fillRect(_this, 55, 25, 10, 10, ST7735_RGB(0x0f, 0x00, 0x00));	// red
	Sitronix7735_fillRect(_this, 65, 35, 10, 10, ST7735_RGB(0x00, 0x1f, 0x00));	// green
	Sitronix7735_fillRect(_this, 55, 45, 10, 10, ST7735_RGB(0x00, 0x1f, 0x0f));	// cyan
	Sitronix7735_fillRect(_this, 65, 55, 10, 10, ST7735_RGB(0x0f, 0x00, 0x0f));	// magenta
	Sitronix7735_fillRect(_this, 55, 65, 10, 10, ST7735_RGB(0x0f, 0x1f, 0x00));	// yellow
	Sitronix7735_fillRect(_this, 65, 75, 10, 10, ST7735_RGB(0x0f, 0x1f, 0x0f));	// white
}

void Sitronix7735_Reset(Sitronix7735 *_this)
{
	// write initialization commands to TFT
	Sitronix7735_writeCommands(_this, gf_TftInitCommands_Part1, sizeof(gf_TftInitCommands_Part1));
	Sitronix7735_writeCommands(_this, gf_TftInitCommands_Part2, sizeof(gf_TftInitCommands_Part2));
	Sitronix7735_writeCommands(_this, gf_TftInitCommands_Part3, sizeof(gf_TftInitCommands_Part3));
}

void Sitronix7735_LightOn(Sitronix7735 *_this)
{
	port_pin_set_output_level(_this->m_litePin, true);
}

void Sitronix7735_LightOff(Sitronix7735 *_this)
{
	port_pin_set_output_level(_this->m_litePin, false);
}

// virtual methods, overrides for AdafruitGfx library
void Sitronix7735_Destructor(Sitronix7735 *_this)
{
	// TODO
}

void Sitronix7735_fillScreen(Sitronix7735 *_this, uint16_t color)
{
	Sitronix7735_fillRect(_this, 0, 0, _this->m_base.m_width, _this->m_base.m_height, color);
}

void Sitronix7735_drawPixel(Sitronix7735 *_this, int16_t x, int16_t y, uint16_t color)
{
	if((x < 0) ||(x >= _this->m_base.m_width) || (y < 0) || (y >= _this->m_base.m_height)) return;

	Sitronix7735_setAddrWindow(_this, x, y, x + 1, y + 1);
	uint8_t colorBuffer[2] = { (color >> 8) & 0xff, color & 0xff };
	Sitronix7735_writeData(_this, colorBuffer, sizeof(colorBuffer));
	spi_select_slave(&_this->spiModule, &_this->spiSlave, false);
}

void Sitronix7735_drawFastVLine(Sitronix7735 *_this, int16_t x, int16_t y, int16_t h, uint16_t color)
{
	if((x >= _this->m_base.m_width) || (y >= _this->m_base.m_height)) return;
	if((y+h-1) >= _this->m_base.m_height) h = _this->m_base.m_height-y;
	Sitronix7735_fillRect(_this, x, y, 1, h, color);
}

void Sitronix7735_drawFastHLine(Sitronix7735 *_this, int16_t x, int16_t y, int16_t w, uint16_t color)
{
  if((x >= _this->m_base.m_width) || (y >= _this->m_base.m_height)) return;
  if((x+w-1) >= _this->m_base.m_width)  w = _this->m_base.m_width-x;
	Sitronix7735_fillRect(_this, x, y, w, 1, color);
}

void Sitronix7735_fillRect(Sitronix7735 *_this, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	// rudimentary clipping (drawChar w/big text requires this)
	if((x >= _this->m_base.m_width) || (y >= _this->m_base.m_height)) return;
	if((x + w - 1) >= _this->m_base.m_width)  w = _this->m_base.m_width  - x;
	if((y + h - 1) >= _this->m_base.m_height) h = _this->m_base.m_height - y;

	Sitronix7735_setAddrWindow(_this, x, y, x + w - 1, y + h - 1);

	uint8_t colorBuffer[64];
	const int bufferElementCount = sizeof(colorBuffer) >> 1;
	uint8_t *p = colorBuffer;
	for (int i = 0; i < bufferElementCount; i++)
	{
		*p++ = color >> 8;
		*p++ = color;
	}

	int elementsToWrite = h * w;
	while (elementsToWrite > 0)
	{
		int cnt = min(elementsToWrite, bufferElementCount);
		Sitronix7735_writeData(_this, colorBuffer, cnt * 2);
		elementsToWrite -= cnt;
	}

	spi_select_slave(&_this->spiModule, &_this->spiSlave, false);
}

#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

void Sitronix7735_setRotation(Sitronix7735 *_this, uint8_t m)
{
	uint8_t buff[4] = {
		1,					// single command in the buffer
		ST7735_MADCTL, 1,	// Memory data read/write direction command with 1 parameter
		MADCTL_BGR
	};

	_this->m_base.m_rotation = m % 4; // can't be higher than 3
	switch (_this->m_base.m_rotation) {
		case 0:
			buff[3] = MADCTL_MX | MADCTL_MY | MADCTL_BGR;
			_this->m_base.m_height = _this->m_base.m_HEIGHT;
			_this->m_base.m_width  = _this->m_base.m_WIDTH;
			_this->m_xstart = _this->m_colstart;
			_this->m_ystart = _this->m_rowstart;
			break;

	  case 1:
			buff[3] = MADCTL_MY | MADCTL_MV | MADCTL_BGR;

			_this->m_base.m_width = _this->m_base.m_HEIGHT;
			_this->m_base.m_height = _this->m_base.m_WIDTH;
			_this->m_ystart = _this->m_colstart;
			_this->m_xstart = _this->m_rowstart;
			break;
	  case 2:
			_this->m_base.m_height = _this->m_base.m_HEIGHT;
			_this->m_base.m_width  = _this->m_base.m_WIDTH;
			_this->m_xstart = _this->m_colstart;
			_this->m_ystart = _this->m_rowstart;
			break;
	  case 3:
			buff[3] = MADCTL_MX | MADCTL_MV | MADCTL_BGR;

			_this->m_base.m_width = _this->m_base.m_HEIGHT;
			_this->m_base.m_height = _this->m_base.m_WIDTH;
			_this->m_ystart = _this->m_colstart;
			_this->m_xstart = _this->m_rowstart;
			break;
	}

	Sitronix7735_writeCommands(_this, buff, sizeof(buff));
}

void Sitronix7735_invertDisplay(Sitronix7735 *_this, bool i)
{
	uint8_t buff[4] = {
		1,					// single command in the buffer
		i ? ST7735_INVON : ST7735_INVOFF, 0,	// Memory data read/write direction command, no parameters
	};
	Sitronix7735_writeCommands(_this, buff, sizeof(buff));
}

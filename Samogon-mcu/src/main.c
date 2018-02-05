#include "asf.h"
#include "Max31855.h"
#include "utils.h"

// This callback is triggered every ~1ms (1024 Hz)
static void callbackTcc0(struct tcc_module *const module_inst)
{
	// TODO: implement async continuation logic here.
}

static void configureTcc0(struct tcc_module *pTcc)
{
	// configure TCC0 for ~1 ms generation (1024 Hz):
	//	 - source GCLK1 (32K), prescaler 1, period 32, channel 0 trigger 15 (~50% duty cycle)
	struct tcc_config config;
	tcc_get_config_defaults(&config, CONF_BOARD_TCC_MODULE);
	config.counter.clock_source = GCLK_GENERATOR_1;
	config.counter.clock_prescaler = TCC_CLOCK_PRESCALER_DIV1;
	config.counter.period = 31;
	config.compare.wave_generation = TCC_WAVE_GENERATION_SINGLE_SLOPE_PWM;
	config.compare.match[CONF_BOARD_TCC_CHANNEL] = 16;
	config.pins.enable_wave_out_pin[CONF_BOARD_TCC_OUTPUT] = true;
	config.pins.wave_out_pin[CONF_BOARD_TCC_OUTPUT]        = CONF_BOARD_TCC_OUT_PIN;
	config.pins.wave_out_pin_mux[CONF_BOARD_TCC_OUTPUT]    = CONF_BOARD_TCC_OUT_MUX;

	tcc_init(pTcc, CONF_BOARD_TCC_MODULE, &config);
	tcc_enable(pTcc);

	// configure TCC0 callback
	tcc_register_callback(pTcc, callbackTcc0, (enum tcc_callback)CONF_BOARD_TCC_CHANNEL);
	tcc_enable_callback(pTcc, (enum tcc_callback)CONF_BOARD_TCC_CHANNEL);
}

void configureSpiTempSensor(struct spi_module *pSpiModuleTempSensor, struct spi_slave_inst *pSpiSlaveInstance)
{
	struct spi_config spiConfig;
	struct spi_slave_inst_config slaveConfig;

	// Configure and initialize software device instance of peripheral slave
	spi_slave_inst_get_config_defaults(&slaveConfig);
	slaveConfig.ss_pin = CONF_BOARD_MAX31855_SS_OUT_PIN;
	spi_attach_slave(pSpiSlaveInstance, &slaveConfig);

	// Configure, initialize and enable SERCOM SPI module
	spi_get_config_defaults(&spiConfig);
	spiConfig.mode_specific.master.baudrate = 1000000;
	spiConfig.mux_setting = CONF_BOARD_MAX31855_SERCOM_MUX_SETTING;
	spiConfig.pinmux_pad0 = CONF_BOARD_MAX31855_SERCOM_PAD0;
	spiConfig.pinmux_pad1 = CONF_BOARD_MAX31855_SERCOM_PAD1;
	spiConfig.pinmux_pad3 = CONF_BOARD_MAX31855_SERCOM_PAD3;
	spi_init(pSpiModuleTempSensor, CONF_BOARD_MAX31855_SERCOM, &spiConfig);
	spi_enable(pSpiModuleTempSensor);
}

void callbackPushButton(void)
{
	//bool pin_state = port_pin_get_input_level(BUTTON_0_PIN);
	//port_pin_set_output_level(LED_0_PIN, pin_state);
}

void callbackRotaryEncoderChannelA(void)
{
}

void callbackRotaryEncoderChannelB(void)
{
}

uint32_t g_zeroCrossCount = 0;

void callbackZeroCross(void)
{
	g_zeroCrossCount++;
}

void configureInterruptForPin(
	const uint32_t gpioPin, 
	const uint32_t gpioPinMux, 
	const uint8_t channel, 
	const extint_callback_t callback)
{
	struct extint_chan_conf config;
	extint_chan_get_config_defaults(&config);
	config.gpio_pin           = gpioPin;
	config.gpio_pin_mux       = gpioPinMux;
	config.gpio_pin_pull      = EXTINT_PULL_UP;
	config.detection_criteria = EXTINT_DETECT_FALLING;
	extint_chan_set_config(channel, &config);

	extint_register_callback(callback, channel, EXTINT_CALLBACK_TYPE_DETECT);
	extint_chan_enable_callback(channel, EXTINT_CALLBACK_TYPE_DETECT);
}

void configureGpioInterrupts(void)
{
	configureInterruptForPin(
		CONF_BOARD_PUSH_BUTTON_EXTINT_PIN, 
		CONF_BOARD_PUSH_BUTTON_EXTINT_MUX, 
		CONF_BOARD_PUSH_BUTTON_EIC_LINE, 
		callbackPushButton);

	configureInterruptForPin(
		CONF_BOARD_RENCODER_CHANNEL_A_EXTINT_PIN, 
		CONF_BOARD_RENCODER_CHANNEL_A_EXTINT_MUX, 
		CONF_BOARD_RENCODER_CHANNEL_A_EIC_LINE, 
		callbackRotaryEncoderChannelA);

	configureInterruptForPin(
		CONF_BOARD_RENCODER_CHANNEL_B_EXTINT_PIN,
		CONF_BOARD_RENCODER_CHANNEL_B_EXTINT_MUX,
		CONF_BOARD_RENCODER_CHANNEL_B_EIC_LINE,
		callbackRotaryEncoderChannelB);

	configureInterruptForPin(
		CONF_BOARD_ZERO_CROSS_EXTINT_PIN,
		CONF_BOARD_ZERO_CROSS_EXTINT_MUX,
		CONF_BOARD_ZERO_CROSS_EIC_LINE,
		callbackZeroCross);
}

#define DELAY 0x80

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
		150,                    //     150 ms delay
	ST7735_SLPOUT ,   DELAY,	//  2: Out of sleep mode, 0 args, w/delay
		255,                    //     500 ms delay
	ST7735_FRMCTR1, 3      ,	//  3: Frame rate ctrl - normal mode, 3 args:
		0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
	ST7735_FRMCTR2, 3      ,	//  4: Frame rate control - idle mode, 3 args:
		0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
	ST7735_FRMCTR3, 6      ,	//  5: Frame rate ctrl - partial mode, 6 args:
		0x01, 0x2C, 0x2D,       //     Dot inversion mode
		0x01, 0x2C, 0x2D,       //     Line inversion mode
	ST7735_INVCTR , 1      ,	//  6: Display inversion ctrl, 1 arg, no delay:
		0x07,                   //     No inversion
	ST7735_PWCTR1 , 3      ,	//  7: Power control, 3 args, no delay:
		0xA2,
		0x02,                   //     -4.6V
		0x84,                   //     AUTO mode
	ST7735_PWCTR2 , 1      ,	//  8: Power control, 1 arg, no delay:
		0xC5,                   //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
	ST7735_PWCTR3 , 2      ,	//  9: Power control, 2 args, no delay:
		0x0A,                   //     Opamp current small
		0x00,                   //     Boost frequency
	ST7735_PWCTR4 , 2      ,	// 10: Power control, 2 args, no delay:
		0x8A,                   //     BCLK/2, Opamp current small & Medium low
		0x2A,  
	ST7735_PWCTR5 , 2      ,	// 11: Power control, 2 args, no delay:
		0x8A, 0xEE,
	ST7735_VMCTR1 , 1      ,	// 12: Power control, 1 arg, no delay:
		0x0E,
	ST7735_INVOFF , 0      ,	// 13: Don't invert display, no args, no delay
	ST7735_MADCTL , 1      ,	// 14: Memory access control (directions), 1 arg:
		0xC8,                   //     row addr/col addr, bottom to top refresh
	ST7735_COLMOD , 1      ,	// 15: set color mode, 1 arg, no delay:
		0x05                    //     16-bit color
};

static const uint8_t gf_TftInitCommands_Part2[] = {            // Init for 7735R, part 2 (green tab only)
    2,                        //  2 commands in list:
    ST7735_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
      0x00, 0x02,             //     XSTART = 0
      0x00, 0x7F+0x02,        //     XEND = 127
    ST7735_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x01,             //     XSTART = 0
      0x00, 0x9F+0x01         //     XEND = 159
};

static const uint8_t gf_TftInitCommands_Part3[] = {                 // Init for 7735R, part 3 (red or green tab)
    4,                        //  4 commands in list:
    ST7735_GMCTRP1, 16      , //  1: Magical unicorn dust, 16 args, no delay:
      0x02, 0x1c, 0x07, 0x12,
      0x37, 0x32, 0x29, 0x2d,
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
    ST7735_GMCTRN1, 16      , //  2: Sparkles and rainbows, 16 args, no delay:
      0x03, 0x1d, 0x07, 0x06,
      0x2E, 0x2C, 0x29, 0x2D,
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
    ST7735_NORON  ,    DELAY, //  3: Normal display on, no args, w/delay
      10,                     //     10 ms delay
    ST7735_DISPON ,    DELAY, //  4: Main screen turn on, no args w/delay
      100                     //     100 ms delay
};

void tftWriteCommand(
	struct spi_module *pSpiTftModule, 
	uint8_t cmd)
{
	port_pin_set_output_level(CONF_BOARD_TFT_DC_SELECT_OUT_PIN, false);
	spi_write_buffer_wait(pSpiTftModule, &cmd, 1);
}

void tftWriteData(
	struct spi_module *pSpiTftModule, 
	const uint8_t *pData,
	size_t size)
{
	if (pData && size > 0)
	{
		port_pin_set_output_level(CONF_BOARD_TFT_DC_SELECT_OUT_PIN, true);
		spi_write_buffer_wait(pSpiTftModule, pData, size);
	}
}

void tftWriteCommands(
	struct spi_module *pSpiTftModule, 
	struct spi_slave_inst *pSpiTftSlave,
	const uint8_t *pCmdBuffer,
	size_t cmdBufferSize)
{
	if (!pSpiTftModule || !pSpiTftSlave || !pCmdBuffer || cmdBufferSize <= 0)
	{
		return;
	}

	uint8_t *p = pCmdBuffer;
	uint8_t cmdCount = *p++;
	uint8_t *pEnd = pCmdBuffer + cmdBufferSize;
	for (uint8_t i = 0; i < cmdCount && p < pEnd; i++)
	{
		uint8_t cmd = *p++;
		uint8_t paramCount = *p++;
		bool delayed = (paramCount & DELAY) != 0;
		paramCount &= ~DELAY;

		spi_select_slave(pSpiTftModule, pSpiTftSlave, true);
		tftWriteCommand(pSpiTftModule, cmd);
		tftWriteData(pSpiTftModule, p, paramCount);
		spi_select_slave(pSpiTftModule, pSpiTftSlave, false);

		p += paramCount;

		if (delayed)
		{
			uint8_t ms = *p++;
			delay_ms((ms == 255) ? 500 : ms);
		}
	}
}

// fill a rectangle
void tftFillRect(
	struct spi_module *pSpiTftModule,
	struct spi_slave_inst *pSpiTftSlave, 
	int16_t x, 
	int16_t y, 
	int16_t w, 
	int16_t h, 
	uint16_t color) 
{
	uint16_t _width = 128;
	uint16_t _height = 160;

	// rudimentary clipping (drawChar w/big text requires this)
	if((x >= _width) || (y >= _height)) return;
	if((x + w - 1) >= _width)  w = _width  - x;
	if((y + h - 1) >= _height) h = _height - y;

	uint8_t xstart = 0;
	uint8_t ystart = 0;
	uint8_t x0 = x, y0 = y, x1 = x + w - 1, y1 = y + h - 1;

	spi_select_slave(pSpiTftModule, pSpiTftSlave, true);
	tftWriteCommand(pSpiTftModule, ST7735_CASET);
	uint8_t columnAddressData[4] = {0x00, x0 + xstart, 0x00, x1 + xstart};
	tftWriteData(pSpiTftModule, columnAddressData, sizeof(columnAddressData));
	spi_select_slave(pSpiTftModule, pSpiTftSlave, false);

	spi_select_slave(pSpiTftModule, pSpiTftSlave, true);
	tftWriteCommand(pSpiTftModule, ST7735_RASET);
	uint8_t rowAddressData[4] = {0x00, y0 + ystart, 0x00, y1 + ystart};
	tftWriteData(pSpiTftModule, rowAddressData, sizeof(rowAddressData));
	spi_select_slave(pSpiTftModule, pSpiTftSlave, false);

	spi_select_slave(pSpiTftModule, pSpiTftSlave, true);
	tftWriteCommand(pSpiTftModule, ST7735_RAMWR);

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
		tftWriteData(pSpiTftModule, colorBuffer, cnt * 2);
		elementsToWrite -= cnt;
	}

	spi_select_slave(pSpiTftModule, pSpiTftSlave, false);
}

void configureTftDisplayPorts(
	struct spi_module *pSpiTftModule, 
	struct spi_slave_inst *pSpiTftSlave)
{
	struct port_config pin_config;
	port_get_config_defaults(&pin_config);
	pin_config.direction = PORT_PIN_DIR_OUTPUT;

	// set RST pin to high
	port_pin_set_config(CONF_BOARD_TFT_RST_OUT_PIN, &pin_config);
	port_pin_set_output_level(CONF_BOARD_TFT_RST_OUT_PIN, true);

	// set LITE pin to high
	port_pin_set_config(CONF_BOARD_TFT_LITE_OUT_PIN, &pin_config);
	port_pin_set_output_level(CONF_BOARD_TFT_LITE_OUT_PIN, true);

	// set C/D pin to high (data)
	port_pin_set_config(CONF_BOARD_TFT_DC_SELECT_OUT_PIN, &pin_config);
	port_pin_set_output_level(CONF_BOARD_TFT_DC_SELECT_OUT_PIN, true);

	struct spi_config spiConfig;
	struct spi_slave_inst_config slaveConfig;

	// Configure and initialize software device instance of peripheral slave
	spi_slave_inst_get_config_defaults(&slaveConfig);
	slaveConfig.ss_pin = CONF_BOARD_TFT_CS_OUT_PIN;
	spi_attach_slave(pSpiTftSlave, &slaveConfig);

	// Configure, initialize and enable SERCOM SPI module
	spi_get_config_defaults(&spiConfig);
	spiConfig.mode_specific.master.baudrate = 10000000;
	spiConfig.mux_setting = CONF_BOARD_TFT_SERCOM_MUX_SETTING;
	spiConfig.pinmux_pad0 = CONF_BOARD_TFT_SERCOM_PAD0;
	spiConfig.pinmux_pad2 = CONF_BOARD_TFT_SERCOM_PAD2;
	spiConfig.pinmux_pad3 = CONF_BOARD_TFT_SERCOM_PAD3;
	spi_init(pSpiTftModule, CONF_BOARD_TFT_SERCOM, &spiConfig);
	spi_enable(pSpiTftModule);

	// write initialization commands to TFT
	tftWriteCommands(pSpiTftModule, pSpiTftSlave, gf_TftInitCommands_Part1, sizeof(gf_TftInitCommands_Part1));
	tftWriteCommands(pSpiTftModule, pSpiTftSlave, gf_TftInitCommands_Part2, sizeof(gf_TftInitCommands_Part2));
	tftWriteCommands(pSpiTftModule, pSpiTftSlave, gf_TftInitCommands_Part3, sizeof(gf_TftInitCommands_Part3));

// Color definitions
#define	ST7735_BLACK   0x0000
#define	ST7735_BLUE    0x001F
#define	ST7735_RED     0xF800
#define	ST7735_GREEN   0x07E0
#define ST7735_CYAN    0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW  0xFFE0
#define ST7735_WHITE   0xFFFF

#define ST7735_RGB(r, g, b) ((r & 0x1f) | ((g & 0x3f) << 5) | ((b & 0x1f) << 11))

	tftFillRect(pSpiTftModule, pSpiTftSlave,  0,  0, 128, 160, ST7735_RGB(10, 30, 15));

	tftFillRect(pSpiTftModule, pSpiTftSlave,  5,  5, 10, 10, ST7735_RGB(0x00, 0x00, 0x00));	// black
	tftFillRect(pSpiTftModule, pSpiTftSlave, 15, 15, 10, 10, ST7735_RGB(0x00, 0x00, 0x1f));	// blue
	tftFillRect(pSpiTftModule, pSpiTftSlave,  5, 25, 10, 10, ST7735_RGB(0x1f, 0x00, 0x00));	// red
	tftFillRect(pSpiTftModule, pSpiTftSlave, 15, 35, 10, 10, ST7735_RGB(0x00, 0x3f, 0x00));	// green
	tftFillRect(pSpiTftModule, pSpiTftSlave,  5, 45, 10, 10, ST7735_RGB(0x00, 0x3f, 0x1f));	// cyan
	tftFillRect(pSpiTftModule, pSpiTftSlave, 15, 55, 10, 10, ST7735_RGB(0x1f, 0x00, 0x1f));	// magenta
	tftFillRect(pSpiTftModule, pSpiTftSlave,  5, 65, 10, 10, ST7735_RGB(0x1f, 0x3f, 0x00));	// yellow
	tftFillRect(pSpiTftModule, pSpiTftSlave, 15, 75, 10, 10, ST7735_RGB(0x1f, 0x3f, 0x1f));	// white

	tftFillRect(pSpiTftModule, pSpiTftSlave, 55,  5, 10, 10, ST7735_RGB(0x00, 0x00, 0x00));	// black
	tftFillRect(pSpiTftModule, pSpiTftSlave, 65, 15, 10, 10, ST7735_RGB(0x00, 0x00, 0x0f));	// blue
	tftFillRect(pSpiTftModule, pSpiTftSlave, 55, 25, 10, 10, ST7735_RGB(0x0f, 0x00, 0x00));	// red
	tftFillRect(pSpiTftModule, pSpiTftSlave, 65, 35, 10, 10, ST7735_RGB(0x00, 0x1f, 0x00));	// green
	tftFillRect(pSpiTftModule, pSpiTftSlave, 55, 45, 10, 10, ST7735_RGB(0x00, 0x1f, 0x0f));	// cyan
	tftFillRect(pSpiTftModule, pSpiTftSlave, 65, 55, 10, 10, ST7735_RGB(0x0f, 0x00, 0x0f));	// magenta
	tftFillRect(pSpiTftModule, pSpiTftSlave, 55, 65, 10, 10, ST7735_RGB(0x0f, 0x1f, 0x00));	// yellow
	tftFillRect(pSpiTftModule, pSpiTftSlave, 65, 75, 10, 10, ST7735_RGB(0x0f, 0x1f, 0x0f));	// white
}

void interactiveReadTempSensor(struct spi_module *pSpiModuleTempSensor, struct spi_slave_inst *pSpiSlaveInstance)
{
	struct Max31855Data tempData;
	if (max31855ReadData(pSpiModuleTempSensor, pSpiSlaveInstance, &tempData))
	{
		char tempStr[20];
		char internalTempStr[20];
		formatFloat(tempData.Temp, tempStr, sizeof(tempStr), false, 0, 2);
		formatFloat(tempData.InternalTemp, internalTempStr, sizeof(internalTempStr), false, 0, 2);

		char printBuf[100];
		int len = snprintf(printBuf, sizeof(printBuf), "temp: %s, int: %s\n\r", tempStr, internalTempStr);
		udi_cdc_write_buf(printBuf, len);
	}
	else 
	{
		udi_cdc_write_buf("Failure reading temp sensor. Failure code: ", 43);
		udi_cdc_putc(tempData.FailureType + '0');
		udi_cdc_write_buf("\n\r", 2);
	}
}

int main (void)
{
	struct tcc_module tccModule0;
	bool my_flag_autorize_cdc_transfer = true;

	system_init();
	irq_initialize_vectors();
	cpu_irq_enable();
	board_init();
	udc_start();
	delay_init();

	configureTcc0(&tccModule0);
	configureGpioInterrupts();
	system_interrupt_enable_global();

	struct spi_module spiTftModule;
	struct spi_slave_inst spiTftSlave;

	configureTftDisplayPorts(&spiTftModule, &spiTftSlave);

	struct spi_module spiModuleTempSensor;
	struct spi_slave_inst spiSlaveInstance;

	configureSpiTempSensor(&spiModuleTempSensor, &spiSlaveInstance);

	while (1)
	{
		// Echo all input back to CDC port
		if (my_flag_autorize_cdc_transfer) {
			char c = udi_cdc_getc();
			if (c == 't') {
				interactiveReadTempSensor(&spiModuleTempSensor, &spiSlaveInstance);
			} else if (c == 'z') {
				printfToCdc("Zero cross count: %ul\n\r", g_zeroCrossCount);
			} else if (c == 'f') {
					tftFillRect(&spiTftModule, &spiTftSlave,  0,  0, 128, 160, ST7735_RGB(5, 10, 5));
			} else {
				udi_cdc_write_buf("Samogon: Unknown command: ", 26);
				udi_cdc_putc(c);
				udi_cdc_write_buf("\n\r", 2);
			}
		}
		delay_ms(10);
	}
}

bool samogon_callback_cdc_enable(void)
{
	return true;
}

void samogon_callback_cdc_disable(void)
{
}

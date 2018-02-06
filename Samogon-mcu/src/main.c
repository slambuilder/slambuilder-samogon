#include "asf.h"
#include "Max31855.h"
#include "utils.h"
#include "Sitronix7735.h"

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

void configureTftDisplayPorts(Sitronix7735 *pTft)
{
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

	Sitronix7735_Constructor(pTft, 128, 160);
	Sitronix7735_Initialize(pTft, 
		CONF_BOARD_TFT_SERCOM_MUX_SETTING,
		CONF_BOARD_TFT_SERCOM_PAD0,
		CONF_BOARD_TFT_SERCOM_PAD1, 
		CONF_BOARD_TFT_SERCOM_PAD2,
		CONF_BOARD_TFT_SERCOM_PAD3, 
		CONF_BOARD_TFT_SERCOM, 
		CONF_BOARD_TFT_SERCOM_BAUDRATE, 
		CONF_BOARD_TFT_LITE_OUT_PIN, 
		CONF_BOARD_TFT_CS_OUT_PIN, 
		CONF_BOARD_TFT_CARD_CS_OUT_PIN, 
		CONF_BOARD_TFT_DC_SELECT_OUT_PIN, 
		CONF_BOARD_TFT_RST_OUT_PIN);

	Sitronix7735_fillRect(pTft,  0,  0, 128, 160, ST7735_RGB(20, 40, 30));

	Sitronix7735_fillRect(pTft,  5,  5, 10, 10, ST7735_RGB(0x00, 0x00, 0x00));	// black
	Sitronix7735_fillRect(pTft, 15, 15, 10, 10, ST7735_RGB(0x00, 0x00, 0x1f));	// blue
	Sitronix7735_fillRect(pTft,  5, 25, 10, 10, ST7735_RGB(0x1f, 0x00, 0x00));	// red
	Sitronix7735_fillRect(pTft, 15, 35, 10, 10, ST7735_RGB(0x00, 0x3f, 0x00));	// green
	Sitronix7735_fillRect(pTft,  5, 45, 10, 10, ST7735_RGB(0x00, 0x3f, 0x1f));	// cyan
	Sitronix7735_fillRect(pTft, 15, 55, 10, 10, ST7735_RGB(0x1f, 0x00, 0x1f));	// magenta
	Sitronix7735_fillRect(pTft,  5, 65, 10, 10, ST7735_RGB(0x1f, 0x3f, 0x00));	// yellow
	Sitronix7735_fillRect(pTft, 15, 75, 10, 10, ST7735_RGB(0x1f, 0x3f, 0x1f));	// white

	Sitronix7735_fillRect(pTft, 55,  5, 10, 10, ST7735_RGB(0x00, 0x00, 0x00));	// black
	Sitronix7735_fillRect(pTft, 65, 15, 10, 10, ST7735_RGB(0x00, 0x00, 0x0f));	// blue
	Sitronix7735_fillRect(pTft, 55, 25, 10, 10, ST7735_RGB(0x0f, 0x00, 0x00));	// red
	Sitronix7735_fillRect(pTft, 65, 35, 10, 10, ST7735_RGB(0x00, 0x1f, 0x00));	// green
	Sitronix7735_fillRect(pTft, 55, 45, 10, 10, ST7735_RGB(0x00, 0x1f, 0x0f));	// cyan
	Sitronix7735_fillRect(pTft, 65, 55, 10, 10, ST7735_RGB(0x0f, 0x00, 0x0f));	// magenta
	Sitronix7735_fillRect(pTft, 55, 65, 10, 10, ST7735_RGB(0x0f, 0x1f, 0x00));	// yellow
	Sitronix7735_fillRect(pTft, 65, 75, 10, 10, ST7735_RGB(0x0f, 0x1f, 0x0f));	// white
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

	Sitronix7735 tft;

	configureTftDisplayPorts(&tft);

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
				printfToCdc("Zero cross count: %u\n\r", g_zeroCrossCount);
			} else if (c == 'f') {
					tft.m_base.vt->pfnFillScreen(&tft, ST7735_RGB(5, 10, 5));
					tft.m_base.vt->pfnSetRotation(&tft, 3);
					AdafruitGfx_setTextSize(&tft, 2);
					AdafruitGfx_setCursor(&tft, 0, 0);
					tft.m_base.vt->pfnWrite(&tft, 'a');
					tft.m_base.vt->pfnWrite(&tft, 'b');
					tft.m_base.vt->pfnWrite(&tft, 'c');
					tft.m_base.vt->pfnWrite(&tft, 'd');
					tft.m_base.vt->pfnWrite(&tft, 'e');
					tft.m_base.vt->pfnWrite(&tft, 'f');
					tft.m_base.vt->pfnWrite(&tft, 'g');
					tft.m_base.vt->pfnWrite(&tft, 'h');
					tft.m_base.vt->pfnWrite(&tft, 'i');
					tft.m_base.vt->pfnWrite(&tft, 'j');
					tft.m_base.vt->pfnWrite(&tft, 'k');
					tft.m_base.vt->pfnWrite(&tft, 'l');
					tft.m_base.vt->pfnWrite(&tft, 'm');
					AdafruitGfx_setCursor(&tft, 0, 20);
					tft.m_base.vt->pfnWrite(&tft, 'A');
					tft.m_base.vt->pfnWrite(&tft, 'B');
					tft.m_base.vt->pfnWrite(&tft, 'C');
					tft.m_base.vt->pfnWrite(&tft, 'D');
					tft.m_base.vt->pfnWrite(&tft, 'E');
					tft.m_base.vt->pfnWrite(&tft, 'F');
					tft.m_base.vt->pfnWrite(&tft, 'G');
					tft.m_base.vt->pfnWrite(&tft, 'H');
					tft.m_base.vt->pfnWrite(&tft, 'I');
					tft.m_base.vt->pfnWrite(&tft, 'J');
					tft.m_base.vt->pfnWrite(&tft, 'K');
					tft.m_base.vt->pfnWrite(&tft, 'L');
					tft.m_base.vt->pfnWrite(&tft, 'M');
					AdafruitGfx_setCursor(&tft, 0, 40);
					tft.m_base.vt->pfnWrite(&tft, '0');
					tft.m_base.vt->pfnWrite(&tft, '1');
					tft.m_base.vt->pfnWrite(&tft, '2');
					tft.m_base.vt->pfnWrite(&tft, '3');
					tft.m_base.vt->pfnWrite(&tft, '4');
					tft.m_base.vt->pfnWrite(&tft, '5');
					tft.m_base.vt->pfnWrite(&tft, '6');
					tft.m_base.vt->pfnWrite(&tft, '7');
					tft.m_base.vt->pfnWrite(&tft, '8');
					tft.m_base.vt->pfnWrite(&tft, '9');
					tft.m_base.vt->pfnWrite(&tft, '-');
					tft.m_base.vt->pfnWrite(&tft, '+');
					tft.m_base.vt->pfnWrite(&tft, '[');
					AdafruitGfx_setCursor(&tft, 0, 60);
					tft.m_base.vt->pfnWrite(&tft, '!');
					tft.m_base.vt->pfnWrite(&tft, '@');
					tft.m_base.vt->pfnWrite(&tft, '#');
					tft.m_base.vt->pfnWrite(&tft, '$');
					tft.m_base.vt->pfnWrite(&tft, '%');
					tft.m_base.vt->pfnWrite(&tft, '^');
					tft.m_base.vt->pfnWrite(&tft, '&');
					tft.m_base.vt->pfnWrite(&tft, '*');
					tft.m_base.vt->pfnWrite(&tft, '(');
					tft.m_base.vt->pfnWrite(&tft, ')');
					tft.m_base.vt->pfnWrite(&tft, ';');
					tft.m_base.vt->pfnWrite(&tft, ':');
					tft.m_base.vt->pfnWrite(&tft, '{');
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

#include "asf.h"
#include "utils.h"
#include "UserTasks.h"

// forward declarations
void callbackRotaryEncoderChannelA(void);
void callbackRotaryEncoderChannelB(void);
void callbackPushButton(void);
void callbackZeroCross(void);
void tempSampleCallback(TimerHandle_t hTimer);

void configureSpiTempSensor(
	struct spi_module *pSpiModuleTempSensor, 
	struct spi_slave_inst *pSpiSlaveInstance1,
	struct spi_slave_inst *pSpiSlaveInstance2)
{
	struct spi_config spiConfig;
	struct spi_slave_inst_config slaveConfig;

	// Configure and initialize software device instance of peripheral slave 

	// first sensor
	spi_slave_inst_get_config_defaults(&slaveConfig);
	slaveConfig.ss_pin = CONF_BOARD_MAX31855_SENSOR1_SS_OUT_PIN;
	spi_attach_slave(pSpiSlaveInstance1, &slaveConfig);

	// second sensor
	spi_slave_inst_get_config_defaults(&slaveConfig);
	slaveConfig.ss_pin = CONF_BOARD_MAX31855_SENSOR2_SS_OUT_PIN;
	spi_attach_slave(pSpiSlaveInstance2, &slaveConfig);

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

	pTft->m_base.vt->pfnSetRotation(pTft, 3);
	pTft->m_base.vt->pfnFillScreen(pTft, TFT_BACKGROUND);
	AdafruitGfx_setTextColorWithBg(pTft, TFT_TEXT_COLOR, TFT_BACKGROUND);

	//pTft->m_base.vt->pfnFillRect(pTft, 145,  5, 10, 10, ST7735_RGB(0x00, 0x00, 0x00));	// black
	//pTft->m_base.vt->pfnFillRect(pTft, 135, 15, 10, 10, ST7735_RGB(0x00, 0x00, 0x1f));	// blue
	//pTft->m_base.vt->pfnFillRect(pTft, 145, 25, 10, 10, ST7735_RGB(0x1f, 0x00, 0x00));	// red
	//pTft->m_base.vt->pfnFillRect(pTft, 135, 35, 10, 10, ST7735_RGB(0x00, 0x3f, 0x00));	// green
	//pTft->m_base.vt->pfnFillRect(pTft, 145, 45, 10, 10, ST7735_RGB(0x00, 0x3f, 0x1f));	// cyan
	//pTft->m_base.vt->pfnFillRect(pTft, 135, 55, 10, 10, ST7735_RGB(0x1f, 0x00, 0x1f));	// magenta
	//pTft->m_base.vt->pfnFillRect(pTft, 145, 65, 10, 10, ST7735_RGB(0x1f, 0x3f, 0x00));	// yellow
	//pTft->m_base.vt->pfnFillRect(pTft, 135, 75, 10, 10, ST7735_RGB(0x1f, 0x3f, 0x1f));	// white
	//pTft->m_base.vt->pfnFillRect(pTft, 145, 85, 10, 10, ST7735_RGB(0x00, 0x1f, 0x0f));	// cyan
	//pTft->m_base.vt->pfnFillRect(pTft, 135, 95, 10, 10, ST7735_RGB(0x0f, 0x00, 0x0f));	// magenta
	//pTft->m_base.vt->pfnFillRect(pTft, 145,105, 10, 10, ST7735_RGB(0x0f, 0x1f, 0x00));	// yellow
	//pTft->m_base.vt->pfnFillRect(pTft, 135,115, 10, 10, ST7735_RGB(0x0f, 0x1f, 0x0f));	// white
}

// global instances
AppData g_appData;

int main (void)
{
	system_init();
	irq_initialize_vectors();
	cpu_irq_enable();
	board_init();
	udc_start();
	delay_init();

	memset(&g_appData, 0, sizeof(g_appData));
	g_appData.bAutorizeCdcTransfer = true;

	if (!readNvmUserConfig(&g_appData.nvmUserConfig))
	{
		getDefaultNvmUserConfig(&g_appData.nvmUserConfig);
	}

	configureGpioInterrupts();
	system_interrupt_enable_global();

	configureTftDisplayPorts(&g_appData.tft);

	configureSpiTempSensor(
		&g_appData.spiModuleTempSensor, 
		&g_appData.spiSlaveTempSensor1,
		&g_appData.spiSlaveTempSensor2);

	g_appData.hControlQueue = xQueueCreate(10, sizeof(ControlMessage));
	g_appData.hTickQueue = xQueueCreate(1, sizeof(TickType_t));
	g_appData.hDisplayChangeQueue = xQueueCreate(10, sizeof(DisplayChangeMessage));

	// CDC loop task has low priority, just 1 above an idle
	xTaskCreate(taskCdcLoop, "CDC", configMINIMAL_STACK_SIZE * 2, &g_appData, tskIDLE_PRIORITY + 1, &g_appData.hTaskCdcLoop);

	// Control loop task has priority 2 above an idle
	xTaskCreate(taskControlProcessLoop, "Control", configMINIMAL_STACK_SIZE * 2, &g_appData, tskIDLE_PRIORITY + 2, &g_appData.hTaskControlProcessLoop);

	// PID Control loop task has priority 3 above an idle
	xTaskCreate(taskPullSensorDataLoop, "PullSensorData", configMINIMAL_STACK_SIZE * 2, &g_appData, tskIDLE_PRIORITY + 3, &g_appData.hTaskPullSensorDataLoop);

	// Display loop task has priority 1 above an idle
	xTaskCreate(taskDisplayLoop, "Display", configMINIMAL_STACK_SIZE * 2, &g_appData, tskIDLE_PRIORITY + 1, &g_appData.hTaskDisplayLoop);

	g_appData.hTempSampleTimer = xTimerCreate("TempSample", 1000, pdTRUE, NULL, tempSampleCallback);
	xTimerStart(g_appData.hTempSampleTimer, 0);

	// Start the scheduler. Normally, this function never returns.
	vTaskStartScheduler();
}

// This is a timer callback function.
// It should never block. 
// Current implementation only sends a message in a queue using non-blocking call.
void tempSampleCallback(TimerHandle_t hTimer)
{
	TickType_t currentTicks = xTaskGetTickCount();
	xQueueSendToBack(g_appData.hTickQueue, &currentTicks, 0);
}

bool samogon_callback_cdc_enable(void)
{
	g_appData.bAutorizeCdcTransfer = true;
	return true;
}

void samogon_callback_cdc_disable(void)
{
	g_appData.bAutorizeCdcTransfer = false;
}

void queueMessageFromCallback(TickType_t *pInitialSignalTicks, ControlMessageType type)
{
	TickType_t currentTicks = xTaskGetTickCountFromISR();

	// check for ticks overflow
	if (currentTicks < *pInitialSignalTicks) 
	{
		// reset initial ticks back to 0 when current ticks are overflown
		*pInitialSignalTicks = 0;
	}

	// debounce logic: only signal once within DEBOUNCE_TICKS interval
	if (currentTicks - *pInitialSignalTicks > DEBOUNCE_TICKS) 
	{
		*pInitialSignalTicks = currentTicks;

		ControlMessage message = {
			type: type,
			ticks: currentTicks
		};
		xQueueSendToBackFromISR(g_appData.hControlQueue, &message, NULL);
	}
}

void callbackPushButton(void)
{
	static TickType_t s_initialSignalTicks = 0;
	queueMessageFromCallback(&s_initialSignalTicks, CM_PushButtonPressed);
}

void callbackRotaryEncoderChannelA(void)
{
	static TickType_t s_initialSignalTicks = 0;
	queueMessageFromCallback(&s_initialSignalTicks, CM_RotaryEncoderChannelA);
}

void callbackRotaryEncoderChannelB(void)
{
	static TickType_t s_initialSignalTicks = 0;
	queueMessageFromCallback(&s_initialSignalTicks, CM_RotaryEncoderChannelB);
}

void callbackZeroCross(void)
{
	g_appData.zeroCrossCount++;
}


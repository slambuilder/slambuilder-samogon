#include "asf.h"
#include "Max31855.h"
#include "utils.h"
#include "Sitronix7735.h"

// forward declarations
void callbackRotaryEncoderChannelA(void);
void callbackRotaryEncoderChannelB(void);
void callbackPushButton(void);
void callbackZeroCross(void);
void tempSampleCallback(TimerHandle_t hTimer);

#define TFT_BACKGROUND ST7735_RGB(30, 60, 30)
#define TFT_TEXT_COLOR ST7735_RGB(1, 2, 1)

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

	pTft->m_base.vt->pfnFillRect(pTft, 145,  5, 10, 10, ST7735_RGB(0x00, 0x00, 0x00));	// black
	pTft->m_base.vt->pfnFillRect(pTft, 135, 15, 10, 10, ST7735_RGB(0x00, 0x00, 0x1f));	// blue
	pTft->m_base.vt->pfnFillRect(pTft, 145, 25, 10, 10, ST7735_RGB(0x1f, 0x00, 0x00));	// red
	pTft->m_base.vt->pfnFillRect(pTft, 135, 35, 10, 10, ST7735_RGB(0x00, 0x3f, 0x00));	// green
	pTft->m_base.vt->pfnFillRect(pTft, 145, 45, 10, 10, ST7735_RGB(0x00, 0x3f, 0x1f));	// cyan
	pTft->m_base.vt->pfnFillRect(pTft, 135, 55, 10, 10, ST7735_RGB(0x1f, 0x00, 0x1f));	// magenta
	pTft->m_base.vt->pfnFillRect(pTft, 145, 65, 10, 10, ST7735_RGB(0x1f, 0x3f, 0x00));	// yellow
	pTft->m_base.vt->pfnFillRect(pTft, 135, 75, 10, 10, ST7735_RGB(0x1f, 0x3f, 0x1f));	// white
	pTft->m_base.vt->pfnFillRect(pTft, 145, 85, 10, 10, ST7735_RGB(0x00, 0x1f, 0x0f));	// cyan
	pTft->m_base.vt->pfnFillRect(pTft, 135, 95, 10, 10, ST7735_RGB(0x0f, 0x00, 0x0f));	// magenta
	pTft->m_base.vt->pfnFillRect(pTft, 145,105, 10, 10, ST7735_RGB(0x0f, 0x1f, 0x00));	// yellow
	pTft->m_base.vt->pfnFillRect(pTft, 135,115, 10, 10, ST7735_RGB(0x0f, 0x1f, 0x0f));	// white
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
		int len = snprintf(printBuf, sizeof(printBuf), "temp: %s, int: %s, ticks: %u\n\r", tempStr, internalTempStr, xTaskGetTickCountFromISR());
		udi_cdc_write_buf(printBuf, len);
	}
	else 
	{
		udi_cdc_write_buf("Failure reading temp sensor. Failure code: ", 43);
		udi_cdc_putc(tempData.FailureType + '0');
		udi_cdc_write_buf("\n\r", 2);
	}
}

typedef enum TControlMessageType
{
	PushButtonPressed,
	RotaryEncoderChannelA,
	RotaryEncoderChannelB,
} ControlMessageType;

typedef struct TControlMessage
{
	ControlMessageType type;
	TickType_t ticks;
} ControlMessage;

typedef struct TAppData
{
	Sitronix7735 tft;
	struct spi_module spiModuleTempSensor;
	struct spi_slave_inst spiSlaveTempSensor;
	TaskHandle_t hTaskCdcLoop;
	TaskHandle_t hTaskControlReadLoop;
	TaskHandle_t hTaskPidControlLoop;
	bool bAutorizeCdcTransfer;
	QueueHandle_t hControlQueue;
	QueueHandle_t hTickQueue;
	TimerHandle_t hTempSampleTimer;
	float temperature;
	float internalTemperature;
	uint32_t zeroCrossCount;
} AppData;

// global instances
AppData g_appData;

void taskCdcLoop(void *pvParameters)
{
	while (1)
	{
		// Echo all input back to CDC port
		if (g_appData.bAutorizeCdcTransfer) {
			char c = udi_cdc_getc();
			if (c == 't') {
				interactiveReadTempSensor(&g_appData.spiModuleTempSensor, &g_appData.spiSlaveTempSensor);
			} else if (c == 'z') {
				printfToCdc("Zero cross count: %u\n\r", g_appData.zeroCrossCount);
			} else if (c == 'f') {
				Sitronix7735 *pTft = &g_appData.tft;
				pTft->m_base.vt->pfnFillScreen(pTft, TFT_BACKGROUND);
			} else {
				udi_cdc_write_buf("Samogon: Unknown command: ", 26);
				udi_cdc_putc(c);
				udi_cdc_write_buf("\n\r", 2);
			}
		} else {
			delay_ms(10);
		}
	}
}

// control reading task waits for a "Control" queue
void taskControlReadLoop(void *pvParameters)
{
	ControlMessage message;

	while (true)
	{
		xQueueReceive(g_appData.hControlQueue, &message, portMAX_DELAY);

		if (message.type == PushButtonPressed) 
		{
			// perform additional debounce for push button
			delay_ms(1);
			bool pin_state = port_pin_get_input_level(CONF_BOARD_PUSH_BUTTON_EXTINT_PIN);
			if (pin_state)
			{
				continue;
			}
		}

		printfToCdc("Control message: %d, time: %u\n\r", message.type, message.ticks);
	}
}

// control reading task waits for a "Tick" queue
void taskPidControlLoop(void *pvParameters)
{
	TickType_t ticks;
	Sitronix7735 *pTft = &g_appData.tft;
	struct Max31855Data tempData;
	char strBuffer[20];

	while (true)
	{
		xQueueReceive(g_appData.hTickQueue, &ticks, portMAX_DELAY);

		// sample the temperature sensor data
		 if (max31855ReadData(&g_appData.spiModuleTempSensor, &g_appData.spiSlaveTempSensor, &tempData)) 
		 {
			AdafruitGfx_setTextSize(pTft, 1);
			Sitronix7735_Text(pTft, "sensor", 2, 2, TFT_TEXT_COLOR, TFT_BACKGROUND, true);

			AdafruitGfx_setTextSize(pTft, 2);
			formatFloat(tempData.Temp, strBuffer, sizeof(strBuffer), false, 0, 2);
			Sitronix7735_Text(pTft, strBuffer, 15, 20, TFT_TEXT_COLOR, TFT_BACKGROUND, true);

			AdafruitGfx_setTextSize(pTft, 1);
			Sitronix7735_Text(pTft, "room", 2, 45, TFT_TEXT_COLOR, TFT_BACKGROUND, true);

			AdafruitGfx_setTextSize(pTft, 2);
			formatFloat(tempData.InternalTemp, strBuffer, sizeof(strBuffer), false, 0, 2);
			Sitronix7735_Text(pTft, strBuffer, 15, 63, TFT_TEXT_COLOR, TFT_BACKGROUND, true);
		}
		else 
		{
			AdafruitGfx_setTextSize(pTft, 2);
			Sitronix7735_Text(pTft, "Sensor error", 0, 0, TFT_TEXT_COLOR, TFT_BACKGROUND, true);
		}
	}
}

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

	configureGpioInterrupts();
	system_interrupt_enable_global();

	configureTftDisplayPorts(&g_appData.tft);

	configureSpiTempSensor(&g_appData.spiModuleTempSensor, &g_appData.spiSlaveTempSensor);

	g_appData.hControlQueue = xQueueCreate(10, sizeof(ControlMessage));
	g_appData.hTickQueue = xQueueCreate(1, sizeof(TickType_t));

	// CDC loop task has low priority, just 1 above an idle
	xTaskCreate(taskCdcLoop, "CDC", configMINIMAL_STACK_SIZE * 2, &g_appData, tskIDLE_PRIORITY + 1, &g_appData.hTaskCdcLoop);

	// Control loop task has priority 2 above an idle
	xTaskCreate(taskControlReadLoop, "ControlRead", configMINIMAL_STACK_SIZE * 2, &g_appData, tskIDLE_PRIORITY + 2, &g_appData.hTaskControlReadLoop);

	// PID Control loop task has priority 3 above an idle
	xTaskCreate(taskPidControlLoop, "PidControl", configMINIMAL_STACK_SIZE * 2, &g_appData, tskIDLE_PRIORITY + 2, &g_appData.hTaskPidControlLoop);

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

#define DEBOUNCE_TICKS 200

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
	queueMessageFromCallback(&s_initialSignalTicks, PushButtonPressed);
}

void callbackRotaryEncoderChannelA(void)
{
	static TickType_t s_initialSignalTicks = 0;
	queueMessageFromCallback(&s_initialSignalTicks, RotaryEncoderChannelA);
}

void callbackRotaryEncoderChannelB(void)
{
	static TickType_t s_initialSignalTicks = 0;
	queueMessageFromCallback(&s_initialSignalTicks, RotaryEncoderChannelB);
}

void callbackZeroCross(void)
{
	g_appData.zeroCrossCount++;
}


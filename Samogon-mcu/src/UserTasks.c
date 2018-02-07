#include <asf.h>
#include "UserTasks.h"
#include "utils.h"

void interactiveReadTempSensor(struct spi_module *pSpiModuleTempSensor, struct spi_slave_inst *pSpiSlaveInstance)
{
	Max31855Data tempData;
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
	DisplayChangeMessage dcMessage;
	BaseType_t ret;
	TickType_t lastTimeChannelA = 0;
	TickType_t lastTimeChannelB = 0;

	while (true)
	{
		ret = xQueueReceive(g_appData.hControlQueue, &message, portMAX_DELAY);
		if (ret != pdTRUE) 
		{
			continue;
		}

		if (message.type == CM_PushButtonPressed)
		{
			// perform additional debounce for push button
			delay_ms(1);
			bool pin_state = port_pin_get_input_level(CONF_BOARD_PUSH_BUTTON_EXTINT_PIN);
			if (pin_state)
			{
				continue;
			}

			dcMessage.type = DCM_PushButtonPressed;
			xQueueSendToBack(g_appData.hDisplayChangeQueue, &dcMessage, 10);
		}
		else if (message.type == CM_RotaryEncoderChannelA)
		{
			if (lastTimeChannelB == 0) 
			{
				lastTimeChannelA = message.ticks;
			}
			else if (message.ticks - lastTimeChannelB < (DEBOUNCE_TICKS * 1) )
			{
				dcMessage.type = DCM_RotaryEncoderCCW;
				xQueueSendToBack(g_appData.hDisplayChangeQueue, &dcMessage, 10);
				lastTimeChannelA = 0;
				lastTimeChannelB = 0;
			}
			else 
			{
				lastTimeChannelB = 0;
				lastTimeChannelA = message.ticks;
			}
		}
		else if (message.type == CM_RotaryEncoderChannelB)
		{
			if (lastTimeChannelA == 0)
			{
				lastTimeChannelB = message.ticks;
			}
			else if (message.ticks - lastTimeChannelA < (DEBOUNCE_TICKS * 1) )
			{
				dcMessage.type = DCM_RotaryEncoderCW;
				xQueueSendToBack(g_appData.hDisplayChangeQueue, &dcMessage, 10);
				lastTimeChannelA = 0;
				lastTimeChannelB = 0;
			}
			else
			{
				lastTimeChannelA = 0;
				lastTimeChannelB = message.ticks;
			}
		}
	}
}

// control reading task waits for a "Tick" queue
void taskPidControlLoop(void *pvParameters)
{
	TickType_t ticks;
	Max31855Data tempData;
	BaseType_t ret;
	float lastTemp = 0.0;
	float lastIntTemp = 0.0;
	DisplayChangeMessage dcMessage;

	while (true)
	{
		ret = xQueueReceive(g_appData.hTickQueue, &ticks, portMAX_DELAY);
		if (ret != pdTRUE)
		{
			continue;
		}

		// sample the temperature sensor data
		if (max31855ReadData(&g_appData.spiModuleTempSensor, &g_appData.spiSlaveTempSensor, &tempData))
		{
			if (tempData.Temp != lastTemp || tempData.InternalTemp != lastIntTemp)
			{
				dcMessage.type = DCM_SensorDataChanged;
				dcMessage.temperature = tempData.Temp;
				dcMessage.internalTemperature = tempData.InternalTemp;
				xQueueSendToBack(g_appData.hDisplayChangeQueue, &dcMessage, 10);

				lastTemp = tempData.Temp;
				lastIntTemp = tempData.InternalTemp;
			}
		}
		else
		{
			dcMessage.type = DCM_SensorError;
			xQueueSendToBack(g_appData.hDisplayChangeQueue, &dcMessage, 10);
		}
	}
}

// The display loop task is responsible for updating any information on display.
void taskDisplayLoop(void *pvParameters)
{
	Sitronix7735 *pTft = &g_appData.tft;
	DisplayChangeMessage message;
	BaseType_t ret;
	char strBuffer[20];

	AdafruitGfx_setTextSize(pTft, 1);
	Sitronix7735_Text(pTft, "sensor", 2, 2, TFT_TEXT_COLOR, TFT_BACKGROUND, true);
	Sitronix7735_Text(pTft, "room", 2, 45, TFT_TEXT_COLOR, TFT_BACKGROUND, true);
	Sitronix7735_Text(pTft, "set", 2, 88, TFT_TEXT_COLOR, TFT_BACKGROUND, true);

	AdafruitGfx_setTextSize(pTft, 2);
	formatFloat(g_appData.nvmUserConfig.targetTemperature, strBuffer, sizeof(strBuffer), false, 0, 2);
	Sitronix7735_Text(pTft, strBuffer, 15, 106, TFT_TEXT_COLOR, TFT_BACKGROUND, true);

	while (true)
	{
		ret = xQueueReceive(g_appData.hDisplayChangeQueue, &message, portMAX_DELAY);
		if (ret != pdTRUE)
		{
			continue;
		}

		switch(message.type)
		{
			case DCM_PushButtonPressed:
				printfToCdc("Display loop: DC_PushButtonPressed\n\r");
				break;
			case DCM_RotaryEncoderCW:
				printfToCdc("Display loop: DC_RotaryEncoderCW\n\r");
				g_appData.nvmUserConfig.targetTemperature += 0.25;
				writeNvmUserConfig(&g_appData.nvmUserConfig);

				AdafruitGfx_setTextSize(pTft, 2);
				formatFloat(g_appData.nvmUserConfig.targetTemperature, strBuffer, sizeof(strBuffer), false, 0, 2);
				Sitronix7735_Text(pTft, strBuffer, 15, 106, TFT_TEXT_COLOR, TFT_BACKGROUND, true);
				break;
			case DCM_RotaryEncoderCCW:
				printfToCdc("Display loop: DC_RotaryEncoderCCW\n\r");
				g_appData.nvmUserConfig.targetTemperature -= 0.25;
				writeNvmUserConfig(&g_appData.nvmUserConfig);

				AdafruitGfx_setTextSize(pTft, 2);
				formatFloat(g_appData.nvmUserConfig.targetTemperature, strBuffer, sizeof(strBuffer), false, 0, 2);
				Sitronix7735_Text(pTft, strBuffer, 15, 106, TFT_TEXT_COLOR, TFT_BACKGROUND, true);
				break;
			case DCM_SensorDataChanged:
				AdafruitGfx_setTextSize(pTft, 2);
				formatFloat(message.temperature, strBuffer, sizeof(strBuffer), false, 0, 2);
				Sitronix7735_Text(pTft, strBuffer, 15, 20, TFT_TEXT_COLOR, TFT_BACKGROUND, true);
				formatFloat(message.internalTemperature, strBuffer, sizeof(strBuffer), false, 0, 2);
				Sitronix7735_Text(pTft, strBuffer, 15, 63, TFT_TEXT_COLOR, TFT_BACKGROUND, true);
				break;
			case DCM_SensorError:
				AdafruitGfx_setTextSize(pTft, 2);
				Sitronix7735_Text(pTft, "Sensor error", 2, 2, TFT_TEXT_COLOR, TFT_BACKGROUND, true);
				break;
			default:
				printfToCdc("Display loop: Unknown message\n\r");
				break;
		}
	}
}
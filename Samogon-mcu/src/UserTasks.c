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

		printfToCdc("temp: %s, int: %s, ticks: %u\n\r", tempStr, internalTempStr, xTaskGetTickCountFromISR());
	}
	else
	{
		printfToCdc("Failure reading temp sensor. Failure code: %d\n\r", tempData.FailureType);
	}
}

extern bool g_enableDebugPrintToCdc;

void taskCdcLoop(void *pvParameters)
{
	while (1)
	{
		// Echo all input back to CDC port
		if (g_appData.bAutorizeCdcTransfer) {
			char c = udi_cdc_getc();
			g_enableDebugPrintToCdc = true;
			if (c == 't') {
				interactiveReadTempSensor(&g_appData.spiModuleTempSensor, &g_appData.spiSlaveTempSensor1);
				interactiveReadTempSensor(&g_appData.spiModuleTempSensor, &g_appData.spiSlaveTempSensor2);
			} else if (c == 'z') {
				printfToCdc("Zero cross count: %u\n\r", g_appData.zeroCrossCount);
			} else if (c == 'f') {
				Sitronix7735 *pTft = &g_appData.tft;
				pTft->m_base.vt->pfnFillScreen(pTft, TFT_BACKGROUND);
			} else {
				printfToCdc("Samogon: Unknown command: %c\n\r", c);
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
	Max31855Data tempData1, tempData2;
	BaseType_t ret;
	float lastTemp1 = 0.0, lastIntTemp1 = 0.0;
	float lastTemp2 = 0.0, lastIntTemp2 = 0.0;
	DisplayChangeMessage dcMessage;

	while (true)
	{
		ret = xQueueReceive(g_appData.hTickQueue, &ticks, portMAX_DELAY);
		if (ret != pdTRUE)
		{
			continue;
		}

		// sample the temperature sensor data
		if (max31855ReadData(&g_appData.spiModuleTempSensor, &g_appData.spiSlaveTempSensor1, &tempData1) && 
			max31855ReadData(&g_appData.spiModuleTempSensor, &g_appData.spiSlaveTempSensor2, &tempData2))
		{
			if (tempData1.Temp != lastTemp1 || tempData1.InternalTemp != lastIntTemp1 || 
				tempData2.Temp != lastTemp2 || tempData2.InternalTemp != lastIntTemp2)
			{
				dcMessage.type = DCM_SensorDataChanged;
				dcMessage.temperature1 = tempData1.Temp;
				dcMessage.internalTemperature1 = tempData1.InternalTemp;
				dcMessage.temperature2 = tempData2.Temp;
				dcMessage.internalTemperature2 = tempData2.InternalTemp;
				xQueueSendToBack(g_appData.hDisplayChangeQueue, &dcMessage, 10);

				lastTemp1 = tempData1.Temp;
				lastIntTemp1 = tempData1.InternalTemp;
				lastTemp2 = tempData2.Temp;
				lastIntTemp2 = tempData2.InternalTemp;
			}
		}
		else
		{
			dcMessage.type = DCM_SensorError;
			xQueueSendToBack(g_appData.hDisplayChangeQueue, &dcMessage, 10);
		}
	}
}

#define TFT_Y_POWER_LABEL 2
#define TFT_Y_SENSOR1_LABEL 45
#define TFT_Y_SENSOR2_LABEL 88
#define TFT_Y_POWER_VALUE (TFT_Y_POWER_LABEL + 18)
#define TFT_Y_SENSOR1_VALUE (TFT_Y_SENSOR1_LABEL + 18)
#define TFT_Y_SENSOR2_VALUE (TFT_Y_SENSOR2_LABEL + 18)

#define TFT_X_LABEL 2
#define TFT_X_VALUE 15

#define TFT_X_POWER 81
#define TFT_Y_POWER 10
#define TFT_POWER_X1 75
#define TFT_POWER_Y1 2
#define TFT_POWER_X2 157
#define TFT_POWER_Y2 45
#define TFT_POWER_BORDER_COLOR ST7735_RGB(0,0,0)
#define TFT_POWER_ACTIVE_COLOR ST7735_RGB(0x1f,0,0x1f)
#define TFT_POWER_PASSIVE_COLOR ST7735_RGB(0,0,0)

// The display loop task is responsible for updating any information on display.
void taskDisplayLoop(void *pvParameters)
{
	Sitronix7735 *pTft = &g_appData.tft;
	DisplayChangeMessage message;
	BaseType_t ret;
	char strBuffer[20];
	bool sensorError = false;
	bool powerOn = false;

	AdafruitGfx_setTextSize(pTft, 1);
	Sitronix7735_Text(pTft, "power   ", TFT_X_LABEL,   TFT_Y_POWER_LABEL, TFT_TEXT_COLOR, TFT_BACKGROUND, true);
	Sitronix7735_Text(pTft, "sensor 1", TFT_X_LABEL, TFT_Y_SENSOR1_LABEL, TFT_TEXT_COLOR, TFT_BACKGROUND, true);
	Sitronix7735_Text(pTft, "sensor 2", TFT_X_LABEL, TFT_Y_SENSOR2_LABEL, TFT_TEXT_COLOR, TFT_BACKGROUND, true);

	AdafruitGfx_setTextSize(pTft, 2);
	formatFloat(g_appData.nvmUserConfig.power, strBuffer, sizeof(strBuffer), false, 0, 0);
	appendString(strBuffer, sizeof(strBuffer), "%  ");
	Sitronix7735_Text(pTft, strBuffer, TFT_X_VALUE, TFT_Y_POWER_VALUE, TFT_TEXT_COLOR, TFT_BACKGROUND, true);

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
				powerOn = !powerOn;
				{
					AdafruitGfx_setTextSize(pTft, 4);
					Sitronix7735_drawFastHLine(pTft, TFT_POWER_X1, TFT_POWER_Y1, TFT_POWER_X2 - TFT_POWER_X1 + 1, TFT_POWER_BORDER_COLOR);
					Sitronix7735_drawFastHLine(pTft, TFT_POWER_X1, TFT_POWER_Y2, TFT_POWER_X2 - TFT_POWER_X1 + 1, TFT_POWER_BORDER_COLOR);
					Sitronix7735_drawFastVLine(pTft, TFT_POWER_X1, TFT_POWER_Y1, TFT_POWER_Y2 - TFT_POWER_Y1 + 1, TFT_POWER_BORDER_COLOR);
					Sitronix7735_drawFastVLine(pTft, TFT_POWER_X2, TFT_POWER_Y1, TFT_POWER_Y2 - TFT_POWER_Y1 + 1, TFT_POWER_BORDER_COLOR);
					pTft->m_base.vt->pfnFillRect(pTft, 
						TFT_POWER_X1 + 1, 
						TFT_POWER_Y1 + 1, 
						TFT_POWER_X2 - TFT_POWER_X1 - 1, 
						TFT_POWER_Y2 - TFT_POWER_Y1 - 1, 
						powerOn ? TFT_POWER_ACTIVE_COLOR : TFT_POWER_PASSIVE_COLOR);
					Sitronix7735_Text(pTft, 
						powerOn ? "ON" : "OFF", 
						powerOn ? TFT_X_POWER + 14 : TFT_X_POWER, 
						TFT_Y_POWER, 
						powerOn ? TFT_POWER_PASSIVE_COLOR : TFT_BACKGROUND, 
						powerOn ? TFT_POWER_ACTIVE_COLOR : TFT_POWER_PASSIVE_COLOR, 
						true);
				}
				printfToCdc("Display loop: DC_PushButtonPressed\n\r");
				break;
			case DCM_RotaryEncoderCW:
				printfToCdc("Display loop: DC_RotaryEncoderCW\n\r");
				g_appData.nvmUserConfig.power += 5.0;
				if (g_appData.nvmUserConfig.power > 100.0) 
				{
					g_appData.nvmUserConfig.power = 100.0;
				}
				writeNvmUserConfig(&g_appData.nvmUserConfig);

				AdafruitGfx_setTextSize(pTft, 2);
				formatFloat(g_appData.nvmUserConfig.power, strBuffer, sizeof(strBuffer), false, 0, 0);
				appendString(strBuffer, sizeof(strBuffer), "%  ");
				Sitronix7735_Text(pTft, strBuffer, TFT_X_VALUE, TFT_Y_POWER_VALUE, TFT_TEXT_COLOR, TFT_BACKGROUND, true);
				break;
			case DCM_RotaryEncoderCCW:
				printfToCdc("Display loop: DC_RotaryEncoderCCW\n\r");
				g_appData.nvmUserConfig.power -= 5.0;
				if (g_appData.nvmUserConfig.power < 5.0)
				{
					g_appData.nvmUserConfig.power = 5.0;
				}
				writeNvmUserConfig(&g_appData.nvmUserConfig);

				AdafruitGfx_setTextSize(pTft, 2);
				formatFloat(g_appData.nvmUserConfig.power, strBuffer, sizeof(strBuffer), false, 0, 0);
				appendString(strBuffer, sizeof(strBuffer), "%  ");
				Sitronix7735_Text(pTft, strBuffer, TFT_X_VALUE, TFT_Y_POWER_VALUE, TFT_TEXT_COLOR, TFT_BACKGROUND, true);
				break;
			case DCM_SensorDataChanged:
				if (sensorError) 
				{
					sensorError = false;
				}
				AdafruitGfx_setTextSize(pTft, 2);
				formatFloat(message.temperature1, strBuffer, sizeof(strBuffer), false, 0, 2);
				appendString(strBuffer, sizeof(strBuffer), "   ");
				Sitronix7735_Text(pTft, strBuffer, TFT_X_VALUE, TFT_Y_SENSOR1_VALUE, TFT_TEXT_COLOR, TFT_BACKGROUND, true);
				formatFloat(message.temperature2, strBuffer, sizeof(strBuffer), false, 0, 2);
				appendString(strBuffer, sizeof(strBuffer), "   ");
				Sitronix7735_Text(pTft, strBuffer, TFT_X_VALUE, TFT_Y_SENSOR2_VALUE, TFT_TEXT_COLOR, TFT_BACKGROUND, true);
				break;
			case DCM_SensorError:
				AdafruitGfx_setTextSize(pTft, 2);
				Sitronix7735_Text(pTft, "        ", TFT_X_VALUE, TFT_Y_SENSOR1_VALUE, TFT_TEXT_COLOR, TFT_BACKGROUND, true);
				Sitronix7735_Text(pTft, "        ", TFT_X_VALUE, TFT_Y_SENSOR2_VALUE, TFT_TEXT_COLOR, TFT_BACKGROUND, true);

				AdafruitGfx_setTextSize(pTft, 1);
				Sitronix7735_Text(pTft, "Sensor error", TFT_X_VALUE, TFT_Y_SENSOR1_VALUE + 5, TFT_TEXT_COLOR, TFT_BACKGROUND, true);
				Sitronix7735_Text(pTft, "Sensor error", TFT_X_VALUE, TFT_Y_SENSOR2_VALUE + 5, TFT_TEXT_COLOR, TFT_BACKGROUND, true);
				sensorError = true;
				break;
			default:
				printfToCdc("Display loop: Unknown message\n\r");
				break;
		}
	}
}
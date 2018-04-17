#ifndef USERTASKS_H_
#define USERTASKS_H_

#include "Max31855.h"
#include "Sitronix7735.h"
#include "NvmUserConfig.h"

typedef enum TControlMessageType
{
	CM_Unknown,
	CM_PushButtonPressed,
	CM_RotaryEncoderChannelA,
	CM_RotaryEncoderChannelB,
} ControlMessageType;

typedef struct TControlMessage
{
	ControlMessageType type;
	TickType_t ticks;
} ControlMessage;

typedef enum TDisplayChangeMessageType
{
	DCM_Unknown,
	DCM_PowerLevelChanged,
	DCM_PowerOutputChanged,
	DCM_SensorDataChanged,
	DCM_SensorError,
} DisplayChangeMessageType;

typedef struct TDisplayChangeMessage
{
	DisplayChangeMessageType type;
	bool powerOn;
	int powerLevelPercent;
	float temperature1;
	float internalTemperature1;
	float temperature2;
	float internalTemperature2;
} DisplayChangeMessage;

typedef struct TAppData
{
	Sitronix7735 tft;
	struct spi_module spiModuleTempSensor;
	struct spi_slave_inst spiSlaveTempSensor1;
	struct spi_slave_inst spiSlaveTempSensor2;
	TaskHandle_t hTaskCdcLoop;
	TaskHandle_t hTaskControlProcessLoop;
	TaskHandle_t hTaskPullSensorDataLoop;
	TaskHandle_t hTaskDisplayLoop;
	bool bAutorizeCdcTransfer;
	QueueHandle_t hControlQueue;
	QueueHandle_t hTickQueue;
	QueueHandle_t hDisplayChangeQueue;
	TimerHandle_t hTempSampleTimer;
	uint32_t powerPercent;
	bool powerOn;
	uint32_t zeroCrossCount;
	NvmUserConfig nvmUserConfig;
} AppData;

// Tasks
void taskCdcLoop(void *pvParameters);
void taskControlProcessLoop(void *pvParameters);
void taskPullSensorDataLoop(void *pvParameters);
void taskDisplayLoop(void *pvParameters);

#define TFT_BACKGROUND ST7735_RGB(30, 60, 30)
#define TFT_TEXT_COLOR ST7735_RGB(1, 2, 1)

#define DEBOUNCE_TICKS 200

extern AppData g_appData;

#endif /* USERTASKS_H_ */
#include "asf.h"
#include "Max31855.h"

struct Max31855Packet
{
	uint8_t OC: 1;
	uint8_t SVG: 1;
	uint8_t SVC: 1;
	uint8_t Reserved0: 1;
	int16_t IntTempMagnitude: 12;

	uint8_t Fault: 1;
	uint8_t Reserved1: 1;
	int16_t TempMagnitude: 14;
};

bool max31855ReadData(
	struct spi_module *pSpiModuleTempSensor, 
	struct spi_slave_inst *pSpiSlaveInstance, 
	struct Max31855Data *pData)
{
	// read from SPI
	uint8_t bufferSpiRead[4];
	spi_select_slave(pSpiModuleTempSensor, pSpiSlaveInstance, true);
	spi_read_buffer_wait(pSpiModuleTempSensor, bufferSpiRead, sizeof(bufferSpiRead), 0);
	spi_select_slave(pSpiModuleTempSensor, pSpiSlaveInstance, false);

	union {
		uint8_t binary[4];
		struct Max31855Packet packet;
	} u;

	u.binary[0] = bufferSpiRead[3];
	u.binary[1] = bufferSpiRead[2];
	u.binary[2] = bufferSpiRead[1];
	u.binary[3] = bufferSpiRead[0];

	pData->Temp = (float)u.packet.TempMagnitude / 4.0;
	pData->InternalTemp = (float)u.packet.IntTempMagnitude / 16.0;
	pData->FailureType = OK;
	if (u.packet.OC) 
	{
		pData->FailureType = OpenFault;
	}
	else if (u.packet.SVG)
	{
		pData->FailureType = ShortCircuiteToGND;
	}
	else if (u.packet.SVC)
	{
		pData->FailureType = ShortCircuiteToVCC;
	}

	return !u.packet.Fault;
}


#include "asf.h"
#include "Max31855.h"
#include <math.h>

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

// reverses a string 'str' of length 'len'
void reverse(char *str, int len)
{
	int i=0, j=len-1, temp;
	while (i<j)
	{
		temp = str[i];
		str[i] = str[j];
		str[j] = temp;
		i++; j--;
	}
}

// Converts a given integer x to string str[].  d is the number
// of digits required in output. If d is more than the number
// of digits in x, then 0s are added at the beginning.
int intToStr(int x, char str[], int d)
{
	int i = 0;
	while (x)
	{
		str[i++] = (x%10) + '0';
		x = x/10;
	}
	
	// If number of digits required is more, then
	// add 0s at the beginning
	while (i < d)
	str[i++] = '0';
	
	reverse(str, i);
	str[i] = '\0';
	return i;
}

bool formatFloat(float f, char *pBuffer, int bufferSize, bool forceSign, int digits, int precision)
{
	int index = 0;
	int maxIndex = bufferSize - 1;
	if (index >= maxIndex) return false;

	// add sign
	if (forceSign)
	{
		pBuffer[index++] = (f < 0.0) ? '-' : '+';
	}
	else if (f < 0.0)
	{
		pBuffer[index++] = '-';
	}
	if (index >= maxIndex) return false;

	// add digits
	f = fabsf(f);

	// Extract integer and floating parts
    int ipart = (int)floor(f);
    float fpart = f - (float)ipart;
    
    // convert integer part to string
    int i = intToStr(ipart, pBuffer + index, digits);
    index += i;
	if (index >= maxIndex) return false;

    // check for display option after point
    if (precision != 0)
    {
	    pBuffer[index++] = '.';  // add dot
	    
	    // Get the value of fraction part upto given no.
	    // of points after dot. The third parameter is needed
	    // to handle cases like 233.007
	    fpart = fpart * pow(10, precision);
	    
	    i = intToStr((int)fpart, pBuffer + index, precision);
		index += i;
    }
	if (index >= maxIndex) return false;

	// null terminate the string
	pBuffer[index++] = 0;

	return true;
}
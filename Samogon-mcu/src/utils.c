#include "asf.h"
#include "utils.h"
#include <math.h>
#include <stdarg.h>

char intToHex(uint8_t i)
{
	return (i < 10) ? (char)(i + '0') : (char)(i - 10 + 'A');
}

bool g_enableDebugPrintToCdc = false;

void printBinaryDataToCdc(uint8_t *pBuffer, int size)
{
	if (!g_enableDebugPrintToCdc) return;
	char ach[3];
	ach[2] = ' ';
	for (int i = 0; i < size; i++) {
		ach[0] = intToHex(pBuffer[i] / 16);
		ach[1] = intToHex(pBuffer[i] % 16);
		udi_cdc_write_buf(ach, i < size-1 ? 3 : 2 );
	}
}

void printfToCdc(const char *format, ...)
{
	if (!g_enableDebugPrintToCdc) return;
	char buffer[100];
	va_list vl;
	va_start(vl, format);

	int i = vsnprintf(buffer, sizeof(buffer), format, vl);
	if (i > 0)
	{
		udi_cdc_write_buf(buffer, i);
	}

	va_end(vl);
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
	double ipartDouble = floor(f);
	int ipart = (int)ipartDouble;
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

bool appendString(char *pBuffer, int bufferSize, const char *pAppend)
{
	int len = strlen(pBuffer);
	if (len + strlen(pAppend) >= bufferSize) 
	{
		return false;
	}
	strcpy(pBuffer + len, pAppend);
	return true;
}

void clampUInt32(uint32_t *pInt, uint32_t minInt, uint32_t maxInt)
{
	if (*pInt < minInt)
	{
		*pInt = minInt;
	}
	if (*pInt > maxInt)
	{
		*pInt = maxInt;
	}
}

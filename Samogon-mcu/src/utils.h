#ifndef UTILS_H_
#define UTILS_H_

bool formatFloat(float f, char *pBuffer, int bufferSize, bool forceSign, int digits, int precision);

char intToHex(uint8_t i);
void printBinaryDataToCdc(uint8_t *pBuffer, int size);
void printfToCdc(const char *format, ...)  __attribute__ ((format (gnu_printf, 1, 2)));

bool appendString(char *pBuffer, int bufferSize, const char *pAppend);

#endif /* UTILS_H_ */
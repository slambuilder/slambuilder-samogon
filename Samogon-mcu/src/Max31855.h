#ifndef MAX31855_H_
#define MAX31855_H_

typedef enum TMax31855FailureType 
{
	OK,
	OpenFault,
	ShortCircuiteToGND,
	ShortCircuiteToVCC,
} Max31855FailureType;

typedef struct TMax31855Data 
{
	float Temp; // measured temperature in C
	float InternalTemp; // internal temperature in C
	Max31855FailureType FailureType;
} Max31855Data;

bool max31855ReadData(
	struct spi_module *pSpiModuleTempSensor, 
	struct spi_slave_inst *pSpiSlaveInstance, 
	Max31855Data *pData);

#endif /* MAX31855_H_ */
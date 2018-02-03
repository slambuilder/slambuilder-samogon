#ifndef MAX31855_H_
#define MAX31855_H_

enum Max31855FailureType 
{
	OK,
	OpenFault,
	ShortCircuiteToGND,
	ShortCircuiteToVCC,
};

struct Max31855Data 
{
	float Temp; // measured temperature in C
	float InternalTemp; // internal temperature in C
	enum Max31855FailureType FailureType;
};

bool max31855ReadData(
	struct spi_module *pSpiModuleTempSensor, 
	struct spi_slave_inst *pSpiSlaveInstance, 
	struct Max31855Data *pData);

#endif /* MAX31855_H_ */
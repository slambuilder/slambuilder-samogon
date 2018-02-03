#include "asf.h"
#include "Max31855.h"
#include "utils.h"

// This callback is triggered every ~1ms (1024 Hz)
static void callbackTcc0(struct tcc_module *const module_inst)
{
	// TODO: implement async continuation logic here.
}

static void configureTcc0(struct tcc_module *pTcc)
{
	// configure TCC0 for ~1 ms generation (1024 Hz):
	//	 - source GCLK1 (32K), prescaler 1, period 32, channel 0 trigger 15 (~50% duty cycle)
	struct tcc_config config;
	tcc_get_config_defaults(&config, CONF_BOARD_TCC_MODULE);
	config.counter.clock_source = GCLK_GENERATOR_1;
	config.counter.clock_prescaler = TCC_CLOCK_PRESCALER_DIV1;
	config.counter.period = 31;
	config.compare.wave_generation = TCC_WAVE_GENERATION_SINGLE_SLOPE_PWM;
	config.compare.match[CONF_BOARD_TCC_CHANNEL] = 16;
	config.pins.enable_wave_out_pin[CONF_BOARD_TCC_OUTPUT] = true;
	config.pins.wave_out_pin[CONF_BOARD_TCC_OUTPUT]        = CONF_BOARD_TCC_OUT_PIN;
	config.pins.wave_out_pin_mux[CONF_BOARD_TCC_OUTPUT]    = CONF_BOARD_TCC_OUT_MUX;

	tcc_init(pTcc, CONF_BOARD_TCC_MODULE, &config);
	tcc_enable(pTcc);

	// configure TCC0 callback
	tcc_register_callback(pTcc, callbackTcc0, (enum tcc_callback)CONF_BOARD_TCC_CHANNEL);
	tcc_enable_callback(pTcc, (enum tcc_callback)CONF_BOARD_TCC_CHANNEL);
}

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
	spiConfig.mux_setting = CONF_BOARD_MAX31855_SERCOM_MUX_SETTING;
	spiConfig.pinmux_pad0 = CONF_BOARD_MAX31855_SERCOM_PAD0;
	spiConfig.pinmux_pad1 = CONF_BOARD_MAX31855_SERCOM_PAD1;
	spiConfig.pinmux_pad3 = CONF_BOARD_MAX31855_SERCOM_PAD3;
	spi_init(pSpiModuleTempSensor, CONF_BOARD_MAX31855_SERCOM, &spiConfig);
	spi_enable(pSpiModuleTempSensor);
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
		int len = snprintf(printBuf, sizeof(printBuf), "temp: %s, int: %s\n\r", tempStr, internalTempStr);
		udi_cdc_write_buf(printBuf, len);
	}
	else 
	{
		udi_cdc_write_buf("Failure reading temp sensor. Failure code: ", 43);
		udi_cdc_putc(tempData.FailureType + '0');
		udi_cdc_write_buf("\n\r", 2);
	}
}

int main (void)
{
	struct tcc_module tccModule0;
	bool my_flag_autorize_cdc_transfer = true;

	system_init();
	irq_initialize_vectors();
	cpu_irq_enable();
	board_init();
	udc_start();
	delay_init();

	configureTcc0(&tccModule0);

	struct spi_module spiModuleTempSensor;
	struct spi_slave_inst spiSlaveInstance;

	configureSpiTempSensor(&spiModuleTempSensor, &spiSlaveInstance);

	while (1)
	{
		// Echo all input back to CDC port
		if (my_flag_autorize_cdc_transfer) {
			char c = udi_cdc_getc();
			if (c == 't') {
				interactiveReadTempSensor(&spiModuleTempSensor, &spiSlaveInstance);
			} else {
				// udi_cdc_putc(c + 1);
				udi_cdc_write_buf("Samogon: Unknown command: ", 26);
				udi_cdc_putc(c);
				udi_cdc_write_buf("\n\r", 2);
			}
		}
		delay_ms(10);
	}
}

bool samogon_callback_cdc_enable(void)
{
	return true;
}

void samogon_callback_cdc_disable(void)
{
}

#include <asf.h>

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

char intToHex(uint8_t i)
{
	return (i < 10) ? (char)(i + '0') : (char)(i - 10 + 'A');
}

void printBinaryDataToCdc(uint8_t *pBuffer, int size)
{
	char ach[3];
	ach[2] = ' ';
	for (int i = 0; i < size; i++) {
		ach[0] = intToHex(pBuffer[i] / 16);
		ach[1] = intToHex(pBuffer[i] % 16);
		udi_cdc_write_buf(ach, i < size-1 ? 3 : 2 );
	}
}

struct Max31855Packet
{
	uint8_t OC: 1;
	uint8_t SVG: 1;
	uint8_t SVC: 1;
	uint8_t Reserved0: 1;
	uint16_t IntTempMagnitude: 11;
	uint8_t IntTempSign: 1;

	uint8_t Fault: 1;
	uint8_t Reserved1: 1;
	uint16_t TempMagnitude: 13;
	uint8_t TempSign : 1;
};

void interactiveReadTempSensor(struct spi_module *pSpiModuleTempSensor, struct spi_slave_inst *pSpiSlaveInstance)
{
	// read from SPI
	uint8_t bufferSpiRead[4];
	spi_select_slave(pSpiModuleTempSensor, pSpiSlaveInstance, true);
	spi_read_buffer_wait(pSpiModuleTempSensor, bufferSpiRead, sizeof(bufferSpiRead), 0);
	spi_select_slave(pSpiModuleTempSensor, pSpiSlaveInstance, false);

	// print raw data
	udi_cdc_write_buf("Binary data read from MAX31855: ", 35);
	printBinaryDataToCdc(bufferSpiRead, sizeof(bufferSpiRead));
	udi_cdc_write_buf("\n\r", 2);

	union {
		uint8_t binary[4];
		struct Max31855Packet packet;
	} u;

	u.binary[0] = bufferSpiRead[3];
	u.binary[1] = bufferSpiRead[2];
	u.binary[2] = bufferSpiRead[1];
	u.binary[3] = bufferSpiRead[0];

	char printBuf[100];
	int len = snprintf(
		printBuf, 
		sizeof(printBuf), 
		"temp: %d, int: %d, failed: %d\n\r", 
		u.packet.TempMagnitude, 
		u.packet.IntTempMagnitude, 
		u.packet.Fault);
	udi_cdc_write_buf(printBuf, len);
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

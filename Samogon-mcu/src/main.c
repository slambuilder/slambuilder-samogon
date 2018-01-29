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

	struct port_config portConfig;
	port_get_config_defaults(&portConfig);
	portConfig.direction = PORT_PIN_DIR_OUTPUT;
	portConfig.powersave = false;
	port_pin_set_config(CONF_BOARD_MAX31855_SS_OUT_PIN, &portConfig);
	port_pin_set_output_level(CONF_BOARD_MAX31855_SS_OUT_PIN, true);

	configureTcc0(&tccModule0);

	/* Replace with your application code */
	while (1)
	{
		// Echo all input back to CDC port
		if (my_flag_autorize_cdc_transfer) {
			char c = udi_cdc_getc();
			udi_cdc_putc(c + 1);
		}
		// delay_ms(10);
		port_pin_toggle_output_level(CONF_BOARD_MAX31855_SS_OUT_PIN);
	}
}

bool samogon_callback_cdc_enable(void)
{
	return true;
}

void samogon_callback_cdc_disable(void)
{
}

#include <asf.h>

// DAC on PA02
#define BOARD_PINS_DAC_PORT 0
#define BOARD_PINS_DAC_PIN 2

#define CONF_PWM_MODULE			TCC0
#define CONF_PWM_CHANNEL		0
#define CONF_PWM_OUTPUT			0
#define CONF_PWM_OUT_PIN		PIN_PA04E_TCC0_WO0
#define CONF_PWM_OUT_MUX		MUX_PA04E_TCC0_WO0

// This callback is triggered every ~1ms (1024 Hz)
static void callbackTcc0(struct tcc_module *const module_inst)
{
	// TODO: implement async continuation logic here.
}

static void configureTcc0(struct tcc_module *pTcc)
{
	// configure TCC0 for ~1 ms generation (1024 Hz):
	//	 - source GCLK1 (32K), prescaler 1, period 32, channel 0 trigger 15 (~50% duty cycle)
	struct tcc_config config_tcc;
	tcc_get_config_defaults(&config_tcc, CONF_PWM_MODULE);
	config_tcc.counter.clock_source = GCLK_GENERATOR_1;
	config_tcc.counter.clock_prescaler = TCC_CLOCK_PRESCALER_DIV1;
	config_tcc.counter.period = 31;
	config_tcc.compare.wave_generation = TCC_WAVE_GENERATION_SINGLE_SLOPE_PWM;
	config_tcc.compare.match[CONF_PWM_CHANNEL] = 16;
	config_tcc.pins.enable_wave_out_pin[CONF_PWM_OUTPUT] = true;
	config_tcc.pins.wave_out_pin[CONF_PWM_OUTPUT]        = CONF_PWM_OUT_PIN;
	config_tcc.pins.wave_out_pin_mux[CONF_PWM_OUTPUT]    = CONF_PWM_OUT_MUX;
	tcc_init(pTcc, CONF_PWM_MODULE, &config_tcc);
	tcc_enable(pTcc);

	// configure TCC0 callback
	tcc_register_callback(pTcc, callbackTcc0, (enum tcc_callback)CONF_PWM_CHANNEL);
	tcc_enable_callback(pTcc, (enum tcc_callback)CONF_PWM_CHANNEL);
}

int main (void)
{
	struct tcc_module tcc_instance;
	bool my_flag_autorize_cdc_transfer = true;

	system_init();
	irq_initialize_vectors();
	cpu_irq_enable();
	board_init();
	udc_start();
	delay_init();

	PORT->Group[BOARD_PINS_DAC_PORT].DIRSET.reg	= 1 << BOARD_PINS_DAC_PIN;
	// PORT->Group[BOARD_PINS_DAC_PORT].PINCFG[PIN_PA02B_DAC_VOUT]

	configureTcc0(&tcc_instance);

	/* Replace with your application code */
	while (1)
	{
		// Echo all input back to CDC port
		 if (my_flag_autorize_cdc_transfer) {
			char c = udi_cdc_getc();
			udi_cdc_putc(c);
		 }
		 // delay_ms(10);
		PORT->Group[BOARD_PINS_DAC_PORT].OUTTGL.reg = 1 << BOARD_PINS_DAC_PIN;
	}
}

bool samogon_callback_cdc_enable(void)
{
	return true;
}

void samogon_callback_cdc_disable(void)
{
}

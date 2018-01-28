/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# Minimal main function that starts with a call to system_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>

// DAC on PA02
#define BOARD_PINS_DAC_PORT 0
#define BOARD_PINS_DAC_PIN 2

static bool my_flag_autorize_cdc_transfer = false;

//#define		PIN_PB30E_TCC0_WO0   62
//#define 	MUX_PB30E_TCC0_WO0   4

#define LED_0_PWM4CTRL_MODULE   TCC0
#define LED_0_PWM4CTRL_CHANNEL  0
#define LED_0_PWM4CTRL_OUTPUT   0
#define LED_0_PWM4CTRL_PIN		PIN_PA04E_TCC0_WO0
#define LED_0_PWM4CTRL_MUX		MUX_PA04E_TCC0_WO0
#define LED_0_PWM4CTRL_PINMUX   PINMUX_PA04E_TCC0_WO0

#define CONF_PWM_MODULE			LED_0_PWM4CTRL_MODULE
#define CONF_PWM_CHANNEL		LED_0_PWM4CTRL_CHANNEL
#define CONF_PWM_OUTPUT			LED_0_PWM4CTRL_OUTPUT
#define CONF_PWM_OUT_PIN		LED_0_PWM4CTRL_PIN
#define CONF_PWM_OUT_MUX		LED_0_PWM4CTRL_MUX

struct tcc_module tcc_instance;

static void tcc_callback_to_change_duty_cycle(struct tcc_module *const module_inst)
{
	static uint32_t delay = 10;
	static uint32_t i = 0;
	if (--delay) {
		return;
	}
	delay = 10;
	i = (i + 0x0800) & 0xFFFF;
	tcc_set_compare_value(module_inst,
		(enum tcc_match_capture_channel)
		(TCC_MATCH_CAPTURE_CHANNEL_0 + CONF_PWM_CHANNEL),
		i + 1);
}

static void configure_tcc(void)
{
	struct tcc_config config_tcc;
	tcc_get_config_defaults(&config_tcc, CONF_PWM_MODULE);
	config_tcc.counter.period = 0xFFFF;
	config_tcc.compare.wave_generation = TCC_WAVE_GENERATION_SINGLE_SLOPE_PWM;
	config_tcc.compare.match[CONF_PWM_CHANNEL] = 0xFFFF;
	config_tcc.pins.enable_wave_out_pin[CONF_PWM_OUTPUT] = true;
	config_tcc.pins.wave_out_pin[CONF_PWM_OUTPUT]        = CONF_PWM_OUT_PIN;
	config_tcc.pins.wave_out_pin_mux[CONF_PWM_OUTPUT]    = CONF_PWM_OUT_MUX;
	tcc_init(&tcc_instance, CONF_PWM_MODULE, &config_tcc);
	tcc_enable(&tcc_instance);
}

static void configure_tcc_callbacks(void)
{
	tcc_register_callback(
		&tcc_instance,
		tcc_callback_to_change_duty_cycle,
		(enum tcc_callback)(TCC_CALLBACK_CHANNEL_0 + CONF_PWM_CHANNEL));
	tcc_enable_callback(
		&tcc_instance,
		(enum tcc_callback)(TCC_CALLBACK_CHANNEL_0 + CONF_PWM_CHANNEL));
}

int main (void)
{
	system_init();
	irq_initialize_vectors();
	cpu_irq_enable();
	board_init();
	udc_start();
	delay_init();

	PORT->Group[BOARD_PINS_DAC_PORT].DIRSET.reg	= 1 << BOARD_PINS_DAC_PIN;
	// PORT->Group[BOARD_PINS_DAC_PORT].PINCFG[PIN_PA02B_DAC_VOUT]

	 configure_tcc();
	 configure_tcc_callbacks();

	/* Replace with your application code */
	while (1)
	{
		// Echo all input back to CDC port
		 if (my_flag_autorize_cdc_transfer) {
			char c = udi_cdc_getc();
			udi_cdc_putc(c);
		 }
		 delay_ms(10);
		PORT->Group[BOARD_PINS_DAC_PORT].OUTTGL.reg = 1 << BOARD_PINS_DAC_PIN;
	}
}

bool bldecent_callback_cdc_enable(void)
{
	my_flag_autorize_cdc_transfer = true;
	return true;
}
void bldecent_callback_cdc_disable(void)
{
	my_flag_autorize_cdc_transfer = false;
}

#ifndef SITRONIX7735_H_
#define SITRONIX7735_H_

struct Sitronix7735Instance
{
	// PWM enabled light control. 1 - full brightness, 0 - no light.
	uint8_t litePin;
	// Low enable chip select.
	uint8_t chipSelectPin;
	// Low enable micro-ssd card chip select.
	uint8_t cardChipSelectPin;
	// High - display data or parameter, low - command data
	uint8_t dcSelectorPin;
	// Active low.
	uint8_t resetPin;

	// SPI configuration
	enum spi_signal_mux_setting
	uint32_t pinmux_pad0;
	uint32_t pinmux_pad1;
	uint32_t pinmux_pad2;
	uint32_t pinmux_pad3;

	Sercom const *sercomHw;
};

void Sitronix7735_GetConfigDefaults(Sitronix7735Instance *instance);
void Sitronix7735_Initialize(Sitronix7735Instance *instance);
void Sitronix7735_Reset(Sitronix7735Instance *instance);
void Sitronix7735_LightOn(Sitronix7735Instance *instance);
void Sitronix7735_LightOff(Sitronix7735Instance *instance);


#endif /* SITRONIX7735_H_ */
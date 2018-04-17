/**
 * \file
 *
 * \brief User board configuration template
 *
 * Copyright (C) 2013-2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#ifndef CONF_BOARD_H
#define CONF_BOARD_H

#define CONF_BOARD_USB_PORT

// GPIO (IOPORT) output signal for SSR
#define CONF_BOARD_SSR_OUT IOPORT_CREATE_PIN(IOPORT_PORTB, 8)

// GPIO Inverted Slave Select Pin (!SS) on PA18
#define CONF_BOARD_MAX31855_SENSOR1_SS_OUT_PIN PIN_PA18
#define CONF_BOARD_MAX31855_SENSOR2_SS_OUT_PIN PIN_PB09

#define CONF_BOARD_MAX31855_SERCOM				SERCOM1
#define CONF_BOARD_MAX31855_SERCOM_MUX_SETTING	SPI_SIGNAL_MUX_SETTING_D
// MOSI
#define CONF_BOARD_MAX31855_SERCOM_PAD0			PINMUX_PA16C_SERCOM1_PAD0
// SCK
#define CONF_BOARD_MAX31855_SERCOM_PAD1			PINMUX_PA17C_SERCOM1_PAD1
// Do not uncomment this line -- pad2 is not used!!!
// #define CONF_BOARD_MAX31855_SERCOM_PAD2			PINMUX_PA18C_SERCOM1_PAD2
// MISO
#define CONF_BOARD_MAX31855_SERCOM_PAD3			PINMUX_PA19C_SERCOM1_PAD3

// push button interrupt
#define CONF_BOARD_PUSH_BUTTON_EXTINT_PIN			PIN_PA02A_EIC_EXTINT2
#define CONF_BOARD_PUSH_BUTTON_EXTINT_MUX			MUX_PA02A_EIC_EXTINT2
#define CONF_BOARD_PUSH_BUTTON_EIC_LINE				2

// rotary encoder interrupts
#define CONF_BOARD_RENCODER_CHANNEL_A_EXTINT_PIN	PIN_PA03A_EIC_EXTINT3
#define CONF_BOARD_RENCODER_CHANNEL_A_EXTINT_MUX	MUX_PA03A_EIC_EXTINT3
#define CONF_BOARD_RENCODER_CHANNEL_A_EIC_LINE		3

#define CONF_BOARD_RENCODER_CHANNEL_B_EXTINT_PIN	PIN_PA04A_EIC_EXTINT4
#define CONF_BOARD_RENCODER_CHANNEL_B_EXTINT_MUX	MUX_PA04A_EIC_EXTINT4
#define CONF_BOARD_RENCODER_CHANNEL_B_EIC_LINE		4

// zero cross interrupt
#define CONF_BOARD_ZERO_CROSS_EXTINT_PIN			PIN_PA14A_EIC_EXTINT14
#define CONF_BOARD_ZERO_CROSS_EXTINT_MUX			MUX_PA14A_EIC_EXTINT14
#define CONF_BOARD_ZERO_CROSS_EIC_LINE				14

// TFT display Lite (GPIO OUT)
#define CONF_BOARD_TFT_LITE_OUT_PIN				PIN_PA15

// TFT display \CS (GPIO OUT)
#define CONF_BOARD_TFT_CS_OUT_PIN				PIN_PA11

// TFT display Card \CS (GPIO OUT)
#define CONF_BOARD_TFT_CARD_CS_OUT_PIN			PIN_PA10

// TFT display D/C selector (GPIO OUT)
#define CONF_BOARD_TFT_DC_SELECT_OUT_PIN		PIN_PA20

// TFT display RST (GPIO OUT)
#define CONF_BOARD_TFT_RST_OUT_PIN				PIN_PA21


#define CONF_BOARD_TFT_SERCOM					SERCOM0
#define CONF_BOARD_TFT_SERCOM_MUX_SETTING		SPI_SIGNAL_MUX_SETTING_E
// MISO
#define CONF_BOARD_TFT_SERCOM_PAD0				PINMUX_PA08C_SERCOM0_PAD0
// Not used
#define CONF_BOARD_TFT_SERCOM_PAD1				PINMUX_DEFAULT
// MOSI
#define CONF_BOARD_TFT_SERCOM_PAD2				PINMUX_PA06D_SERCOM0_PAD2
// SCK
#define CONF_BOARD_TFT_SERCOM_PAD3				PINMUX_PA07D_SERCOM0_PAD3

#define CONF_BOARD_TFT_SERCOM_BAUDRATE			10000000
#endif // CONF_BOARD_H

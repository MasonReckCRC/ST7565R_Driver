
/*
 * ST7565R.h
 *
 * Created: 10/26/2022 4:03:16 PM
 *  Author: Mason Reck
 */

///*****************************************************************************
 //*
/// Program for writing to NHD-C12832A1Z display Series with the ST7565R Controller.
///
/// Newhaven Display invests time and resources providing this open source code,
/// Please support Newhaven Display by purchasing products from Newhaven Display!
//
//* Copyright (c) 2019, Newhaven Display International
//*
//* This code is provided as an example only and without any warranty by Newhaven Display.
//* Newhaven Display accepts no responsibility for any issues resulting from its use.
//* The developer of the final application incorporating any parts of this
//* sample code is responsible for ensuring its safe and correct operation
//* and for any consequences resulting from its use.
//* See the GNU General Public License for more details.
//*
//* Use Vertical Orientation when converting BMP to hex code to display custom image using LCD assistant.
//
//
//*
//*****************************************************************************/

#ifndef ST7565R_H_
#define ST7565R_H_


#define ST7565R_USING_STM
//#define ST7565R_USING_ATMEL

#include <stdbool.h>

#ifdef ST7565R_USING_STM
#include "spi.h"
#endif



/*****************************************************
*            Pre-Processor Directives				 *
*****************************************************/
#define SCREENWIDTH		128
#define SCREENHEIGHT	32
#define NUM_PAGES		4

#define LOW 			GPIO_PIN_RESET
#define HIGH 			GPIO_PIN_SET


#define font_num_bytes_per_char(width, height) 		(((int)width/8) * height)
#define font_num_bytes_per_row(width) 				((int)width / 8)

#ifdef ST7565R_USING_STM
#define digital_write(portPin, highLow) HAL_GPIO_WritePin(portPin.port, portPin.pin, highLow)
#endif
#ifdef ST7565R_USING_ATMEL
	//TODO: configure for Atmel
#endif


/****************************************************
*              Commands				                *
****************************************************/
#define ST7565R_CMD_DISPLAY_ON                      0xAF
#define ST7565R_CMD_DISPLAY_OFF                     0xAE
#define ST7565R_CMD_START_LINE_SET(line)            (0x40 | (line))
#define ST7565R_CMD_PAGE_ADDRESS_SET(page)          (0xB0 | (page))
#define ST7565R_CMD_COLUMN_MSB(column)  			(0x10 | (column))
#define ST7565R_CMD_COLUMN_LSB(column)  			(0x00 | (column))
#define ST7565R_CMD_MSB								0x10
#define ST7565R_CMD_LSB								0x00
#define ST7565R_CMD_ADC_NORMAL                     	0xA0
#define ST7565R_CMD_ADC_REVERSE                     0xA1
#define ST7565R_CMD_DISPLAY_NORMAL                  0xA6
#define ST7565R_CMD_DISPLAY_REVERSE                 0xA7
#define ST7565R_CMD_DISPLAY_ALL_POINTS_OFF          0xA4
#define ST7565R_CMD_DISPLAY_ALL_POINTS_ON           0xA5
#define ST7565R_CMD_LCD_BIAS_1_DIV_5_DUTY33         0xA1
#define ST7565R_CMD_LCD_BIAS_1_DIV_6_DUTY33         0xA2
#define ST7565R_CMD_NORMAL_SCAN_DIRECTION           0xC0
#define ST7565R_CMD_REVERSE_SCAN_DIRECTION          0xC8
#define ST7565R_CMD_VOLTAGE_RESISTOR_RATIO_0        0x20
#define ST7565R_CMD_VOLTAGE_RESISTOR_RATIO_1        0x21
#define ST7565R_CMD_VOLTAGE_RESISTOR_RATIO_2        0x22
#define ST7565R_CMD_VOLTAGE_RESISTOR_RATIO_3        0x23
#define ST7565R_CMD_VOLTAGE_RESISTOR_RATIO_4        0x24
#define ST7565R_CMD_VOLTAGE_RESISTOR_RATIO_5        0x25
#define ST7565R_CMD_VOLTAGE_RESISTOR_RATIO_6        0x26
#define ST7565R_CMD_VOLTAGE_RESISTOR_RATIO_7        0x27
#define ST7565R_CMD_POWER_CTRL_ALL_ON               0x2F
#define ST7565R_CMD_SLEEP_MODE                      0xAC
#define ST7565R_CMD_NORMAL_MODE                     0xAD
#define ST7565R_CMD_RESET                           0xE2
#define ST7565R_CMD_NOP                             0xE3
#define ST7565R_CMD_ELECTRONIC_VOLUME_MODE_SET      0x81
#define ST7565R_CMD_ELECTRONIC_VOLUME(volume)       (0x3F & (~volume))
#define ST7565R_CMD_BOOSTER_RATIO_SET               0xF8
#define ST7565R_CMD_BOOSTER_RATIO_2X_3X_4X          0x00
#define ST7565R_CMD_BOOSTER_RATIO_5X                0x01
#define ST7565R_CMD_BOOSTER_RATIO_6X                0x03
#define ST7565R_CMD_STATUS_READ                     0x00
#define ST7565R_CMD_END                             0xEE
#define ST7565R_CMD_READ_MODIFY_WRITE               0xE0


/****************************************************
*           Enumerations and Structures             *
****************************************************/

#ifdef ST7565R_USING_STM
typedef struct ST7565R_STM_Pin {
	GPIO_TypeDef* port;
	uint16_t pin;
}ST7565R_STM_Pin;
#endif

typedef struct ST7565R_Font{
	uint8_t* glyphs;
	uint8_t width;
	uint8_t height;
	char firstChar;
	char lastChar;
} ST7565R_Font;

const enum{
	ERASE,
	DRAW
};

/****************************************************
*                 Functions			                *
****************************************************/

void ST7565R_command(uint8_t command);
void ST7565R_drawByte(uint8_t byte);
void ST7565R_setByte(unsigned column, unsigned page, uint8_t byte);
void ST7565R_paintPixel(unsigned x, unsigned y, bool newLevel);
void ST7565R_paintString(char* string, unsigned x, unsigned y);
void ST7565R_paintChar(char c, unsigned x, unsigned y);
void ST7565R_drawFullscreenBitmap(uint8_t* bitmap);
void ST7565R_clearScreen(void);


void ST7565R_configureFont(ST7565R_Font newFont);

void ST7565R_init_LCD(void);
void ST7565R_setup(void);

void setBacklightNHD(uint8_t brightness);
void blinkBacklightNHD(uint8_t oscillation);

void screenTest(void);


/*************************************************************************************************************************\
|   Bitmaps											   																	  |
|*************************************************************************************************************************|
|   	Find bitmaps in the following bitmaps.h file */#pragma BITMAPS_H /* (ctrl+click) to nav there					  |
\*************************************************************************************************************************/
#endif







































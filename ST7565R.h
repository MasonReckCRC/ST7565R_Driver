
/*
 * ST7565R.h
 *
 * Created: 10/26/2022 4:03:16 PM
 *  Author: Mason Reck
 */

///*****************************************************************************
/*
 The following note is from the driver that I based this driver off of from New Haven Displays
 https://support.newhavendisplay.com/hc/en-us/articles/4415264814231-NHD-C12832A1Z-with-Arduino
 Some elements of the original driver may remain within this code, and thus this note from NHD
 has been left in.
*/

/* Program for writing to NHD-C12832A1Z display Series with the ST7565R Controller.

 Newhaven Display invests time and resources providing this open source code,
 Please support Newhaven Display by purchasing products from Newhaven Display!

* Copyright (c) 2019, Newhaven Display International
*
* This code is provided as an example only and without any warranty by Newhaven Display.
* Newhaven Display accepts no responsibility for any issues resulting from its use.
* The developer of the final application incorporating any parts of this
* sample code is responsible for ensuring its safe and correct operation
* and for any consequences resulting from its use.
* See the GNU General Public License for more details.
*
* Use Vertical Orientation when converting BMP to hex code to display custom image using LCD assistant.
//*****************************************************************************/




#ifndef ST7565R_H_
#define ST7565R_H_

#include <stdbool.h>



/***** CONFIGURE ME! *****/
#define ST7565R_USING_STM
//#define ST7565R_USING_ATMEL
//#define ST7565R_USING_CUSTOM
/***** CONFIGURE ME! *****/

#ifdef ST7565R_USING_STM
#include "main.h"
#include "spi.h"
#endif



/*****************************************************
*     Configurable Pre-Processor Directives			 *
/**********************************************************************************************************************************************************************\
|		DEFINITION NAME							VALUE TYPE			VALUE 								SUGGESTED RANGE						DEFAULT VALUE			   |
\**********************************************************************************************************************************************************************/
//#define PAINT_IMMEDIATELY					 // Definition			UN/COMMENTED						 UN/COMMENTED						UNCOMMENTED
#define SCREENWIDTH								((uint16_t) 		128									)// 1 - 5000						128 pixels
#define SCREENHEIGHT							((uint16_t)			32									)// 1 - 5000						32 	pixels

#if defined(ST7565R_USING_STM)
#define	ST7565R_PWM_TIMER_CHANNEL									TIM2->CCR1							 // Configured Capture/Compare Register for a PWM
#define ST7565R_SPI													hspi3								 // Configured spi for STM
#endif

/**********************************************************************************************************************************************************************\
*   PROGRAM SETTINGS' DESCRIPTIONS																																	   *
/**********************************************************************************************************************************************************************\
*/  #pragma PAINT_IMMEDIATELY																																          /*
*\		Comment out PAINT_IMMEDIATELY to use this driver in a different way. If you comment this out when you call the paint functions								  \*
*\		it will only add them to the curScreen data structure. The driver will only paint to the display when you call ST7565R_updateDisplay();					 	  \*
*\		One thing you can do is set up an interrupt on a timer to give our screen a specified frame rate. This reduces "flashing"				 					  \*
*/  #pragma SCREENWIDTH 																																 			  /*
*\		Configure this to the width of your screen in pixels																						  				  \*
*/  #pragma SCREENHEIGHT																																  			  /*
*\		Configure this to the height of your screen in pixels																										  \*
*/	#pragma ST7565R_PWM_TIMER_CHANNEL																														 		  /*
*\		STM: Configure this to the Capture/Compare Register in your timer that you have configured Pulse Width Modulation for. This is for the backlight.   				  \*
*/  #pragma ST7565R_SPI																																				  /*
*\		STM: Configure this to the spi structure thats configured to your screen.																	  								  \*
\**********************************************************************************************************************************************************************/


#if defined(ST7565R_USING_STM)
#define ST7565R_set_pwm(dutyCycle)							ST7565R_PWM_TIMER_CHANNEL = dutyCycle 									/*TODO: Configure Me */
#define ST7565R_spi_transmit(data)							HAL_SPI_Transmit(&ST7565R_SPI, &data, 1, HAL_MAX_DELAY)		/*TODO: Configure Me */

#define ST7565R_digital_write(portPin, highLow) 			HAL_GPIO_WritePin(portPin.port, portPin.pin, highLow)
#define ST7565R_delay(delayTime)							HAL_Delay(delayTime)
#elif defined(ST7565R_USING_ATMEL)
#define ST7565R_set_pwm(dutyCycle)							/*TODO: Configure for Atmel*/
#define ST7565R_digital_write(portPin, highLow) 			/*TODO: Configure for Atmel*/
#define ST7565R_delay(delayTime)							delay_ms(delayTime);
#define ST7565R_spi_transmit(spi, data, size, timeout)		/*TODO: Configure for Atmel*/
#else
#define ST7565R_set_pwm(dutyCycle)							/*TODO: Configure this function to your own architecture*/
#define ST7565R_digital_write(portPin, highLow) 			/*TODO: Configure this function to your own architecture*/
#define ST7565R_delay(delayTime)							/*TODO: Configure this function to your own architecture*/
#define ST7565R_spi_transmit(spi, data, size, timeout)		/*TODO: Configure this function to your own architecture*/
#endif

/*****************************************************
*     Non-Configurable Pre-Processor Directives		 *
*****************************************************/

// Macros
#define font_num_bytes_per_row(width) 						(width % 8 == 0 ? ((int)width / 8) : (1 + ((int)width / 8)))
#define font_num_bytes_per_char(width, height) 				(font_num_bytes_per_row(width) * height)
#define ST7565R_num_pages_from_height(height)				((height/8) + (height%8==0 ? 0 : 1))

// Definitions
#define SCREENPAGES											ST7565R_num_pages_from_height(SCREENHEIGHT)
#define SCREENBYTES											SCREENPAGES * SCREENWIDTH

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

#if defined(ST7565R_USING_STM)
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

typedef const enum{
	ERASE,
	DRAW
} ST7565R_DrawState;

typedef const enum
{
  LOW = 0U,
  HIGH
} ST7565R_PinState;

/****************************************************
*        Function Prototypes               		    *
****************************************************/

// ST7565R functions
void ST7565R_command(uint8_t command);
void ST7565R_paintByteHere(uint8_t byte);
void ST7565R_paintByte(uint8_t byte, unsigned column, unsigned page);
void ST7565R_paintPixel(ST7565R_DrawState drawOrErase, unsigned x, unsigned y);
void ST7565R_paintString(char* string, unsigned x, unsigned y);
void ST7565R_paintChar(char c, unsigned x, unsigned y);
void ST7565R_paintFullscreenBitmap(uint8_t* bitmap);
void ST7565R_paintBitmap(uint8_t* bitmap, unsigned width, unsigned height, unsigned x, unsigned y);
void ST7565R_paintRectangle(ST7565R_DrawState drawOrErase, unsigned x, unsigned y, unsigned width, unsigned height);
void ST7565R_clearScreen(void);
void ST7565R_updateDisplay(void);
void ST7565R_initScreen(void);
void ST7565R_setup(void);

// Font Functions
void ST7565R_configureFont(ST7565R_Font newFont);
void ST7565R_configureFontDefault(void);
void ST7565R_configureFontFlow(void);
/* To use custom fonts, you will need to make and pass
 * your own font structure */ #pragma ST7565R_Font /*
 * */
 
// Private Functions
static void ST7565R_paintCurScreen(void);
static void ST7565R_addCharToCurScreen(char c, unsigned x, unsigned y);
static void ST7565R_addPixelToCurScreen(ST7565R_DrawState  drawOrErase, unsigned x, unsigned y);

// Backlight functions
void ST7565R_setBacklight(uint8_t brightness);
void ST7565R_blinkBacklight(uint8_t oscillation);


// Test Functions
void ST7565R_screenTest(void);


/*************************************************************************************************************************\
|   Bitmaps											   																	  |
|*************************************************************************************************************************|
|   	Find bitmaps in the following bitmaps.h file */ #pragma BITMAPS_H /* (ctrl+click) to nav there					  |
\*************************************************************************************************************************/
#endif












































/*
 ***********************************************************************************************************************************************************************
	Author:		Mason Reck
 ***********************************************************************************************************************************************************************
	Description:
		Customized ST7565R Driver for our NHD-C12832A1Z-FSW-FBW-3V3 screen

	Driver Info:
		https://www.edeca.net/pages/the-st7565-display-controller/
	Original ST7565R Driver written by NewHaven Displays
		https://support.newhavendisplay.com/hc/en-us/articles/4415264814231-NHD-C12832A1Z-with-Arduino
	Store page for screen I am using:
		https://www.digikey.com/en/products/detail/newhaven-display-intl/NHD-C12832A1Z-FSW-FBW-3V3/2059236
	Data Sheet for screen I am using:
		https://newhavendisplay.com/content/specs/NHD-C12832A1Z-FSW-FBW-3V3.pdf
 ***********************************************************************************************************************************************************************
 */

#include <string.h>

#include "ST7565R.h"
#include "Fonts/crcFont.h"
#include "Fonts/flowFont.h"
#include "Fonts/kleinFont.h"


/****************************************************
*               PINS					            *
*****************************************************
***** CONFIGURE THESE PINS TO MATCH YOUR PINOUT *****
****************************************************/
#if defined(ST7565R_USING_STM)
const ST7565R_STM_Pin NHD_LED = {.port = NHD_LED_GPIO_Port, .pin = NHD_LED_Pin};		//PWM Voltage Source for Back Light
const ST7565R_STM_Pin NHD_RES = {.port = NHD_RES_GPIO_Port, .pin = NHD_RES_Pin};		//Reset signal
const ST7565R_STM_Pin NHD_A0  = {.port = NHD_A0_GPIO_Port , .pin = NHD_A0_Pin };		//Register select signal (Command = Low, Data=High)
const ST7565R_STM_Pin NHD_SDA = {.port = NHD_SDA_GPIO_Port, .pin = NHD_SDA_Pin};		//(MOSI) Serial data signal
const ST7565R_STM_Pin NHD_CS  = {.port = NHD_CS_GPIO_Port , .pin = NHD_CS_Pin };		//Chip select signal
const ST7565R_STM_Pin NHD_SCL = {.port = NHD_SCL_GPIO_Port, .pin = NHD_SCL_Pin};		//Serial clock signal
#elif defined(ST7565R_USING_ATMEL)
#define NHD_LED								IOPORT_CREATE_PIN(PORTD, 0)		// Voltage Source for Back Light
#define NHD_RES								IOPORT_CREATE_PIN(PORTD, 3)		//Reset signal
#define NHD_A0								IOPORT_CREATE_PIN(PORTD, 4)		//Register select signal
#define NHD_SDA								IOPORT_CREATE_PIN(PORTD, 5)		//Serial data signal
#define NHD_CS								IOPORT_CREATE_PIN(PORTD, 6)		//Chip select signal
#define NHD_SCL								IOPORT_CREATE_PIN(PORTD, 7)		//Serial clock signal
#endif

/****************************************************
*        Current Screen					            *
****************************************************/

#ifndef PAINT_IMMEDIATELY
static uint8_t* lastScreen;
#endif
static uint8_t* curScreen;
static ST7565R_Font curFont;

/****************************************************
*        Display Functions		                    *
****************************************************/
void ST7565R_command(uint8_t cmd)
{	// This function sends a command to the screen. See list of commands in ST7565R.h
	ST7565R_digital_write(NHD_CS, LOW);		// Set Chip Select to Low to begin transmission over SPI
	ST7565R_digital_write(NHD_A0, LOW);		// Set AO Low to specify a Command Transmission
	ST7565R_spi_transmit(cmd);				// Transmit command via SPI
	ST7565R_digital_write(NHD_CS, HIGH);	// Set Chip Select to High to signal end of transmission
}

void ST7565R_paintByteHere(uint8_t byte)
{	// This function simply draws the byte as is in the preselected location
	ST7565R_digital_write(NHD_CS, LOW);		// Set Chip Select to Low to signal beginning of transmission over SPI
	ST7565R_digital_write(NHD_A0, HIGH);	// Set AO High to specify a data transmission
	ST7565R_spi_transmit(byte); 			// Transmit data byte through SPI
	ST7565R_digital_write(NHD_CS, HIGH);	// Set Chip Select to High to signal end of transmission
}

void ST7565R_paintByte(uint8_t byte, unsigned column, unsigned page)
{	// Paint a byte of data at a specified column and page (Columns are along x-axis, Pages are along y-axis in groups of 8)
	if(column >= SCREENWIDTH){return;}
	if(page >= SCREENPAGES)	 {return;}
	int byteIndex = (SCREENWIDTH*page)+column;
	curScreen[byteIndex] = byte;
	uint8_t colMSB = column/0x10;
	uint8_t colLSB = column%0x10;

	ST7565R_command(ST7565R_CMD_DISPLAY_OFF);		        // Set Display OFF
	ST7565R_command(ST7565R_CMD_PAGE_ADDRESS_SET(page));    // Specify which page to draw to
	ST7565R_command(ST7565R_CMD_COLUMN_MSB(colMSB));	    // Specify which column to draw to, upper 4 bits + 0x10
	ST7565R_command(ST7565R_CMD_COLUMN_LSB(colLSB));	    // Specify which column to draw to, lower 4 bits + 0x00
	ST7565R_paintByteHere(byte);							// Paint the byte passed to the function
	ST7565R_command(ST7565R_CMD_DISPLAY_ON);				// Set Display ON
}

void ST7565R_paintPixel(ST7565R_DrawState drawOrErase, unsigned x, unsigned y)
{	// Paint an individual pixel at a specified (x,y) coordinate.  DON'T FORGET TO updateDisplay()
	if(x >= SCREENWIDTH) {return;}
	if(y >= SCREENHEIGHT){return;}
	int byteIndex = (SCREENWIDTH*(y/8))+x;
	uint8_t colMSB = x/0x10;
	uint8_t colLSB = x%0x10;

	ST7565R_addPixelToCurScreen(drawOrErase, x, y);
	ST7565R_command(ST7565R_CMD_DISPLAY_OFF);		    // Set Display OFF
	ST7565R_command(ST7565R_CMD_PAGE_ADDRESS_SET(y/8)); // Specify which page to draw to
	ST7565R_command(ST7565R_CMD_COLUMN_MSB(colMSB));	// Specify which column to draw to, upper 4 bits + 0x10
	ST7565R_command(ST7565R_CMD_COLUMN_LSB(colLSB));	// Specify which column to draw to, lower 4 bits + 0x00
	ST7565R_paintByteHere(curScreen[byteIndex]);						// Paint the new byte with the new pixel
	ST7565R_command(ST7565R_CMD_DISPLAY_ON);			// Set Display ON
}

void ST7565R_paintString(char* string, unsigned x, unsigned y)
{	// Paint a string of characters at a specified (x,y) coordinate. DON'T FORGET TO updateDisplay()
	if(string == NULL)	 {return;}
	if(x >= SCREENWIDTH) {return;}
	if(y >= SCREENHEIGHT){return;}
	unsigned originalX = x;

	for(int i = 0; i < strlen(string); i ++){

		// Special Characters
		switch(string[i]){
		case '\n': // New Line
			y += curFont.height+1;
			continue;
		case '\r': // Carriage Return
			x = originalX;
			continue;
		case '\t': // Tab
			x += 2*curFont.width;
			continue;
		case '\0': // End of String
			continue;
		}

#ifdef PAINT_IMMEDIATELY
		ST7565R_paintChar(string[i], x, y);
#else
		ST7565R_addCharToCurScreen(string[i], x, y);
#endif
		x += curFont.width;
	}
}

void ST7565R_paintChar(char c, unsigned x, unsigned y)
{	// Paint an individual character at a specified (x,y) coordinate
	unsigned originalX = x;
	unsigned bytesPerRow = font_num_bytes_per_row(curFont.width);
	unsigned bytesPerChar = font_num_bytes_per_char(curFont.width, curFont.height);
	unsigned startIndex = (c - curFont.firstChar) * bytesPerChar;
	unsigned endIndex = startIndex + bytesPerChar;
	unsigned iterateRowTest = 0;
	unsigned width = 0;

	// Loop for all the bytes
	for(int i = startIndex; i < endIndex; i ++){
		uint8_t charByte = curFont.glyphs[i];

		//Loop for each individual byte
		for(int j = 0; j < 8; j ++){
			width = x - originalX;
			if(width < curFont.width){
				bool drawOrErase = (0b10000000 & (charByte<<j)) != 0;
#ifdef PAINT_IMMEDIATELY
				ST7565R_paintPixel(drawOrErase, x, y);
#else
				ST7565R_addPixelToCurScreen(drawOrErase, x, y);
#endif
			}
			x++;
		}

		//Test for Next Line/Row
		iterateRowTest++;
		if(iterateRowTest % bytesPerRow == 0){
			x = originalX;
			y++;
		}
	}
}

void ST7565R_paintFullscreenBitmap(uint8_t* bitmap)
{// Paint a bitmap that matches the size of the screen,  DON"T FORGET TO updateDisplay()
	if(bitmap == NULL){bitmap = bmp_clear();}					// Catch Null Pointers
	for(int i = 0; i < SCREENBYTES; i++){
		curScreen[i] = bitmap[i];
	} 	// Set the curScreen to the new bitmap

#ifdef PAINT_IMMEDIATELY
	ST7565R_paintCurScreen();
#endif
}

void ST7565R_paintBitmap(uint8_t* bitmap, unsigned width, unsigned height, unsigned x, unsigned y)
{	// Paint a bitmap to a specified (x,y) coordinate of the screen
	if(bitmap == NULL)	  {return;}
	if(x >= SCREENWIDTH)  {return;}
	if(y >= SCREENHEIGHT) {return;}

	unsigned originalX = x;
	unsigned x2 = x + width;
	unsigned y2 = y + height;
	unsigned pages = ST7565R_num_pages_from_height(height);

	for(int i = x; i < x2; i++){
		for(int j = y; j < y2; j ++){

#ifdef PAINT_IMMEDIATELY
			// TODO: Finish Implement paint Bitmap
#else
			// TODO: Finish Implement paint Bitmap
#endif
		}
	}
}

void ST7565R_paintRectangle(ST7565R_DrawState drawOrErase, unsigned x, unsigned y, unsigned width, unsigned height){
	if(x >= SCREENWIDTH  || width > SCREENWIDTH) {return;}
	if(y >= SCREENHEIGHT || height > SCREENHEIGHT){return;}
	unsigned originalX = x;
	unsigned x2 = x + width;
	unsigned y2 = y + height;

	for(int i = x; i < x2; i++){
		for(int j = y; j < y2; j ++){
#ifdef PAINT_IMMEDIATELY
			ST7565R_paintPixel(drawOrErase, i, j);
#else
			ST7565R_addPixelToCurScreen(drawOrErase, i, j);
#endif
		}
	}
}


void ST7565R_clearScreen(void)
{	// Erase the entire screen
	ST7565R_paintFullscreenBitmap(bmp_clear());
	ST7565R_updateDisplay();
}

void ST7565R_updateDisplay(void){
#ifndef PAINT_IMMEDIATELY
	ST7565R_paintCurScreen();
	for(int i = 0; i < SCREENBYTES; i++){
		lastScreen[i] = curScreen[i];
		curScreen[i] = 0x00;
	} 	// Record the displayed screen and reset the current screen data-structure

#endif
}


/****************************************************
*   Functions not to be referenced outside driver   *
****************************************************/
static void ST7565R_paintCurScreen(void){
	uint8_t page = 0;
	uint8_t column = 0;
	uint8_t colMSB = column/0x10;
	uint8_t colLSB = column%0x10;
	int byteIndex = (SCREENWIDTH*page)+column;

	ST7565R_command(ST7565R_CMD_DISPLAY_OFF);					// Set Display OFF
	for(int i=0; i<SCREENPAGES; i++){							// 32 pixel display / 8 pixels per page = 4 pages
		ST7565R_command(ST7565R_CMD_PAGE_ADDRESS_SET(page));	// Send page address
		for(int j=0; j<SCREENWIDTH; j++){	// 128 columns wide

#ifndef PAINT_IMMEDIATELY
			if(curScreen[byteIndex] == lastScreen[byteIndex]){		// Only paint if there's something new
				column++;
				byteIndex++;
				continue;
			}
#endif

			colMSB = column/0x10;
			colLSB = column%0x10;
			ST7565R_command(ST7565R_CMD_COLUMN_MSB(colMSB));// Specify which column to draw to, upper 4 bits + 0x10
			ST7565R_command(ST7565R_CMD_COLUMN_LSB(colLSB));// Specify which column to draw to, lower 4 bits + 0x00
			ST7565R_paintByteHere(curScreen[byteIndex]);		// Send bitmap byte

			column++;
			byteIndex++;
		}
		page++;		// After drawing, go to next page
		column = 0;
	}
	ST7565R_command(ST7565R_CMD_DISPLAY_ON);			// Set Display ON
}
static void ST7565R_addCharToCurScreen(char c, unsigned x, unsigned y){
	unsigned originalX = x;
	unsigned bytesPerRow = font_num_bytes_per_row(curFont.width);
	unsigned bytesPerChar = font_num_bytes_per_char(curFont.width, curFont.height);
	unsigned startIndex = (c - curFont.firstChar) * bytesPerChar;
	unsigned endIndex = startIndex + bytesPerChar;
	unsigned iterateRowTest = 0;
	unsigned width = 0;

	// Loop for all the bytes
	for(int i = startIndex; i < endIndex; i ++){
		uint8_t charByte = curFont.glyphs[i];

		//Loop for each individual byte
		for(int j = 0; j < 8; j ++){
			width = x - originalX;
			if(width < curFont.width){
				bool drawOrErase = (0b10000000 & (charByte<<j)) != 0;
				ST7565R_addPixelToCurScreen(drawOrErase, x, y);
			}
			x++;
		}

		//Test for Next Line/Row
		iterateRowTest++;
		if(iterateRowTest % bytesPerRow == 0){
			x = originalX;
			y++;
		}
	}
}
static void ST7565R_addPixelToCurScreen(ST7565R_DrawState drawOrErase, unsigned x, unsigned y){
	if(x >= SCREENWIDTH) {return;}
	if(y >= SCREENHEIGHT){return;}
	int byteIndex = (SCREENWIDTH*(y/8))+x;
	uint8_t newByte = curScreen[byteIndex];
	if(drawOrErase){
		newByte |=  (0b00000001<<(y%8)); 	// Draw
	} else {
		newByte &= ~(0b00000001<<(y%8));	// Erase
	}
	curScreen[byteIndex] = newByte;
}


/****************************************************
*        Font Functions		                    	*
****************************************************/
void ST7565R_configureFont(ST7565R_Font newFont)
{	// Send a complete Font struct to this function to configure the current font
	curFont.glyphs = newFont.glyphs;
	curFont.width = newFont.width;
	curFont.height = newFont.height;
	curFont.firstChar = newFont.firstChar;
	curFont.lastChar = newFont.lastChar;
}
void ST7565R_configureFontDefault(void){
#if defined(USING_FONT_CRC)
	ST7565R_Font defaultFont = {
		.glyphs = 		fontCRC,
		.width = 		CRCFONT_WIDTH,
		.height = 		CRCFONT_HEIGHT,
		.firstChar = 	CRCFONT_FIRSTCHAR,
		.lastChar = 	CRCFONT_LASTCHAR
	};
	ST7565R_configureFont(defaultFont);
#endif
}
void ST7565R_configureFontFlow(void){
#if defined(USING_FONT_FLOW)
	ST7565R_Font newFont = {
		.glyphs = 		fontFlow,
		.width = 		FLOWFONT_WIDTH,
		.height = 		FLOWFONT_HEIGHT,
		.firstChar = 	FLOWFONT_FIRSTCHAR,
		.lastChar = 	FLOWFONT_LASTCHAR
	};
	ST7565R_configureFont(newFont);
#endif
}
void ST7565R_configureFontKlein(void){
#if defined(USING_FONT_KLEIN)
	ST7565R_Font newFont = {
		.glyphs = 		fontKlein,
		.width =		KLEINFONT_WIDTH,
		.height = 		KLEINFONT_HEIGHT,
		.firstChar = 	KLEINFONT_FIRSTCHAR,
		.lastChar = 	KLEINFONT_LASTCHAR
	};
	ST7565R_configureFont(newFont);
#endif
}

/****************************************************
*           Initialization For controller           *
****************************************************/
void ST7565R_initScreen(void)
{	// Initialize the screen
	ST7565R_command(ST7565R_CMD_ADC_NORMAL);   				// ADC select
	ST7565R_command(ST7565R_CMD_DISPLAY_OFF);   			// Display OFF
	ST7565R_command(ST7565R_CMD_REVERSE_SCAN_DIRECTION);    // COM direction scan
	ST7565R_command(ST7565R_CMD_LCD_BIAS_1_DIV_6_DUTY33);   // LCD bias set
	ST7565R_command(ST7565R_CMD_POWER_CTRL_ALL_ON);   		// Power Control set
	ST7565R_command(ST7565R_CMD_VOLTAGE_RESISTOR_RATIO_1);  // Resistor Ratio Set
	ST7565R_command(ST7565R_CMD_ELECTRONIC_VOLUME_MODE_SET);// Electronic Volume Command (set contrast) Double Btye: 1 of 2
	ST7565R_command(ST7565R_CMD_VOLTAGE_RESISTOR_RATIO_0);  // Electronic Volume value (contrast value) Double Byte: 2 of 2
	ST7565R_command(ST7565R_CMD_DISPLAY_ON);   				// Display ON
}

/****************************************************
*           Setup/Shutdown Functions 		        *
****************************************************/
void ST7565R_setup(void)
{	// Initial Setup for ST7565R driver and screen
#ifndef PAINT_IMMEDIATELY
	lastScreen = (uint8_t*) malloc(SCREENBYTES);
	for(int i = 0; i < SCREENBYTES; i++){
		lastScreen[i] = 0xFF;
	}
#endif
	curScreen = (uint8_t*) malloc(SCREENBYTES);
	for(int i = 0; i < SCREENBYTES; i++){
		curScreen[i] = 0x00;
	}
	ST7565R_configureFontDefault();
	ST7565R_digital_write(NHD_RES, LOW);
	ST7565R_delay(100);
	ST7565R_digital_write(NHD_RES, HIGH);
	ST7565R_delay(100);
	ST7565R_initScreen();
#ifndef PAINT_IMMEDIATELY
	ST7565R_clearScreen();
	ST7565R_updateDisplay();
#endif
}

void ST7565R_shutdown(void)
{
	ST7565R_clearScreen();
	ST7565R_updateDisplay();
	ST7565R_command(ST7565R_CMD_DISPLAY_OFF);
	free(lastScreen);
	free(curScreen);
}


/****************************************************
*            Backlight Functions			        *
****************************************************/
void ST7565R_setBacklight(uint8_t brightness)
{	// Set the LED backlight of the screen to a specified brightness
	if(brightness < 0)
		brightness = 0;
	else if (brightness > 100)
		brightness = 100;

	ST7565R_set_pwm(brightness);
}
void ST7565R_blinkBacklight(float oscillationSpeed)
{	// Oscillate the brightness of the LED Backlight.
	static int16_t tempBright = 0;
	static int8_t dir = 1;
	tempBright += dir * oscillationSpeed;

	if(tempBright >= 100){
		tempBright = 100;
		dir = -1;
	} else if (tempBright <= 0){
		tempBright = 0;
		dir = 1;
	}

	ST7565R_set_pwm(tempBright);
}


/****************************************************
*            Test Functions			  		        *
****************************************************/
void ST7565R_screenTest(void)
{
	ST7565R_setBacklight(70);
	ST7565R_paintFullscreenBitmap(bmp_crcLeft());
	return;
	unsigned testX = 0;
	unsigned testY = 0;
	bool testBool = true;
	while (1) {
		ST7565R_delay(10);
		ST7565R_paintPixel(testBool, testX, testY);
		testX++;
		if (testX == SCREENWIDTH) {
			testX = 0;
			testY++;
			if (testY == SCREENHEIGHT) {
				testY = 0;
				testBool = !testBool;
			}
		}
	}
}














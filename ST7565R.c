




/*
 ***********************************************************************************************************************************************************************
	Author:		Mason Reck
 ***********************************************************************************************************************************************************************
	Description:
		Customized ST7565R Driver for our NHD-C12832A1Z-FSW-FBW-3V3 screen

	Driver Info:
		https://www.edeca.net/pages/the-st7565-display-controller/
	Screen Data Sheet:
		https://newhavendisplay.com/content/specs/NHD-C12832A1Z-FSW-FBW-3V3.pdf
 ***********************************************************************************************************************************************************************
 */

#include "ST7565R.h"
#include "bitmaps.h"

/****************************************************
*               PINS					            *
****************************************************/
#ifdef ST7565R_USING_STM
const ST7565R_STM_Pin NHD_LED = {.port = GPIOA, .pin = GPIO_PIN_15};		//PWM Voltage Source for Back Light
const ST7565R_STM_Pin NHD_RES = {.port = GPIOD, .pin = GPIO_PIN_2 };		//Reset signal
const ST7565R_STM_Pin NHD_A0  = {.port = GPIOB, .pin = GPIO_PIN_5 };		//Register select signal (Command = Low, Data=High)
const ST7565R_STM_Pin NHD_SDA = {.port = GPIOC, .pin = GPIO_PIN_12};		//(MOSI) Serial data signal
const ST7565R_STM_Pin NHD_CS  = {.port = GPIOB, .pin = GPIO_PIN_6 };		//Chip select signal
const ST7565R_STM_Pin NHD_SCL = {.port = GPIOC, .pin = GPIO_PIN_10};		//Serial clock signal
#endif
#ifdef ST7565R_USING_ATMEL
//TODO: configure driver for Atmel


#endif

/****************************************************
*        Current Screen					            *
****************************************************/

static uint8_t* curScreen;
static ST7565R_Font curFont;

/****************************************************
*        Display Functions		                    *
****************************************************/
void ST7565R_command(uint8_t cmd)
{
	digital_write(NHD_CS, LOW);		// Set Chip Select to Low to begin transmission over SPI
	digital_write(NHD_A0, LOW);		// Set AO Low to specify a Command Transmission
	HAL_SPI_Transmit(&hspi3, &cmd, 1, HAL_MAX_DELAY);
	digital_write(NHD_CS, HIGH);		// Set Chip Select to High to signal end of transmission
}

void ST7565R_drawByte(uint8_t byte)
{	// This function simply draws the byte as is in the preselected location
	digital_write(NHD_CS, LOW);		// Set Chip Select to Low to signal beginning of transmission over SPI
	digital_write(NHD_A0, HIGH);		// Set AO High to specify a data transmission
	HAL_SPI_Transmit(&hspi3, &byte, 1, HAL_MAX_DELAY);
	digital_write(NHD_CS, HIGH);		// Set Chip Select to High to signal end of transmission
}

void ST7565R_setByte(unsigned column, unsigned page, uint8_t byte)
{	// setByte function specifies location, unlike the drawByte function
	if(column >= SCREENWIDTH){column = SCREENWIDTH-1;}
	if(page >= NUM_PAGES){page = NUM_PAGES-1;}
	int byteIndex = (SCREENWIDTH*page)+column;
	curScreen[byteIndex] = byte;
	uint8_t colMSB = column/0x10;
	uint8_t colLSB = column%0x10;

	ST7565R_command(ST7565R_CMD_DISPLAY_OFF);		        // Set Display OFF
	ST7565R_command(ST7565R_CMD_PAGE_ADDRESS_SET(page));    // Send page address, There are 4 Pages for a 32 pixel screenheight
	ST7565R_command(ST7565R_CMD_COLUMN_MSB(colMSB));	    // Column address upper 4 bits + 0x10
	ST7565R_command(ST7565R_CMD_COLUMN_LSB(colLSB));	    // Column address lower 4 bits + 0x00
	ST7565R_drawByte(byte);
	ST7565R_command(ST7565R_CMD_DISPLAY_ON);
}

void ST7565R_paintPixel(unsigned x, unsigned y, bool newLevel)
{
	if(x >= SCREENWIDTH){x = SCREENWIDTH-1;}
	if(y >= SCREENHEIGHT){y = SCREENHEIGHT-1;}
	int byteIndex = (SCREENWIDTH*(y/8))+x;
	uint8_t newByte = curScreen[byteIndex];
	if(newLevel){
		newByte |= 0b00000001<<(y%8);
	} else {
		newByte &= 0b11111110<<(y%8);
	}
	curScreen[byteIndex] = newByte;
	uint8_t colMSB = x/0x10;
	uint8_t colLSB = x%0x10;

	ST7565R_command(ST7565R_CMD_DISPLAY_OFF);		    // Set Display OFF
	ST7565R_command(ST7565R_CMD_PAGE_ADDRESS_SET(y/8)); // Send page address, There are 4 Pages for a 32 pixel screenheight
	ST7565R_command(ST7565R_CMD_COLUMN_MSB(colMSB));	// Column address upper 4 bits + 0x10
	ST7565R_command(ST7565R_CMD_COLUMN_LSB(colLSB));	// Column address lower 4 bits + 0x00
	ST7565R_drawByte(newByte);
	ST7565R_command(ST7565R_CMD_DISPLAY_ON);
}

void ST7565R_paintString(char* string, unsigned x, unsigned y){
	unsigned originalX = x;

	for(int i = 0; i < strlen(string); i ++){

		// Special Characters
		if (string[i] == '\n'){y += curFont.height;}	// New Line
		if (string[i] == '\r'){x = originalX;}		// Carriage Return


		// Paint the currently selected character
		ST7565R_paintChar(string[i], x, y);
	}
}
void ST7565R_paintChar(char c, unsigned x, unsigned y){
	unsigned originalX = x;
	unsigned bytesPerRow = font_num_bytes_per_row(curFont.width);
	unsigned bytesPerChar = font_num_bytes_per_char(curFont.width, curFont.height);
	unsigned startIndex = (c - curFont.firstChar) * bytesPerChar;
	unsigned endIndex = startIndex + bytesPerChar;

	// Loop for all the bytes
	for(int i = startIndex; i < endIndex; i ++){

		uint8_t charByte = curFont.glyphs[i];


		//Loop for each individual byte
		for(int j = 0; j < 8; j ++){
			bool drawOrErase = (0x10 & (charByte<<j)) != 0;
//			ST7565R_paintPixel(x, y, drawOrErase);
		}
	}
}

void ST7565R_drawFullscreenBitmap(uint8_t* bitmap)
{
	if(bitmap == NULL){bitmap = crc;}					// Catch Null Pointers
	uint8_t page = ST7565R_CMD_PAGE_ADDRESS_SET(0);		// Initialize page
														// LOGIC FLOW:
	ST7565R_command(ST7565R_CMD_DISPLAY_OFF);			// Set Display OFF
	ST7565R_command(ST7565R_CMD_START_LINE_SET(0));		// Set the start line to line 0
	for(int i=0; i<NUM_PAGES; i++){						// 32 pixel display / 8 pixels per page = 4 pages
		ST7565R_command(page);							// Send page address
		ST7565R_command(ST7565R_CMD_MSB);				// Column address upper 4 bits + 0x10
		ST7565R_command(ST7565R_CMD_LSB);				// Column address lower 4 bits + 0x00
		for(int j=0; j<SCREENWIDTH; j++){				// 128 columns wide
			ST7565R_drawByte(*bitmap);					// Send bitmap byte
			bitmap++;									// Increment bitmap
		}
		page++;											// After drawing, go to next page
	}
	ST7565R_command(ST7565R_CMD_DISPLAY_ON);			// Set Display ON
}

void ST7565R_clearScreen(void)
{
	ST7565R_drawFullscreenBitmap(clear);
	for(int i = 0; i < 512; i ++){
		curScreen[i] = 0x00;
	}
}


/****************************************************
*        Font Functions		                    	*
****************************************************/

void ST7565R_configureFont(ST7565R_Font newFont){
	curFont.glyphs = newFont.glyphs;
	curFont.width = newFont.width;
	curFont.height = newFont.height;
	curFont.firstChar = newFont.firstChar;
	curFont.lastChar = newFont.lastChar;
}


/****************************************************
*           Initialization For controller           *
****************************************************/
void ST7565R_init_LCD(void)  {
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
*           Setup Functions 		                *
****************************************************/
void ST7565R_setup(void) {
	curScreen = clear;
	digital_write(NHD_RES, LOW);
	HAL_Delay(100);
	digital_write(NHD_RES, HIGH);
	HAL_Delay(100);
	ST7565R_init_LCD();
}


/****************************************************
*            Backlight Functions			        *
****************************************************/
void setBacklightNHD(uint8_t brightness){
	if(brightness < 0)
		brightness = 0;
	else if (brightness > 100)
		brightness = 100;

	TIM2->CCR1 = brightness;
}
void blinkBacklightNHD(uint8_t oscillationSpeed){
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

	TIM2->CCR1 = tempBright;
}



void screenTest(void){
	setBacklightNHD(70);
	ST7565R_drawFullscreenBitmap(crc);
	return;
	unsigned testX = 0;
	unsigned testY = 0;
	bool testBool = true;
	while (1) {
		HAL_Delay(10);
		ST7565R_paintPixel(testX, testY, testBool);
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














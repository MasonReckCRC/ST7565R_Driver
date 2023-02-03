# ST7565R_Driver
Driver for Display Controllers that use the ST7565R driver

Instructions:
  You will need to download ST7565R.c/.h at the very least
  Bring the files into your project, and use:
      #include "ST7565R.h" in your main.c or wherever you want to access the driver from
      
  Make sure you configure your Pins to whatever you have your MCU pinout set to. 
  You will also need to configure a PWM if you want to use the LED backlight for select screens
  I have provided my custom font, but you can use whatever font you want provided you supply the
  ST7565R_configureFont(ST7565R_Font font) function with a valid font structure.
  (You will need to create an instance of ST7565R_Font yourself and send it to the config function)
  


DRIVER IS STILL IN PROGRESS

Use Vertically Oriented Bitmaps to display custom images. 
Fonts are Horizontally Oriented. 

I am using this with the NHD-C12832A1Z-FSW-FBW-3V3 screen:
https://www.digikey.com/en/products/detail/newhaven-display-intl/NHD-C12832A1Z-FSW-FBW-3V3/2059236

Driver currently configured for STM and Atmel devices.
However; it shouldn't be too difficult to reconfigure for other platforms.

Driver is based off of a bare-bones Arduino driver from Newhaven Displays:
https://support.newhavendisplay.com/hc/en-us/articles/4415264814231-NHD-C12832A1Z-with-Arduino
See Link

The driver in its current state is not well optimized, and could be restructured to run a little smoother
if necessary. 







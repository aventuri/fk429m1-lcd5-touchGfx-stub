# Sample prj for STM32F429BI with CubeIde CubeMx TouchGfx 4.13

the board used is this one:
https://www.aliexpress.com/item/32918445880.html

with the 5 inch LCD (800x480) capacitive touch GT911

as per TouchGFX is just a mockup with two screens and two buttons

using latest CubeIde 1.3.1 (updated online from 1.3.0 stock) you can
* compile & debug on the device with ST-Link v2
* edit screens clicking on the *.touchgfx TouchGfx prj
* edit the IO clicking on the *.ioc CubeMx config file

## what works

the main components are working for a starting prj

* the LCD with a backlight on PWM and dutycyce 50%
* the touch GT911
* the LED1 is blinking on another FreeRTOS task to show core is running
* the USART1 is bound to printf() so you can use as service console

## what DO NOT works
some embedded components are still not configured, but they should not be too difficult
* SD card
* SPI Flash



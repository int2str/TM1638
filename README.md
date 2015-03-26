# AVR C++ library for the TM1638 LED driver

## About
The TM1638 LED driver can be found in 8 digit, 7-segment LED displays
which also incorporate 8 buttons as well as bi-colored red and green
LEDs. This "library" allows control of the module and integration
into AVR based projects.

## Features
* AVR C++
* Significantly faster than existing implementations
* No delays in any display code (only 1 for button reads)
* Simple to use
* Light weight

## Usage
Simply add tm1638.h/.cpp to your project. Make sure tor *change the port
assignments in tm1638.h*.

## Limitations
Currently the pins necessary to drive the chip (CLK - Clock; DIO - Data;
STB - Strobe) have to be on the same I/O registers (ex. PORTB). This is
largely because of how the direct pin mapping is implemented.

## Compiling / uploading

Simply run _make_ to compile the provided example.
_make upload_ will upload the hex file to your device, specified in
Makefile.inc.

## Revision history
* v1.02
  - Uploaded to source control
  - Added example
  - Minor cleanups & warning fixes

* v1.01
  - Added divmod10_asm()
  - Un-rolled send loop
  - Switch to toggeling output ports

* v1.00
  - Initial release


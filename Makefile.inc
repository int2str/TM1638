###############################################################################
# NoSpark - OpenEVSE charger firmware
# Copyright (C) 2015 Andre Eisenbach
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# See LICENSE for a copy of the GNU General Public License or see
# it online at <http://www.gnu.org/licenses/>.
###############################################################################

###############################################################################
#
# Board configuration
#
###############################################################################

# --- ATMega2560

#PORT = -P /dev/ttyUSB*
#UPLOAD_PROTOCOL = stk500v2
#BUILD_MCU = atmega2560
#BUILD_VARIANT = mega
#UPLOAD_SPEED = 115200
#BUILD_F_CPU = 16000000L

# --- Arduino UNO

PORT = -P COM11
# PORT = -P /dev/ttyACM*
UPLOAD_PROTOCOL = arduino
#UPLOAD_PROTOCOL = usbtiny
BUILD_MCU = atmega328p
BUILD_VARIANT = standard
UPLOAD_SPEED = 115200
BUILD_F_CPU = 16000000L


###############################################################################
#
# Tools & flags
#
################################################################################

CXX = avr-g++
AR = avr-ar
SIZE = avr-size
OBJCOPY = avr-objcopy
AVRDUDE = avrdude

DUDEFLAGS = -V -p $(BUILD_MCU) $(PORT) -c $(UPLOAD_PROTOCOL) -b $(UPLOAD_SPEED)

SIZEFLAGS = 

PRJCXXFLAGS = -Os -g -mmcu=$(BUILD_MCU) -DF_CPU=$(BUILD_F_CPU) \
	-ffunction-sections -fdata-sections -fmerge-all-constants \
	-fno-inline-small-functions -fshort-enums \
	-fno-exceptions -std=c++11 \
	-W -Wall -pedantic

LDFLAGS := $(LDFLAGS) -Wl,--gc-sections -Wl,--relax

FORMAT = ihex


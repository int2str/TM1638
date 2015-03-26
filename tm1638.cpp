//
// AVR TM1638 "Library" v1.02
// Enables control of TM1638 chip based modules, using direct port access.
//
// Copyright (c) 2013 IronCreek Software
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include <avr/io.h>
#include <util/delay.h>
#include "tm1638.h"
#include "tm1638Font.h"

#define TM_CLK_LOW()            (TM_OUT &= ~TM_BIT_CLK)
#define TM_CLK_HIGH()           (TM_OUT |= TM_BIT_CLK)
#define TM_STB_LOW()            (TM_OUT &= ~TM_BIT_STB)
#define TM_STB_HIGH()           (TM_OUT |= TM_BIT_STB)
#define TM_DAT_LOW()            (TM_OUT &= ~TM_BIT_DAT)
#define TM_DAT_HIGH()           (TM_OUT |= TM_BIT_DAT)

// Register mappings
#define REG_MAX                 0x0F
#define REG_LED_OFFSET          1
#define LED_TO_REG(p)           (REG_LED_OFFSET + (p << 1))

// Instructions
#define TM_DATA_CMD             0x40
#define TM_DISP_CTRL            0x80
#define TM_ADDR_CMD             0xC0

// Data command set
#define TM_WRITE_DISP           0x00
#define TM_READ_KEYS            0x02
#define TM_FIXED_ADDR           0x04

// Display control command
#define TM_DISP_PWM_MASK        0x07 // First 3 bits are brightness (PWM controlled)
#define TM_DISP_ENABLE          0x08

namespace
{
    void port_setup()
    {
        TM_DDR |= TM_BIT_STB | TM_BIT_CLK | TM_BIT_DAT;
        TM_OUT |= TM_BIT_STB | TM_BIT_CLK;
    }

    void send(uint8_t b)
    {
        for (uint8_t i = 8; i; --i, b >>= 1)
        {
            TM_CLK_LOW();
            if (b & 1)
                TM_DAT_HIGH();
            else
                TM_DAT_LOW();
            TM_CLK_HIGH();
        }
    }

    void send_cmd(const uint8_t cmd)
    {
        TM_STB_LOW();
        send(cmd);
        TM_STB_HIGH();
    }

    void send_data(const uint8_t addr, const uint8_t data)
    {
        send_cmd(TM_DATA_CMD | TM_FIXED_ADDR);
        TM_STB_LOW();
        send(TM_ADDR_CMD | addr);
        send(data);
        TM_STB_HIGH();
    }

    uint8_t receive()
    {
        uint8_t rc = 0;

        // Change DAT pin to INPUT and enable pull-up
        TM_DDR &= ~TM_BIT_DAT;
        TM_DAT_HIGH();

        for (uint8_t i = 8, b = 1; i; --i, b <<= 1)
        {
            TM_CLK_LOW();

            // Must wait tWAIT for CLK transition
            _delay_us(1);

            if (TM_IN & TM_BIT_DAT)
                rc |= b;
            TM_CLK_HIGH();
        }

        // Disable pull-up and reset pin
        TM_DDR |= TM_BIT_DAT;
        TM_DAT_LOW();

        return rc;
    }

    uint8_t read_buttons()
    {
        uint8_t rc = 0;

        TM_STB_LOW();
        send(TM_DATA_CMD | TM_READ_KEYS);
        for(uint8_t bytes = 0; bytes != 4; ++bytes)
            rc |= receive() << bytes;
        TM_STB_HIGH();

        return rc;
    }

    uint8_t offset_digits(uint32_t num)
    {
        uint8_t digits = 0;
        while (num >= 10)
        {
            num /= 10;
            ++digits;
        }
        return digits;
    }
}

TM1638::TM1638()
    : _dotMask(0)
{
    port_setup();

    send_cmd(TM_DATA_CMD | TM_WRITE_DISP);
    send_cmd(TM_DISP_CTRL | TM_DISP_ENABLE | TM_DISP_PWM_MASK);

    clear();
    clearLEDs();
}

void TM1638::clear()
{
    for (uint8_t a = 0; a <= REG_MAX; a += 2)
        send_data(a, 0x00);
}

void TM1638::setByte(const uint8_t pos, const uint8_t value)
{
    send_data(pos << 1, value | (_dotMask & (1 << pos) ? TM_DOT : 0));
}

void TM1638::setDigit(const uint8_t pos, const uint8_t value)
{
    setByte(pos, pgm_read_byte(&TM_DIGITS[value & 0xF]));
}

void TM1638::setNumber(uint32_t number, uint8_t offset, const uint8_t align)
{
    if (align == TM_LEFT)
        offset += offset_digits(number);

    while (number && offset != 0xFF)
    {
        setDigit(offset--, number % 10);
        number /= 10;
    }
}

void TM1638::setNumberPad(uint32_t number, uint8_t offset, uint8_t width, const uint8_t pad)
{
    while (number && width-- && offset != 0xFF)
    {
        setDigit(offset--, number % 10);
        number /= 10;
    }

    while (width -- && offset != 0xFF)
        setByte(offset--, pad);
}

void TM1638::setNumberHex(uint32_t number, uint8_t offset, uint8_t width, const uint8_t pad)
{
    while (number && width-- && offset != 0xFF)
    {
        setDigit(offset--, number & 0x0F);
        number >>= 4;
    }

    while (width -- && offset != 0xFF)
        setByte(offset--, pad);
}

void TM1638::setChar(const uint8_t pos, const char value)
{
    const uint8_t b = TM1638_map_char(value);
    if (b)
        setByte(pos, b);

    else if (value >= 'a' && value <= 'z')
        setByte(pos, pgm_read_byte(&TM_DIGITS[value - 'a' + 10]));

    else if (value >= 'A' && value <= 'Z')
        setByte(pos, pgm_read_byte(&TM_DIGITS[value - 'A' + 10]));

    else if (value >= '0' && value <= '9')
        setByte(pos, pgm_read_byte(&TM_DIGITS[value - '0']));
}

void TM1638::setChars(const char* value, uint8_t offset)
{
    while (*value)
        setChar(offset++, *value++);
}

void TM1638::setDots(const uint8_t mask)
{
    _dotMask = mask;
}

void TM1638::clearLEDs()
{
    for (uint8_t a = REG_LED_OFFSET; a <= REG_MAX; a += 2)
        send_data(a, 0x00);
}

void TM1638::setLED(const uint8_t pos, const uint8_t color)
{
    send_data(LED_TO_REG(pos), color);
}

void TM1638::setLEDs(uint8_t green, uint8_t red)
{
    for (uint8_t a = REG_LED_OFFSET; a <= REG_MAX; a += 2, green >>= 1, red >>= 1)
        send_data(a, (green & 1) | ((red & 1) << 1));
}

uint8_t TM1638::getButtons()
{
    return read_buttons();
}

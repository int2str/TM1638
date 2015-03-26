#include <util/delay.h>
#include "tm1638.h"

int main()
{
    TM1638 display;

    // Say hi ...

    display.setChars(" TM1638 ");
    display.setLEDs(0xAA, 0x55);

    _delay_ms(5000);

    display.clear();
    display.clearLEDs();


    // Count up and handle buttons as well...

    uint32_t i = 0;
    while (1)
    {
        if (i == 0)
            display.clear();

        display.setNumber(i++);
        display.setLEDs(i >> 8, display.getButtons());
    }
}

#define F_CPU 16000000UL

#include "funsape/globalDefines.hpp"
#include "funsape/peripheral/timer1.hpp"

#include <ssd1306.h>
#include <ssd1306_fonts.h>
#include <stdio.h>

#include "drivers/dht11.hpp"


volatile bool requestSample = false;


void timer1CompareACallback()
{
    requestSample = true;
}

int main()
{
    DHT11 dht;
    dht.init(PORTC, DDRC, PINC, 3);

    ssd1306_128x64_i2c_initEx(-1, -1, 0x3C);
    ssd1306_clearScreen();
    ssd1306_setFixedFont(ssd1306xled_font6x8);
    ssd1306_flipVertical(1);
    ssd1306_flipHorizontal(1);

    DHT11::Data data;
    char buffer[22];

    timer1.init(Timer1::Mode::CTC_OCRA, Timer1::ClockSource::PRESCALER_1024);
    timer1.setCompareAValue(31249);
    timer1.activateCompareAInterrupt();

    sei();


    while (1)
    {
        bool sample;

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            sample = requestSample;
            requestSample = false;
        }

        if (sample)
        {
            if (dht.read(data) == DHT11::Result::OK)
            {
                snprintf(buffer, sizeof(buffer), "Temp: %2u.%u C ", (unsigned)data.temperature, (unsigned)data.temperatureDecimal);
                ssd1306_printFixed(0, 0, buffer, STYLE_NORMAL);

                snprintf(buffer, sizeof(buffer), "Hum: %3u.%u%% ", (unsigned)data.humidity, (unsigned)data.humidityDecimal);
                ssd1306_printFixed(0, 10, buffer, STYLE_NORMAL);
            }
            else
            {
                ssd1306_printFixed(0, 20, "Status: ERROR", STYLE_NORMAL);
            }
        }
    }
}
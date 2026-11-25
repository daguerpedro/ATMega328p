#define F_CPU 16000000UL

#include "funsape/globalDefines.hpp"
#include "funsape/peripheral/timer1.hpp"
#include "funsape/peripheral/gpioPin.hpp"
#include "funsape/peripheral/pcint0.hpp"

#include <ssd1306.h>
#include <ssd1306_fonts.h>
#include <stdio.h>

#include "drivers/dht11.hpp"
#include "drivers/encoder.hpp"

volatile bool fRequestSample = false;

Encoder encoder;

void timer1CompareACallback()
{
    fRequestSample = true;
}

void pcint0InterruptCallback()
{
    encoder.update();
}

int main()
{
    encoder.init(&PORTB, PinIndex::P0, PinIndex::P1, PinIndex::P2, true);

    DHT11 dht;
    dht.init(PORTC, DDRC, PINC, 3);

    ssd1306_128x64_i2c_initEx(-1, -1, 0x3C);
    ssd1306_clearScreen();
    ssd1306_setFixedFont(ssd1306xled_font6x8);
    ssd1306_flipVertical(1);
    ssd1306_flipHorizontal(1);

    DHT11::Data data;
    char buffer[128];

    timer1.init(Timer1::Mode::CTC_OCRA, Timer1::ClockSource::PRESCALER_1024);
    timer1.setCompareAValue(31249); // 2 Sec
    timer1.activateCompareAInterrupt();

    pcint0.init(Pcint0::Pin::PIN_PCINT0 | Pcint0::Pin::PIN_PCINT1 | Pcint0::Pin::PIN_PCINT2);
    pcint0.activateInterrupt();

    sei();

    int8_t index = 0;
    int8_t curIndex = 0;

    while (1)
    {
        bool sample;

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            sample = fRequestSample;
            fRequestSample = false;
        }

        if (sample)
        {
            DHT11::Result res = dht.read(data);

            if (res != DHT11::Result::OK)
                ssd1306_printFixed(0, 20, "Status: ERROR", STYLE_NORMAL);
        }

        Encoder::InputAction act = encoder.getInputAction();

        switch (act.rotAction)
        {
        case Encoder::RotEvent::CLOCKWISE:
            index++;
            break;
            ;
        case Encoder::RotEvent::COUNTERCLOCK:
            index--;
            break;
        default:
            break;
        }

        if (index < 0)
            index = 2;

        if (index > 2)
            index = 0;

        if (index != curIndex)
        {
            ssd1306_clearBlock(0, 0, 128, 8);
            curIndex = index;
        }

        switch (index)
        {
        case 0:
            snprintf(buffer, sizeof(buffer), "Temp: %u.%u *C", data.temperature, data.temperatureDecimal);
            break;
        case 1:
            snprintf(buffer, sizeof(buffer), "Hum: %u.%u %%", data.humidity, data.humidityDecimal);
            break;
        case 2:
            snprintf(buffer, sizeof(buffer), "Teste");
            break;

        default:
            break;
        }
        ssd1306_printFixed(0, 0, buffer, STYLE_NORMAL);
    }
}
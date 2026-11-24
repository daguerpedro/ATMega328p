#define F_CPU 16000000UL

#include "funsape/globalDefines.hpp"
#include "funsape/peripheral/timer1.hpp"
#include "funsape/peripheral/timer0.hpp"
#include "funsape/peripheral/gpioPin.hpp"

#include <ssd1306.h>
#include <ssd1306_fonts.h>
#include <stdio.h>

#include "drivers/dht11.hpp"
#include "drivers/encoder.hpp"

volatile bool fRequestSample = false;
volatile bool fUpdateEncoder = false;

GpioPin s1;
GpioPin s2;
GpioPin key;
Encoder encoder(s1, s2, key, 100);

void timer0CompareACallback()
{
    fUpdateEncoder = true;
}

void timer1CompareACallback()
{
    fRequestSample = true;
}

int main()
{

    s1.init(&PORTB, PinIndex::P0);
    s2.init(&PORTB, PinIndex::P1);
    key.init(&PORTB, PinIndex::P2);

    s1.setMode(PinMode::INPUT_PULLED_UP);
    s2.setMode(PinMode::INPUT_PULLED_UP);
    key.setMode(PinMode::INPUT_PULLED_UP);

    encoder.init();

    DHT11 dht;
    dht.init(PORTC, DDRC, PINC, 3);

    ssd1306_128x64_i2c_initEx(-1, -1, 0x3C);
    ssd1306_clearScreen();
    ssd1306_setFixedFont(ssd1306xled_font6x8);
    ssd1306_flipVertical(1);
    ssd1306_flipHorizontal(1);

    DHT11::Data data;
    char buffer[128];

    timer0.init(Timer0::Mode::CTC_OCRA, Timer0::ClockSource::PRESCALER_64);
    timer0.setCompareAValue(249); // 1 ms
    timer0.activateCompareAInterrupt();

    timer1.init(Timer1::Mode::CTC_OCRA, Timer1::ClockSource::PRESCALER_1024);
    timer1.setCompareAValue(31249); // 2 Sec
    timer1.activateCompareAInterrupt();

    sei();

    int8_t index = 0;
    int8_t curIndex = 0;

    while (1)
    {
        bool sample;
        bool updateEnc;

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            sample = fRequestSample;
            updateEnc = fUpdateEncoder;

            fUpdateEncoder = false;
            fRequestSample = false;
        }

        if (updateEnc)
        {
            encoder.update();
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

        if(index != curIndex)
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
#include "system.h"

#include "funsape/globalDefines.hpp"
#include "funsape/peripheral/timer1.hpp"
#include "funsape/peripheral/gpioPin.hpp"
#include "funsape/peripheral/pcint0.hpp"

#include <ssd1306.h>
#include <ssd1306_fonts.h>
#include <stdio.h>

#include "drivers/dht11.hpp"
#include "drivers/encoder.hpp"
#include "drivers/ds1302.hpp"

void System::startDevices()
{
    // TODO: funcao privada p iniciar display
    ssd1306_128x64_i2c_initEx(-1, -1, 0x3C);
    ssd1306_flipVertical(1);
    ssd1306_flipHorizontal(1);
    ssd1306_setFixedFont(ssd1306xled_font6x8);
    ssd1306_clearScreen();

    ssd1306_printFixed((12 * 6) / 2, 8, "ThermoLOG", EFontStyle::STYLE_NORMAL);
    ssd1306_setCursor(0, 26);

    encoder.init(&PORTB, PinIndex::P0, PinIndex::P1, PinIndex::P2, true);

    // TODO: deixar essa funcao apenas iniciar os dispositivos
    // TODO: mover os prints de erro para outra funcao apos init.
    DHT11::Result res = dht11.init(PORTC, DDRC, PINC, 3);

    ssd1306_print("[ DHT: ");
    ssd1306_print(dht11.ResultString(res));
    ssd1306_print(" ]\n");

    ssd1306_print("[ RTC: ");
    if (rtc.init(PORTD, DDRD, PIND, PD5, PD6, PD7) == false)
    {
        ssd1306_print(rtc.ErrorString(rtc.getLastError()));
        ssd1306_print(" ]\n");
        delayMs(10000);
    }
    ssd1306_print("OK ]\n");

    rtc.clockStart();

    activeScreen = SCREEN::SENSORS;
    screen = SCREEN::CONFIG;

    ssd1306_clearScreen();
}

void System::startPeripherals()
{
    timer1.init(Timer1::Mode::CTC_OCRA, Timer1::ClockSource::PRESCALER_1024);
    timer1.setCompareAValue(31249); // 2 Sec
    timer1.activateCompareAInterrupt();

    pcint0.init(Pcint0::Pin::PIN_PCINT0 | Pcint0::Pin::PIN_PCINT1 | Pcint0::Pin::PIN_PCINT2);
    pcint0.activateInterrupt();
}

void System::sampleRoutine()
{
    bool sampleRequested;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        sampleRequested = m_fRequestSample;
        m_fRequestSample = false;
    }

    if (sampleRequested)
    {
        DHT11::Result res = dht11.read(dhtBuffer);

        if (res != DHT11::Result::OK)
            ssd1306_printFixed(0, 20, "Status: ERROR", STYLE_NORMAL);
    }
}

void System::drawScreen()
{
    sampleRoutine();
/*
    Encoder::InputAction act = encoder.getInputAction();

    switch (act.rotAction)
    {
    case Encoder::RotEvent::CLOCKWISE:
        screen = static_cast<SCREEN>((static_cast<uint8_t>(activeScreen) + 1));
        break;
    case Encoder::RotEvent::COUNTERCLOCK:
        screen =  static_cast<SCREEN>((static_cast<uint8_t>(activeScreen) - 1));
        break;
    default:
        break;
    }

 
    if ((uint8_t)screen < 0)
        screen = (SCREEN)2;

    if ((uint8_t)screen > 2)
        screen = (SCREEN)0;

    if (screen != activeScreen)
    {
        activeScreen = screen;
        ssd1306_clearScreen();
        ssd1306_clearBlock(0,0, UINT8_MAX, UINT8_MAX);

        snprintf(displayBuffer, sizeof(displayBuffer), "SC: %u", (uint8_t)activeScreen);
    }

    switch (activeScreen)
    {
    default:
        break;
    }
*/
    ssd1306_printFixed(0, 0, displayBuffer, STYLE_NORMAL);
}

void timer1CompareACallback()
{
    System::getInstance().requestSample();
}

void pcint0InterruptCallback()
{
    System::getInstance().requestEncoder();
}

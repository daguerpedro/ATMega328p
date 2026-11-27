#include "system.h"

#include "funsape/globalDefines.hpp"
#include "funsape/peripheral/timer1.hpp"
#include "funsape/peripheral/timer0.hpp"
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
        delayMs(5000);
    }
    ssd1306_print("OK ]\n");

    rtc.clockStart();
    rtc.getTime(
        &rtcBuffer.hora,
        &rtcBuffer.min,
        &rtcBuffer.sec
    );

    activeScreen = SCREEN::SENSORS;
    targetScreen = SCREEN::SENSORS;

    ssd1306_clearScreen();
}

void System::startPeripherals()
{
    timer1.init(Timer1::Mode::CTC_OCRA, Timer1::ClockSource::PRESCALER_1024);
    timer1.setCompareAValue(31249); // 2 Sec
    timer1.setCompareBValue(15624); // 1 Sec
    timer1.activateCompareAInterrupt();
    timer1.activateCompareBInterrupt();

    pcint0.init(Pcint0::Pin::PIN_PCINT0 | Pcint0::Pin::PIN_PCINT1 | Pcint0::Pin::PIN_PCINT2);
    pcint0.activateInterrupt();
}

void System::checkFlags()
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        sampleRequested = m_fRequestSample;
        rtcRequested = m_fRequestRTC;

        m_fRequestSample = false;
        m_fRequestRTC = false;
    }
};

void System::sampleRoutine()
{
    if (sampleRequested)
    {
        DHT11::Result res = dht11.read(dhtBuffer);
        if (res != DHT11::Result::OK)
            ssd1306_printFixed(0, 20, "Status: ERROR", STYLE_NORMAL);
        
        if(activeScreen == SCREEN::SENSORS)
            m_updateScreen = true;
    }
}

void System::rtcRoutine()
{
    if(rtcRequested)
    {        
        rtc.getTime(
            &rtcBuffer.hora,
            &rtcBuffer.min,
            &rtcBuffer.sec
        );

        if(activeScreen == SCREEN::TIME)
            m_updateScreen = true;
    }
}

void System::inputRoutine()
{
    Encoder::InputAction act = encoder.getInputAction();

    switch (act.rotAction)
    {
    case Encoder::RotEvent::CLOCKWISE:
        targetScreen = static_cast<SCREEN>((static_cast<uint8_t>(activeScreen) + 1));
        break;
    case Encoder::RotEvent::COUNTERCLOCK:
        targetScreen = static_cast<SCREEN>((static_cast<uint8_t>(activeScreen) - 1));
        break;
    default:
        break;
    }

    if (targetScreen > SCREEN::CONFIG)
        targetScreen = SCREEN::SENSORS;

    if (targetScreen != activeScreen)
    {
        activeScreen = targetScreen;
        ssd1306_clearScreen();
        m_updateScreen = true;
    }
}

void System::drawScreen()
{
    while (1)
    {
        checkFlags();
        sampleRoutine();
        inputRoutine();
        rtcRoutine();
        
        if (m_updateScreen)
        {
            m_updateScreen = false;

            // ssd1306_clearScreen();

            switch (activeScreen)
            {
            case SCREEN::SENSORS:
                ssd1306_setCursor(0, 8);
                snprintf(displayBuffer, sizeof(displayBuffer), "Temp: %u.%u *C", dhtBuffer.temperature, dhtBuffer.temperatureDecimal);
                ssd1306_print(displayBuffer);
                break;
            case SCREEN::TIME:
                ssd1306_setCursor(0, 8);
                snprintf(displayBuffer, sizeof(displayBuffer), "Horario: %0u:%0u:%0u", rtcBuffer.hora, rtcBuffer.min, rtcBuffer.sec);
                ssd1306_print(displayBuffer);
                break;
            case SCREEN::CONFIG:
                break;

            default:
                break;
            }
        }
    }
}

void timer1CompareBCallback()
{
    System::getInstance().requestRTC();
}

void timer1CompareACallback()
{
    System::getInstance().requestSample();
}

void pcint0InterruptCallback()
{
    System::getInstance().requestEncoder();
}

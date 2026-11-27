#include <avr/pgmspace.h>

#include "system.h"

#include "config.hpp"
#include "funsape/peripheral/timer0.hpp"
#include "funsape/peripheral/timer1.hpp"

#include <ssd1306.h>
#include <ssd1306_fonts.h>
#include <stdio.h>
#include <util/atomic.h>

void System::startDevices()
{
    ssd1306_128x64_i2c_initEx
    (
        -1,
        -1,
        Config::OLED_ADDRESS
    );
    ssd1306_flipVertical(1);
    ssd1306_flipHorizontal(1);
    ssd1306_setFixedFont(ssd1306xled_font6x8);
    ssd1306_clearScreen();
    ssd1306_printFixed(
        30,
        8,
        "ThermoLOG",
        EFontStyle::STYLE_NORMAL
    );
    ssd1306_setCursor(0, 26);

    encoderReady = encoder.init
    (
        Config::ENCODER_PORT,
        Config::ENCODER_S1_PIN,
        Config::ENCODER_S2_PIN,
        Config::ENCODER_KEY_PIN,
        Config::ENCODER_PULL_UP
    );

    dhtInitResult = dht11.init
    (
        Config::DHT11_PORT,
        Config::DHT11_PIN
    );

    rtcRunning = rtc.init
    (
        Config::RTC_PORT,
        Config::RTC_CE_PIN,
        Config::RTC_CLK_PIN,
        Config::RTC_IO_PIN
    ) && rtc.clockStart();

    rtcTimeValid = rtcRunning && rtc.getTime
    (
        &rtcBuffer.hora,
        &rtcBuffer.min,
        &rtcBuffer.sec
    );

    delayMs(1000);
    dhtSampleResult = dht11.read(dhtBuffer);

    activeScreen = SCREEN::SENSORS;
    m_updateScreen = true;
    fahrenheit = false;

    ssd1306_clearScreen();
}

void System::startPeripherals()
{
    timer1.init(Timer1::Mode::CTC_OCRA, Timer1::ClockSource::PRESCALER_1024);

    timer1.setCompareAValue(Config::SAMPLE_COMPARE);
    timer1.activateCompareAInterrupt();

    timer1.setCompareBValue(Config::RTC_COMPARE);
    timer1.activateCompareBInterrupt();

    encoderReady = encoderReady && pcint0.init(Config::ENCODER_INTERRUPT_PINS);

    encoderReady = encoderReady && timer0.init(Timer0::Mode::CTC_OCRA, Timer0::ClockSource::PRESCALER_64);

    if (encoderReady)
    {
        timer0.setCompareAValue(Config::ENCODER_KEY_COMPARE);
        timer0.clearCompareAInterruptRequest();
        timer0.activateCompareAInterrupt();
        pcint0.activateInterrupt();
    }

}

void System::checkFlags()
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        sampleRequested = m_fRequestSample;
        m_fRequestSample = false;

        rtcRequested = m_fRequestRTC;
        m_fRequestRTC = false;
    }
};

void System::sampleRoutine()
{
    if (sampleRequested)
    {
        if (dhtInitResult == DHT11::Result::OK)
            dhtSampleResult = dht11.read(dhtBuffer);

        if (activeScreen == SCREEN::SENSORS)
            m_updateScreen = true;
    }
}

void System::rtcRoutine()
{
    if (rtcRequested && rtcRunning)
    {
        rtcTimeValid = rtc.getTime
        (
            &rtcBuffer.hora,
            &rtcBuffer.min,
            &rtcBuffer.sec
        );

        if (activeScreen == SCREEN::SENSORS)
            m_updateScreen = true;
    }
}

void System::inputRoutine()
{
    if (!encoderReady)
        return;

    const Encoder::InputAction action = encoder.getInputAction();

    if(activeScreen == SCREEN::SENSORS)
    {
        if(action.clicked)
        {
            activeScreen = SCREEN::CONFIG;
            ssd1306_clearScreen();
            m_updateScreen = true;
        }

        return;
    }

    if(activeScreen == SCREEN::CONFIG)
    {
        if(action.rotAction != Encoder::RotEvent::NONE)
        {
            configUnitSelected = !configUnitSelected;
            m_updateScreen = true;
            return;
        }

        if(action.clicked)
        {
            if(configUnitSelected)
            {
                fahrenheit = !fahrenheit;
            }
            else
            {
                activeScreen = SCREEN::SENSORS;
                ssd1306_clearScreen();
            }

            m_updateScreen = true;
        }
        return;
    }

}

void System::run()
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

            drawScreen();
        }
    }
}
void System::drawScreen()
{
    if (activeScreen == SCREEN::SENSORS)
        drawSensors();
    else
        drawConfig();
}

void System::drawSensors()
{
    ssd1306_printFixed(
        30,
        8,
        "ThermoLOG",
        EFontStyle::STYLE_NORMAL
    );

    if (dhtSampleResult == DHT11::Result::OK)
    {
        if(fahrenheit)
        {
            const uint16_t celsiusTenths = dhtBuffer.temperature * 10U + dhtBuffer.temperatureDecimal;
            const uint16_t temperature =  (celsiusTenths * 9U + 2U) / 5U + 320U;

            snprintf_P(
                displayBuffer, 
                sizeof(displayBuffer), 
                PSTR("%u.%u*F"),
                temperature / 10U, 
                temperature % 10U
            );
        }else
        {
            snprintf_P(
                displayBuffer, 
                sizeof(displayBuffer), 
                PSTR("%u.%u*C"),
                dhtBuffer.temperature, 
                dhtBuffer.temperatureDecimal
            );
        }
        ssd1306_printFixed(27, 24, displayBuffer, STYLE_NORMAL);

        snprintf_P(
            displayBuffer, 
            sizeof(displayBuffer), 
            PSTR("%u%%"),
            dhtBuffer.humidity
        );
        ssd1306_printFixed(70, 24, displayBuffer, STYLE_NORMAL);
    }

    if (rtcTimeValid)
    {
        snprintf_P(
            displayBuffer,
            sizeof(displayBuffer),
            PSTR("%02u:%02u:%02u"),
            rtcBuffer.hora, 
            rtcBuffer.min,
            rtcBuffer.sec
        );
        ssd1306_printFixed(30, 34, displayBuffer, STYLE_NORMAL);
    }
    else
    {
        ssd1306_printFixed(0, 34, (rtcRunning ? "RTC sem leitura" : "Falha ao ligar RTC "), STYLE_NORMAL);
    }

    ssd1306_printFixed(0, 48, encoderReady ? "Clique: configurar" : "Falha no encoder", STYLE_NORMAL);

    if(dhtInitResult != DHT11::Result::OK || dhtSampleResult != DHT11::Result::OK)
    {
        ssd1306_setCursor(0, 48);
        ssd1306_print("DHT: ");
        ssd1306_print(DHT11::ResultString(dhtInitResult != DHT11::Result::OK ? dhtInitResult : dhtSampleResult));   
    }
}

void System::drawConfig()
{
    ssd1306_printFixed(0, 0, "Configuracao", STYLE_NORMAL);

    snprintf_P(
        displayBuffer, 
        sizeof(displayBuffer), 
        PSTR("%c Unidade: *%c"),
        configUnitSelected ? '>' : ' ',
        fahrenheit ? 'F' : 'C'
    );
    ssd1306_printFixed(0, 10, displayBuffer, STYLE_NORMAL);
    ssd1306_printFixed(0, 20, configUnitSelected ? "  Voltar" : "> Voltar", STYLE_NORMAL);
    ssd1306_printFixed(0, 30, "Gire para selecionar", STYLE_NORMAL);
    ssd1306_printFixed(0, 38, "Clique para confirmar", STYLE_NORMAL);
}

void timer1CompareBCallback()
{
    System::getInstance().requestRTC();
}

void timer1CompareACallback()
{
    System::getInstance().requestSample();
    System::getInstance().requestRTC();
}

void timer0CompareACallback()
{
    System::getInstance().sampleEncoderKey();
}

void pcint0InterruptCallback()
{
    System::getInstance().requestEncoder();
}

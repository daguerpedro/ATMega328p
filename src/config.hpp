#pragma once

#include <stdint.h>
#include <avr/io.h>

#include "funsape/peripheral/pcint0.hpp"

namespace Config
{
    constexpr uint8_t               OLED_ADDRESS = 0x3C;

    inline const ioRegAddress_t     DHT11_PORT = &PORTC;
    constexpr PinIndex              DHT11_PIN = PinIndex::P3;       // PINO A3

    inline const ioRegAddress_t     RTC_PORT = &PORTD;
    constexpr PinIndex              RTC_CE_PIN = PinIndex::P5;      // PINO D5
    constexpr PinIndex              RTC_CLK_PIN = PinIndex::P6;     // PINO D6
    constexpr PinIndex              RTC_IO_PIN = PinIndex::P7;      // PINO D7

    constexpr uint16_t              SAMPLE_COMPARE = 31249;         // 2 Segundos
    constexpr uint16_t              RTC_COMPARE = 15624;            // 1 Segundo

    inline const ioRegAddress_t     ENCODER_PORT = &PORTB;
    constexpr PinIndex              ENCODER_S1_PIN = PinIndex::P0;  // PINO D8
    constexpr PinIndex              ENCODER_S2_PIN = PinIndex::P1;  // PINO D9
    constexpr PinIndex              ENCODER_KEY_PIN = PinIndex::P2; // PINO D10
    
    constexpr bool                  ENCODER_PULL_UP = true;
    constexpr uint8_t               ENCODER_KEY_COMPARE = F_CPU / 64UL / 1000UL - 1;

    constexpr Pcint0::Pin           ENCODER_INTERRUPT_PINS = static_cast<Pcint0::Pin>(
        (1 << static_cast<uint8_t>(ENCODER_S1_PIN)) |
        (1 << static_cast<uint8_t>(ENCODER_S2_PIN)) |
        (1 << static_cast<uint8_t>(ENCODER_KEY_PIN))
    );
}

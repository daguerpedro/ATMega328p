
#pragma once

#include "../funsape/peripheral/gpioPin.hpp"

#include <stdint.h>
#include <util/delay.h>
#include <util/atomic.h>

class DHT11
{
public:
    DHT11() = default;
    ~DHT11() = default;

    DHT11(const DHT11 &) = delete;
    DHT11 &operator=(const DHT11 &) = delete;

    enum class Result : uint8_t
    {
        OK,
        Invalid,
        NotInitialized,
        NoResponse,
        TimeOut,
        ChecksumError
    };

    static const char *ResultString(Result res)
    {
        switch (res)
        {
        case Result::OK:
            return "Ok";
        case Result::Invalid:
            return "Invalido";
        case Result::NotInitialized:
            return "Não Iniciado";
        case Result::NoResponse:
            return "Sem Resposta";
        case Result::TimeOut:
            return "TimeOut";
        case Result::ChecksumError:
            return "Checksum";
        }

        return "Desconhecido";
    }

    struct Data
    {
        uint8_t humidity;
        uint8_t humidityDecimal;
        uint8_t temperature;
        uint8_t temperatureDecimal;
    };

    Result init(ioRegAddress_t port, PinIndex pinIndex)
    {
        if (static_cast<uint8_t>(pinIndex) >= 8 || m_gpio.isInitialized() || !m_gpio.init(port, pinIndex))
            return Result::Invalid;

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            m_gpio.setMode(PinMode::INPUT_PULLED_UP); // Input Internal pull-up
        }

        return Result::OK;
    }

    Result read(Data &out)
    {
        if (!m_gpio.isInitialized())
            return Result::NotInitialized;

        // 5.2 - MCU Sends out Start Signal to DHT
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            m_gpio.low();
            m_gpio.setMode(PinMode::OUTPUT_PUSH_PULL);
        }
        _delay_ms(20);

        uint8_t data[5] = {};
        Result result = Result::OK;

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            // Release DATA line.
            m_gpio.setMode(PinMode::INPUT_PULLED_UP); // Internal pull-up.

            // 5.3 - DHT Responses to MCU
            // |------ Handshake ------|
            if (!waitLevel(true) || !waitLevel(false)) // HIGH: 20-40us
                result = Result::NoResponse;
            else if (!waitLevel(true) || !waitLevel(false)) // LOW: 80us | HIGH: 80us
                result = Result::TimeOut;
            // |--- End Of Handshake ---|

            else
            {
                for (uint8_t i = 0; i < 40; ++i) // Receive 40 bits, MSB first.
                {
                    // LOW: 50us
                    if (!waitLevel(true))
                    {
                        result = Result::TimeOut;
                        break;
                    }

                    // Detect bit after edge: HIGH: 26-28us = bit 0; 70us = bit 1
                    _delay_us(40);

                    const bool bit = m_gpio.read();
                    const uint8_t byteIndex = i / 8;

                    data[byteIndex] = (data[byteIndex] << 1) | bit;

                    if (!waitLevel(false))
                    {
                        result = Result::TimeOut;
                        break;
                    }
                }

                // End of transmission.
                if (result == Result::OK && !waitLevel(true))
                    result = Result::TimeOut;
            }
        }

        if (result != Result::OK)
            return result;

        // Checksum validation.
        if (static_cast<uint8_t>(data[0] + data[1] + data[2] + data[3]) != data[4])
        {
            return Result::ChecksumError;
        }

        out = Data{data[0], data[1], data[2], data[3]};

        return Result::OK;
    }

private:
    GpioPin m_gpio;

    inline bool waitLevel(bool high)
    {
        uint8_t timeout = 120;

        while (m_gpio.read() != high)
        {
            if (--timeout == 0)
                return false;

            _delay_us(1);
        }

        return true;
    }
};


#pragma once

#include <avr/io.h>
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

    const char* ResultString(Result res)
    {
        switch (res)
        {
            case Result::OK:             return "Ok";
            case Result::Invalid:        return "Invalido";
            case Result::NotInitialized: return "Não Iniciado";
            case Result::NoResponse:     return "Sem Resposta";
            case Result::TimeOut:        return "TimeOut";
            case Result::ChecksumError:  return "Checksum";
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

    Result init(volatile uint8_t &port, volatile uint8_t &ddr, volatile uint8_t &pin, uint8_t pinIndex)
    {
        if (pinIndex >= 8 || m_port != nullptr)
            return Result::Invalid;

        m_port = &port;
        m_ddr = &ddr;
        m_pin = &pin;
        m_pinIndex = pinIndex;

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            m_clearBit(*m_ddr, m_pinIndex); // Input
            m_setBit(*m_port, m_pinIndex);  // Internal pull-up
        }

        _delay_ms(1000);

        return Result::OK;
    }

    Result read(Data &out)
    {
        if (!m_port)
            return Result::NotInitialized;

        // 5.2 - MCU Sends out Start Signal to DHT
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            m_clearBit(*m_port, m_pinIndex);
            m_setBit(*m_ddr, m_pinIndex);
        }
        _delay_ms(20);

        uint8_t data[5] = {};
        Result result = Result::OK;

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            m_clearBit(*m_ddr, m_pinIndex); // Release DATA line.
            m_setBit(*m_port, m_pinIndex);  // Internal pull-up.
            
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

                    const bool bit = m_isBitSet(*m_pin, m_pinIndex);
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
                volatile uint8_t *m_port = nullptr;
                volatile uint8_t *m_ddr = nullptr;
                volatile uint8_t *m_pin = nullptr;

                uint8_t m_pinIndex = 0;

                inline void m_setBit(volatile uint8_t &REG, uint8_t BIT)
                {
                    REG |= (1U << BIT);
                }

                inline void m_clearBit(volatile uint8_t &REG, uint8_t BIT)
                {
                    REG &= ~(1U << BIT);
                }

                inline bool m_isBitSet(volatile uint8_t &REG, uint8_t BIT)
                {
                    return ((REG & (1U << BIT)) != 0);
                }

                inline bool waitLevel(bool high)
                {
                    uint8_t timeout = 120;

                    while (m_isBitSet(*m_pin, m_pinIndex) != high)
                    {
                        if (--timeout == 0)
                            return false;

                        _delay_us(1);
                    }

                    return true;
                }
            };

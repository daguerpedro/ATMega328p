#pragma once

#define twi (TWI::getInstance())

#include <avr/io.h>

enum TWI_STATUS : uint8_t
{
    BUS_ERROR = 0x00,

    START_SENT = 0x08,
    REPEATED_START_SENT = 0x10,

    SLAVE_ADDR_SENT_ACK = 0x18,
    SLAVE_ADDR_SENT_NACK = 0x20,

    DATA_SENT_ACK = 0x28,
    DATA_SENT_NACK = 0x30,

    ARBITRATION_LOST = 0x38,

    SLAVE_ADDR_RECEIVED_ACK = 0x40,
    SLAVE_ADDR_RECEIVED_NACK = 0x48,

    DATA_RECEIVED_ACK = 0x50,
    DATA_RECEIVED_NACK = 0x58
};

enum TWI_PRESCALER : uint8_t
{
    PS_1 = 0x00,
    PS_4 = 0x01,
    PS_16 = 0x02,
    PS_64 = 0x03
};

class TWI
{
public:
    static TWI &getInstance()
    {
        static TWI instance;
        return instance;
    };

    TWI(const TWI &) = delete;
    TWI &operator=(const TWI &) = delete;

private:
    TWI_STATUS tw_lastStatus;

    inline bool tw_waitTransmission()
    {
        // TEMP: Usar timer.
        uint16_t timeout = 60000;
        // O bit TWINT é setado para 1 quando a informação é transimitada.
        // Lemos isso no TWCR
        while (!(TWCR & (1 << TWINT)))
        {
            if (--timeout == 0)
                return false;
        }
        return true;
    }

    inline TWI_STATUS tw_getStatus()
    {
        // Verifica se o status de TWSR (mascarando o prescaler: ultimos 2 bits + 1 bit ignorado)
        tw_lastStatus = static_cast<TWI_STATUS>(TWSR & 0xF8);
        return tw_lastStatus;
    }

    inline void tw_write(uint8_t data)
    {
        TWDR = data;
        TWCR = (1 << TWINT) | (1 << TWEN);
    }

    TWI() : tw_lastStatus(START_SENT) {};

public:
    TWI_STATUS getLastStatus()
    {
        return tw_lastStatus;
    }

    bool init(uint32_t frequency, TWI_PRESCALER scale)
    {
        if (frequency == 0 || (F_CPU / frequency) <= 16)
            return false; // Não faz sentido e evita divisão por zero
        if (scale > 0x03)
            return false; // Existem somente 4 configurações de prescaler

        // Table 22-7. TWI Bit Rate Prescaler
        const uint8_t prescaler_vals[] = {1, 4, 16, 64};

        // Setando os ultimos dois bits de twsr = setar prescaler
        TWSR = (TWSR & 0xFC) | scale;

        // 22.5.2 Bit Rate Generator Unit
        uint32_t aux = 2UL * prescaler_vals[static_cast<uint8_t>(scale)];
        uint32_t bit_rate = ((F_CPU / frequency) - 16) / aux;

        if (bit_rate > 255)
            return false;

        TWBR = static_cast<uint8_t>(bit_rate);

        return true;
    };

    bool startCondition()
    {
        // Escrever no TWCR com o bit TWINT setado.
        // SETAR O BIT TWINT LIMPA A FLAG TWINT
        TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

        if (!tw_waitTransmission())
            return false;

        TWI_STATUS status = tw_getStatus();

        return (
            status == START_SENT ||
            status == REPEATED_START_SENT);
    }

    void stopCondition()
    {
        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    }

    /// @brief Envia para o TWI o endereço do SLAVE
    /// @param address Endereço 7 bits
    /// @param read true: LEITURA | false: ESCRITA
    /// @return
    bool sendAddress(uint8_t address, bool read = false)
    {
        if (address > 0x7F)
            return false;

        // If SLA+W is transmitted, MT mode is entered, if SLA+R is transmitted, MR mode is entered.
        uint8_t data = (address << 1) | (read ? 1 : 0);

        tw_write(data);

        if (!tw_waitTransmission())
            return false;

        return (tw_getStatus() == (read ? SLAVE_ADDR_RECEIVED_ACK : SLAVE_ADDR_SENT_ACK));
    }

    bool sendData(uint8_t data)
    {
        tw_write(data);

        if (!tw_waitTransmission())
            return false;

        return (tw_getStatus() == DATA_SENT_ACK);
    }

    bool readDataACK(uint8_t &outdata)
    {
        // Received data can be read from the TWDR Register when the TWINT Flag is set high by hardware.
        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA); // TWEA: ENABLE ACK

        if (!tw_waitTransmission())
            return false;

        if (tw_getStatus() != DATA_RECEIVED_ACK)
            return false;

        outdata = TWDR;
        return true;
    }

    bool readDataNACK(uint8_t &outdata)
    {
        // Received data can be read from the TWDR Register when the TWINT Flag is set high by hardware.
        TWCR = (1 << TWINT) | (1 << TWEN);

        if (!tw_waitTransmission())
            return false;

        if (tw_getStatus() != DATA_RECEIVED_NACK)
            return false;

        outdata = TWDR;
        return true;
    }
};

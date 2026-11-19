#pragma once

#define twi (TWI::getInstance())

#include <avr/io.h>
#include <stddef.h>

/*
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
};*/

class TWI
{
public:
    static constexpr uint16_t TIMEOUT = 60000;

    enum class Result : uint8_t
    {
        None,
        OK,
        Invalid,
        BusError,
        TimeOut,
        AddressNack,
        DataNack,
        ArbitrationLost,
        Unexpected,
        NotInitialized
    };

    enum class Prescaler : uint8_t
    {
        PS_1 = 0x00,
        PS_4 = 0x01,
        PS_16 = 0x02,
        PS_64 = 0x03
    };

    // Singleton pois ATMega328p só possui 1 periférico TWI
    static TWI &getInstance()
    {
        static TWI instance;
        return instance;
    };

    // Proibido cópias em Singletons
    TWI(const TWI &) = delete;
    TWI &operator=(const TWI &) = delete;

    Result init(uint32_t frequency, Prescaler scale)
    {
        initialized_ = false;

        const uint8_t scaler = static_cast<uint8_t>(scale);
        if (frequency == 0 || scaler > 3)
            return Result::Invalid; // Não faz sentido e evita divisão por zero; Existem somente 4 configurações de prescaler

        uint32_t div = F_CPU / frequency;
        if (div <= 16)
            return Result::Invalid;

        // Table 22-7. TWI Bit Rate Prescaler
        const uint8_t prescaler_vals[] = {1, 4, 16, 64};
        uint32_t prescaler = prescaler_vals[scaler];

        // 22.5.2 Bit Rate Generator Unit
        uint32_t bit_rate = (div - 16) / (2UL * prescaler);
        if (bit_rate < 10 || bit_rate > 255)
            return Result::Invalid;

        uint8_t pins = (1 << PC4) | (1 << PC5);
        DDRC &= ~pins; // Configura SDA e SCL como entradas
        PORTC |= pins; // Pull Up Interno

        PRR &= ~(1 << PRTWI); // Ativa o clock do periférico TWI

        // Setando os ultimos dois bits de twsr = setar prescaler
        TWSR = (TWSR & 0xFC) | scaler;
        TWBR = static_cast<uint8_t>(bit_rate);

        TWCR = (1 << TWEN); // Ativa TWI

        initialized_ = true;
        return Result::OK;
    };

    Result write(uint8_t address, const uint8_t *data, size_t length)
    {
        if (!initialized_)
            return Result::NotInitialized;

        if (address > 0x7F || (length > 0 && data == nullptr))
            return Result::Invalid;

        Result result = start(false);
        if (result != Result::OK)
            return finish(result);

        result = sendAddress(address, false);
        if (result != Result::OK)
            return finish(result);

        result = sendData(data, length);
        return finish(result);
    }

    Result read(uint8_t address, uint8_t *data, size_t length)
    {
        if (!initialized_)
            return Result::NotInitialized;

        if (address > 0x7F || length == 0 || data == nullptr)
            return Result::Invalid;

        Result result = start(false);
        if (result != Result::OK)
            return finish(result);

        result = sendAddress(address, true);
        if (result != Result::OK)
            return finish(result);

        result = readData(data, length);
        return finish(result);
    }

    Result writeRead(uint8_t address, const uint8_t *tx, size_t txLen, uint8_t *rx, size_t rxLen)
    {
        if (!initialized_)
            return Result::NotInitialized;

        if (address > 0x7F || txLen == 0 || rxLen == 0 || tx == nullptr || rx == nullptr)
            return Result::Invalid;

        Result result = start(false);
        if (result != Result::OK)
            return finish(result);

        result = sendAddress(address, false);
        if (result != Result::OK)
            return finish(result);

        result = sendData(tx, txLen);
        if (result != Result::OK)
            return finish(result);

        // Repeated START, sem STOP intermediário
        result = start(true);
        if (result != Result::OK)
            return finish(result);

        result = sendAddress(address, true);
        if (result != Result::OK)
            return finish(result);

        result = readData(rx, rxLen);
        return finish(result);
    }

private:
    bool initialized_ = false;

    Result start(bool repeated)
    {
        // Escrever no TWCR com o bit TWINT setado.
        // SETAR O BIT TWINT LIMPA A FLAG TWINT
        TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

        Result result = tw_wait();
        if (result != Result::OK)
            return result;

        uint8_t status = tw_status();
        if (status == (repeated ? 0x10 : 0x08))
            return Result::OK;

        return unexpected();
    }

    Result stop()
    {
        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);

        uint16_t timeout = TIMEOUT;
        while (TWCR & (1 << TWSTO))
        {
            if (--timeout == 0)
                return Result::TimeOut;
        }

        return Result::OK;
    }

    /// @brief Envia para o TWI o endereço do SLAVE
    /// @param address Endereço 7 bits
    /// @param read true: LEITURA | false: ESCRITA
    /// @return
    Result sendAddress(uint8_t address, bool read)
    {
        // If SLA+W is transmitted, MT mode is entered, if SLA+R is transmitted, MR mode is entered.
        uint8_t data = (address << 1) | (read ? 1 : 0);
        tw_write(data);

        Result result = tw_wait();
        if (result != Result::OK)
            return result;

        uint8_t code = tw_status();
        if (code == (read ? 0x40 : 0x18))
            return Result::OK;

        if (code == (read ? 0x48 : 0x20))
            return Result::AddressNack;

        return unexpected();
    }

    Result sendByte(uint8_t data)
    {
        tw_write(data);

        Result result = tw_wait();
        if (result != Result::OK)
            return result;

        uint8_t code = tw_status();

        if (code == 0x28)
            return Result::OK;

        if (code == 0x30)
            return Result::DataNack;

        return unexpected();
    }

    Result sendData(const uint8_t *data, size_t length)
    {
        for (size_t i = 0; i < length; ++i)
        {
            Result result = sendByte(data[i]);

            if (result != Result::OK)
                return result;
        }

        return Result::OK;
    }

    Result readData(uint8_t *data, size_t length)
    {
        for (size_t i = 0; i < length; ++i)
        {
            bool ack = (i + 1 < length); // Ultima leitura deve conter NACK

            Result result = readByte(data[i], ack);

            if (result != Result::OK)
                return result;
        }

        return Result::OK;
    }

    Result readByte(uint8_t &outdata, bool ack)
    {
        // Received data can be read from the TWDR Register when the TWINT Flag is set high by hardware.
        TWCR = (1 << TWINT) | (1 << TWEN) | (ack ? (1 << TWEA) : 0); // TWEA: ENABLE ACK

        Result result = tw_wait();
        if (result != Result::OK)
            return result;

        uint8_t status = tw_status();
        if (status != (ack ? 0x50 : 0x58))
            return unexpected();

        outdata = TWDR;
        return Result::OK;
    }

    Result tw_wait()
    {
        uint16_t timeout = TIMEOUT;
        // O bit TWINT é setado para 1 quando a informação é transimitada.
        // Lemos isso no TWCR
        while (!(TWCR & (1 << TWINT)))
        {
            if (--timeout == 0)
                return Result::TimeOut;
        }
        return Result::OK;
    }

    uint8_t tw_status()
    {
        // Verifica se o status de TWSR (mascarando o prescaler: ultimos 2 bits + 1 bit ignorado)
        return (TWSR & 0xF8);
    }

    void tw_write(uint8_t data)
    {
        TWDR = data;
        TWCR = (1 << TWINT) | (1 << TWEN);
    }

    Result unexpected()
    {
        const uint8_t code = tw_status();

        if (code == 0x38)
            return Result::ArbitrationLost;
        if (code == 0x00)
            return Result::BusError;

        return Result::Unexpected;
    }

    Result finish(Result result)
    {
        switch (result)
        {
        case Result::ArbitrationLost:
            // Outro master assumiu o barramento.
            // Não podemos transmitir STOP.
            return result;

        case Result::BusError:
            // Recupera o estado interno do TWI.
            // Preserva BusError se a recuperação funcionar.
            if (stop() != Result::OK)
                return Result::TimeOut;
            return Result::BusError;

        case Result::OK:
            // Só considera sucesso quando o STOP
            // também foi concluído.
            return stop();

        case Result::AddressNack:
        case Result::DataNack:
        case Result::Unexpected:
            // Tenta liberar o barramento.
            // Preserva a causa original se STOP funcionar.
            if (stop() != Result::OK)
                return Result::TimeOut;
            return result;

        case Result::TimeOut:
            // Tentativa limitada de encerramento.
            // STOP pode falhar se o barramento travou.
            (void)stop();
            return Result::TimeOut;

        default:
            return result;
        }
    }

    TWI() {};
};

#pragma once

#include "twi.hpp"

class SSD1306
{
public:
    SSD1306() {

    };

    bool init(uint8_t address)
    {
        if (address > 0x7F)
            return false;

        if (!twi.init(100000, TWI_PRESCALER::PS_1))
            return false;

        if (!twi.startCondition())
            return false;

        // 8.1.5 MCU I2C Interface: SSD1306 has to recognize the slave address before transmitting or receiving any information
        if (!twi.sendAddress(address))
        {
            twi.stopCondition();
            return false;
        }
        twi.stopCondition();

        // Sequencia de inicialização
        const uint8_t setupCommands[] = {
            0xAE, // Display OFF

            0xD5, 0x80, // Display clock divide ratio
            0xA8, 0x3F, // Multiplex ratio: 1/64
            0xD3, 0x00, // Display offset: 0
            0x40,       // Start line: 0

            0x8D, 0x14, // Enable charge pump

            0x20, 0x02, // Page Addressing Mode

            0xA1, // Segment remap
            0xC8, // COM scan direction

            0xDA, 0x12, // COM pins hardware configuration

            0x81, 0x7F, // Contrast control

            0xD9, 0xF1, // Pre-charge period
            0xDB, 0x40, // VCOMH deselect level

            0xA4, // Resume to RAM content display
            0xA6, // Normal display

            0xAF // Display ON

        };

        sendCommands(
            setupCommands,
            sizeof(setupCommands));

        m_address = address;
        return true;
    }

    bool setPosition(uint8_t page, uint8_t column)
    {
        if (page > 7 || column > 127)
            return false;

        const uint8_t commands[] =
            {
                static_cast<uint8_t>(0xB0 | page),
                static_cast<uint8_t>(0x00 | (column & 0x0F)),
                static_cast<uint8_t>(0x10 | ((column >> 4) & 0x0F))};

        return sendCommands(commands, sizeof(commands));
    }

    bool clearDisplay()
    {
        uint8_t buffer[128] = {0};

        for (uint8_t page = 0; page < 8; page++)
        {
            if (!setPosition(page, 0))
                return false;

            if (!sendBuffer(buffer, sizeof(buffer)))
                return false;
        }

        return true;
    }

    bool updateDisplay(const uint8_t *framebuffer)
    {
        if (framebuffer == nullptr)
            return false;

        for (uint8_t page = 0; page < 8; page++)
        {
            uint16_t offset = static_cast<uint16_t>(page) * 128;

            if (!setPosition(page, 0))
                return false;

            if (!sendBuffer(&framebuffer[offset], 128))
                return false;
        }

        return true;
    }

private:
    uint8_t m_address;

    /*
    If D/C# pin is HIGH, D[7:0] is interpreted as display data written to Graphic Display Data RAM (GDDRAM).
    If it is LOW, the input at D[7:0] is interpreted as a command. Then data input will be decoded and written to
    the corresponding command register.
    */
    bool sendCommand(uint8_t cmd)
    {
        if (!twi.startCondition())
            return false;

        if (!twi.sendAddress(m_address))
            return abortTransaction();

        // 8.1.5.2 Write mode for I2C: 5) A)
        // Direciona para o comando
        if (!twi.sendData(0x00))
            return abortTransaction();

        if (!twi.sendData(cmd))
            return abortTransaction();

        twi.stopCondition();
        return true;
    }

    bool sendData(uint8_t data)
    {
        if (!twi.startCondition())
            return false;

        if (!twi.sendAddress(m_address))
            return abortTransaction();

        // 8.1.5.2 Write mode for I2C: 5) B)
        // Direciona para a GDDRAM
        if (!twi.sendData(0x40))
            return abortTransaction();

        if (!twi.sendData(data))
            return abortTransaction();

        twi.stopCondition();
        return true;
    }

    bool sendBuffer(const uint8_t *data, uint16_t size)
    {
        if (!twi.startCondition())
            return false;

        if (!twi.sendAddress(m_address))
            return abortTransaction();

        if (!twi.sendData(0x40))
            return abortTransaction();

        for (uint16_t i = 0; i < size; i++)
        {
            if (!twi.sendData(data[i]))
                return abortTransaction();
        }

        twi.stopCondition();
        return true;
    }

    bool sendCommands(const uint8_t *cmds, uint16_t size)
    {
        if (!twi.startCondition())
            return false;

        if (!twi.sendAddress(m_address))
            return abortTransaction();

        if (!twi.sendData(0x00))
            return abortTransaction();

        for (uint16_t i = 0; i < size; i++)
        {
            if (!twi.sendData(cmds[i]))
                return abortTransaction();
        }

        twi.stopCondition();
        return true;
    }

    bool abortTransaction()
    {
        twi.stopCondition();
        return false;
    }
};

/*
0x81: Set Contrast Control
0xA4: Entire Display ON
0xA5: Entire Display ON
0xA6: Set Normal/Inverse Display
0xA7: Set Normal/Inverse Display
0xAE: Set Display ON/OFF
0xAF: Set Display ON/OFF
0x26: Continuous Horizontal Scroll Setup
0x27: Continuous Horizontal Scroll Setup
0x29: Continuous Vertical and Horizontal Scroll Setup
0x2A: Continuous Vertical and Horizontal Scroll Setup
0x2E: Deactivate scroll
0x2F: Activate scroll
0xA3: Set Vertical Scroll Area
0x00: Set Lower Column Start Address for Page Addressing Mode (00h~0Fh)
0x10: Set Higher Column Start Address for Page Addressing Mode (10h~1Fh)
0x20: Set Memory Addressing Mode
0x21: Set Column Address
0x22: Set Page Address
0xB0: Set Page Start Address for Page Addressing Mode (B0h~B7h)
0x40: Set Display Start Line (40h~7Fh)
0xA0: Set Segment Re-map
0xA1: Set Segment Re-map
0xA8: Set Multiplex Ratio
0xC0: Set COM Output Scan Direction
0xC8: Set COM Output Scan Direction
0xD3: Set Display Offset
0xDA: Set COM Pins Hardware Configuration
0xD5: Set Display Clock Divide Ratio/Oscillator Frequency
0xD9: Set Pre-charge Period
0xDB: Set VCOMH Deselect Level
0xE3: NOP
*/
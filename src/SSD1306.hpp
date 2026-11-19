#pragma once

#include "twi.hpp"
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include "font.h"
#include <stdlib.h>

class SSD1306
{
public:
    static constexpr uint8_t WIDTH = 128;
    static constexpr uint8_t HEIGHT = 64;

    uint8_t pageBuffer_[WIDTH] = {};
    uint8_t currentPage_ = 0;

    SSD1306() = default;

    bool init(uint8_t address)
    {
        address_ = address;
        initialized_ = false;

        // 8.1.5 MCU I2C Interface: SSD1306 has to recognize the slave address before transmitting or receiving any information
        if (twi.write(address, nullptr, 0) != TWI::Result::OK)
            return false;

        const uint8_t setupCommands[] = {
            0xAE,       // Display OFF
            0xD5, 0x80, // Clock divide ratio
            0xA8, 0x3F, // Multiplex 1/64
            0xD3, 0x00, // Display offset
            0x40,       // Start line
            0x8D, 0x14, // Charge pump
            0x20, 0x02, // Page Addressing Mode
            0xA1,       // Segment remap
            0xC8,       // COM scan direction
            0xDA, 0x12, // COM pins config
            0x81, 0x7F, // Contrast
            0xD9, 0xF1, // Pre-charge
            0xDB, 0x40, // VCOMH
            0xA4,       // Display follows GDDRAM
            0xA6,       // Normal display
            0xAF        // Display ON
        };

        if (!sendCommands(setupCommands, sizeof(setupCommands)))
            return false;

        initialized_ = true;
        return true;
    }

    bool setRotation(bool inverted)
    {
        if (!initialized_)
            return false;

        const uint8_t commands[] = {
            static_cast<uint8_t>(inverted ? 0xA0 : 0xA1),
            static_cast<uint8_t>(inverted ? 0xC0 : 0xC8)};

        return sendCommands(commands, sizeof(commands));
    }

    bool setPosition(uint8_t page, uint8_t column)
    {
        if (!initialized_ || page > 7 || column > 127)
            return false;

        const uint8_t commands[] = {
            static_cast<uint8_t>(0xB0 | page),
            static_cast<uint8_t>(0x00 | (column & 0x0F)),
            static_cast<uint8_t>(0x10 | (column >> 4))};

        return sendCommands(commands, sizeof(commands));
    }

    bool clearDisplay()
    {
        if (!initialized_)
            return false;

        const uint8_t zeros[kChunkSize] = {};

        for (uint8_t page = 0; page < 8; ++page)
        {
            if (!setPosition(page, 0))
                return false;

            for (uint8_t offset = 0; offset < 128; offset += kChunkSize)
            {
                if (!sendBuffer(zeros, kChunkSize))
                    return false;
            }
        }

        return true;
    }

    bool updateDisplay(const uint8_t *framebuffer)
    {
        if (!initialized_ || framebuffer == nullptr)
            return false;

        for (uint8_t page = 0; page < 8; ++page)
        {
            if (!setPosition(page, 0))
                return false;

            const size_t pageOffset = static_cast<size_t>(page) * 128U;
            if (!sendBuffer(framebuffer + pageOffset, 128U))
                return false;
        }

        return true;
    }

    bool writeChar(uint8_t page, uint8_t column, char c)
    {
        constexpr uint8_t FONT_HEADER_SIZE = 6;
        constexpr uint8_t FONT_WIDTH = 5;
        constexpr uint8_t CHAR_WIDTH = 6;

        if (page > 7 || column > 128 - CHAR_WIDTH)
            return false;

        uint8_t ascii = static_cast<uint8_t>(c);

        if (ascii < 0x20 || ascii > 0x7F)
            ascii = '?';

        uint16_t offset =
            FONT_HEADER_SIZE +
            static_cast<uint16_t>(ascii - 0x20) * FONT_WIDTH;

        uint8_t data[CHAR_WIDTH];

        for (uint8_t i = 0; i < FONT_WIDTH; ++i)
        {
            data[i] = pgm_read_byte(&font5x7[offset + i]);
        }

        data[5] = 0x00; // Espacamento entre caracteres

        if (!setPosition(page, column))
            return false;

        return sendBuffer(data, sizeof(data));
    }

    bool writeString(uint8_t page, uint8_t column, const char *str)
    {
        if (str == nullptr)
            return false;

        while (*str)
        {
            if (column > 122)
            {
                column = 0;
                ++page;
            }

            if (page > 7)
                return false;

            if (!writeChar(page, column, *str))
                return false;

            column += 6;
            ++str;
        }

        return true;
    }

    bool printf(uint8_t page, uint8_t column, const char *format, ...)
    {
        if (format == nullptr)
            return false;

        char buffer[32];

        va_list args;
        va_start(args, format);

        int length = vsnprintf(
            buffer,
            sizeof(buffer),
            format,
            args);

        va_end(args);

        if (length < 0 || length >= static_cast<int>(sizeof(buffer)))
            return false;

        return writeString(page, column, buffer);
    }

    void drawPixel(int16_t x, int16_t y, bool color = true)
    {
        if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
            return;

        uint8_t page = static_cast<uint8_t>(y / 8);

        if (page != currentPage_)
            return;

        uint8_t bit = static_cast<uint8_t>(y % 8);
        uint8_t mask = static_cast<uint8_t>(1U << bit);

        if (color)
            pageBuffer_[x] |= mask;
        else
            pageBuffer_[x] &= static_cast<uint8_t>(~mask);
    }

    void drawLine(int16_t x0, int16_t y0,
                  int16_t x1, int16_t y1,
                  bool color = true)
    {
        int16_t dx = abs(x1 - x0);
        int16_t sx = x0 < x1 ? 1 : -1;

        int16_t dy = -abs(y1 - y0);
        int16_t sy = y0 < y1 ? 1 : -1;

        int16_t err = dx + dy;

        while (true)
        {
            drawPixel(x0, y0, color);

            if (x0 == x1 && y0 == y1)
                break;

            int16_t e2 = 2 * err;

            if (e2 >= dy)
            {
                err += dy;
                x0 += sx;
            }

            if (e2 <= dx)
            {
                err += dx;
                y0 += sy;
            }
        }
    }

    void drawBitmap(int16_t x, int16_t y,
                    uint8_t width, uint8_t height,
                    const uint8_t *bitmap)
    {
        if (bitmap == nullptr)
            return;

        uint8_t pages = (height + 7) / 8;

        for (uint8_t p = 0; p < pages; ++p)
        {
            for (uint8_t col = 0; col < width; ++col)
            {
                size_t index =
                    static_cast<size_t>(p) * width + col;

                uint8_t data = pgm_read_byte(&bitmap[index]);

                for (uint8_t bit = 0; bit < 8; ++bit)
                {
                    uint16_t localY =
                        static_cast<uint16_t>(p) * 8 + bit;

                    if (localY >= height)
                        break;

                    if (data & (1U << bit))
                    {
                        drawPixel(
                            x + col,
                            y + localY,
                            true);
                    }
                }
            }
        }
    }

    void beginPage(uint8_t page)
    {
        currentPage_ = page;

        for (uint8_t i = 0; i < WIDTH; ++i)
            pageBuffer_[i] = 0;
    }

    bool flushPage()
    {
        if (currentPage_ >= 8)
            return false;

        if (!setPosition(currentPage_, 0))
            return false;

        return sendBuffer(pageBuffer_, WIDTH);
    }

private:
    static constexpr uint8_t kChunkSize = 32;

    uint8_t address_ = 0;
    bool initialized_ = false;

    // Control byte SSD1306: Co=0; D/C#=0 para comandos, 1 para dados.
    bool sendPacket(uint8_t control, const uint8_t *data, size_t length)
    {
        if (length != 0 && data == nullptr)
            return false;

        uint8_t packet[kChunkSize + 1];
        packet[0] = control;

        while (length != 0)
        {
            const uint8_t count = length > kChunkSize
                                      ? kChunkSize
                                      : static_cast<uint8_t>(length);

            for (uint8_t i = 0; i < count; ++i)
                packet[i + 1] = data[i];

            if (twi.write(address_, packet, static_cast<size_t>(count) + 1) != TWI::Result::OK)
                return false;

            data += count;
            length -= count;
        }

        return true;
    }

    bool sendCommands(const uint8_t *commands, size_t length)
    {
        return sendPacket(0x00, commands, length);
    }

    bool sendBuffer(const uint8_t *data, size_t length)
    {
        return sendPacket(0x40, data, length);
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
#define F_CPU 16000000UL

#include <ssd1306.h>
#include <ssd1306_fonts.h>

int main()
{
    ssd1306_128x64_i2c_initEx(-1, -1, 0x3C);

    ssd1306_clearScreen();
    ssd1306_setFixedFont(ssd1306xled_font6x8);

    ssd1306_flipVertical(1);
    ssd1306_flipHorizontal(1);

    ssd1306_printFixed(0, 0, "SSD1306 funcionando", STYLE_NORMAL);
    ssd1306_drawHLine(0, 9, 127);

    while (1)
    {
        /* code */
    }
}
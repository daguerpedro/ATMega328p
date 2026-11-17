#define F_CPU 16000000UL

#include "SSD1306.hpp"

int main()
{
    SSD1306 display;
    display.init(0x3C); // 0x78 >> 1
    display.clearDisplay();
    
    while (1)
    {

    }
}
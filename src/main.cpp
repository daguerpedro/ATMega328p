#define F_CPU 16000000UL

#include "system.h"

int main()
{
    System& sys = System::getInstance();
    
    sys.startPeripherals();
    sys.startDevices();

    sei();

    sys.drawScreen();
}
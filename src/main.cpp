#define F_CPU 16000000UL

#include "system.h"

int main()
{
    cli();
    
    System& sys = System::getInstance();
    
    sys.startDevices();
    sys.startPeripherals();

    sei();

    sys.run();
}
#define F_CPU 16000000UL

#include "funsape/globalDefines.hpp"
#include "funsape/peripheral/adc.hpp"
#include "funsape/peripheral/timer0.hpp"
#include "funsape/peripheral/gpioBus.hpp"
#include "funsape/peripheral/gpioPin.hpp"
#include "funsape/device/hd44780.hpp"

Hd44780 display;

int main()
{
    GpioBus bData;
    GpioPin pCtrlE;
    GpioPin pCtrlRs;

    pCtrlE.init(&PIND,  PinIndex::P2);
    pCtrlRs.init(&PIND, PinIndex::P3);
    bData.init(&PIND, PinIndex::P4, 4);
    display.setControlPort(&pCtrlE, &pCtrlRs);
    display.setDataPort(&bData);
    display.init(Hd44780::Size::LCD_16X2);
    
    display.stdio();
    printf("Iniciando");

    timer0.activateOverflowInterrupt();
    timer0.init(Timer0::Mode::NORMAL, Timer0::ClockSource::PRESCALER_1024);
    
    adc.setChannel(Adc::Channel::CHANNEL_0);
    adc.init(Adc::Mode::AUTO_TIMER0_OVERFLOW, Adc::Reference::INTERNAL, Adc::Prescaler::PRESCALER_128);
    adc.activateInterrupt();
    adc.enable();
    
    adc.startConversion(); //discard
    adc.waitUntilConversionFinish();

    sei();

    display.clearScreen();

    while (1)
    {

    }
}

void adcConversionCompleteCallback()
{
    timer0.clearOverflowInterruptRequest();

    uint16_t raw = adc.getValue();
    if(raw < 1) raw = 1;

    uint32_t temp = raw-1;
    temp *= (1100/4);
    temp /= (1024/4);

    display.cursorGoTo(0,0);

    printf("Temp: %lu.%1lu", temp/10, temp%10);
}

/*
    f/(f+s) = v
    f/v = (f+s)
    f(1/v - 1) = s
    
    v   = vin/vcc
    1/v = vcc/vin

    f(vcc/vin - 1) = s

    f = Rf
    s = Rs

    Rs = Rf(vcc/vin - 1)
*/
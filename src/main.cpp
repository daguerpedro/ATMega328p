#define F_CPU 16000000UL

#include "funsape/globalDefines.hpp"
#include "funsape/peripheral/gpioBus.hpp"
#include "funsape/peripheral/gpioPin.hpp"
#include "funsape/peripheral/timer0.hpp"
#include "funsape/peripheral/pcint0.hpp"
#include "funsape/device/sevenSegmentsMuxDisplay.hpp"

#include "counter.hpp"

Counter counter(2048);
GpioBus buttons;

volatile uint8_t buttonsState;

int main()
{
    counter.init(&PIND, PinIndex::P0, &PORTC, PinIndex::P0);

    //Modo CTC para reiniciar o timer quando chegar em OCRA;
    timer0.init(Timer0::Mode::CTC_OCRA, Timer0::ClockSource::PRESCALER_1024);
    timer0.activateCompareAInterrupt();
    timer0.setCompareAValue(77); //Calcule usando Fcpu, Fprescaler, Max | CompAval

    buttons.init(&PINB, PinIndex::P0, 4);
    buttons.setMode(PinMode::INPUT_PULLED_UP);

    pcint0.activateInterrupt();
    pcint0.init(Pcint0::Pin::PIN_PCINT0 | Pcint0::Pin::PIN_PCINT1 | Pcint0::Pin::PIN_PCINT2 | Pcint0::Pin::PIN_PCINT3);

    buttonsState = buttons.read(); //Salva o estado inicial dos botões
    
    sei();

    while (1)
    {
    }
}

void timer0CompareACallback()
{
    counter.display();
};

void pcint0InterruptCallback()
{
    uint8_t newButtonsState = buttons.read();
    auto diff = buttonsState ^ newButtonsState; //Mudança dos estados de botões

    if (isBitSet(diff, 0) && isBitClr(newButtonsState, 0)) //Garante apenas o botão 0 quando desce;
        counter.increment(1);

    if (isBitSet(diff, 1) && isBitClr(newButtonsState, 1)) //Garante apenas o botão 1 quando desce;
        counter.increment(10);

    if (isBitSet(diff, 2) && isBitClr(newButtonsState, 2)) //Garante apenas o botão 2 quando desce;
        counter.increment(100);

    if (isBitSet(diff, 3) && isBitClr(newButtonsState, 3)) //Garante apenas o botão 3 quando desce;
        counter.increment(1000);

    buttonsState = newButtonsState;
};
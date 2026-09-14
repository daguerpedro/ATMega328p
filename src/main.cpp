#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <avr/interrupt.h>

#include "defs.h"
#include "animation.h"

int main()
{
    // Configurações
    DDRD = 255; // TODOS PINOS D COMO SAÍDA

    sei();               // Ativa Interrupções
    PCICR = 0b00000001;  // Ativa Interrupções para o grupo PB
    PCMSK0 = 0b00000011; // Seleciona Apenas PB0(PCINT0) e PB1(PCINT1)

    while (true)
    {
        animate();
        _delay_ms(250);
    }
}

ISR(PCINT0_vect)
{
    uint8_t estadoPINB = PINB;
    if (isBitSet(estadoPINB, 0))
        toggleAnimDir();

    if (isBitSet(estadoPINB, 1))
        toggleAnimFreeze();
}

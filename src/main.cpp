#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <avr/interrupt.h>

#include "defs.h"
#include "animation.h"

volatile uint8_t estadoPINB;
int main()
{
    // Configurações
    DDRD = 255; // TODOS PINOS D COMO SAÍDA
    setBit(PORTB, 0);
    setBit(PORTB, 1);

    sei();               // Ativa Interrupções
    PCICR = 0b00000001;  // Ativa Interrupções para o grupo PB
    PCMSK0 = 0b00000011; // Seleciona Apenas PB0(PCINT0) e PB1(PCINT1)

    estadoPINB = PINB;

    while (true)
    {
        animate();
        _delay_ms(250);
    }
}

ISR(PCINT0_vect)
{
    uint8_t novoEstado = PINB;
    uint8_t diff = estadoPINB ^ novoEstado;
    estadoPINB = novoEstado;

    if (isBitSet(diff, 0) && isBitClear(novoEstado, 0))
        toggleAnimDir();

    if (isBitSet(diff, 1) && isBitClear(novoEstado, 1))
        toggleAnimFreeze();
}

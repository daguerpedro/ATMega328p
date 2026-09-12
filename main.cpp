#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <avr/interrupt.h>

#define setBit(REG, BIT)    ((REG) |= 1 << (BIT))
#define clearBit(REG, BIT)  ((REG) &= ~(1 << (BIT)))
#define toggleBit(REG, BIT) ((REG) ^= 1 << (BIT))
#define isBitSet(REG, BIT) (((REG) & 1 << (BIT)) != false)
#define isBitClear(REG, BIT) (!isBitSet(REG, BIT))

#define FREEZE  0b0001
#define RIGHT   0b0010

volatile unsigned char anim_settings = 0b0000;

inline void freezeAnim()
{
    anim_settings |= FREEZE;
}

inline void unfreezeAnim()
{
    anim_settings &= ~FREEZE;
}

inline void animateRight()
{
    anim_settings |= RIGHT;
}

inline void animateLeft()
{
    anim_settings &= ~RIGHT;
}

inline void toggleAnimDir()
{
    toggleBit(anim_settings, 1);
}

inline void toggleAnimFreeze()
{
    toggleBit(anim_settings, 0);
}

int main()
{
    //Configurações

    DDRD  = 255;                //TODOS PINOS D COMO SAÍDA
    volatile uint8_t anim = 0;  //PRIMEIRO FRAME DA ANIMAÇÃO

    sei();                  //Ativa Interrupções
    PCICR   = 0b00000001;   //Ativa Interrupções para o grupo PB
    PCMSK0  = 0b00000011;   //Seleciona Apenas PB0(PCINT0) e PB1(PCINT1)

    while (true)
    {
        PORTD = anim;
        uint8_t settings;

        ATOMIC_BLOCK(ATOMIC_FORCEON)
        {
            settings = anim_settings;
        }

        if((settings & FREEZE) == false)
        {
            if(settings & RIGHT)
            {
                anim >>= 1;

                if(anim == 0)
                    anim = 0b10000000;
            }
            else
            {
                anim <<= 1;
                if(anim == 0)
                    anim = 1;
            }
        }

        _delay_ms(250);
    }

}

ISR(PCINT0_vect)
{
    uint8_t estadoPINB = PINB;
    if(isBitSet(estadoPINB, 0))
        toggleAnimDir();

    if(isBitSet(estadoPINB, 1))
        toggleAnimFreeze();
}

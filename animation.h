#pragma once

#define FREEZE 0b0001
#define RIGHT 0b0010

volatile unsigned char anim_settings = 0b0000;
volatile uint8_t anim = 0; // PRIMEIRO FRAME DA ANIMAÇÃO

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

inline void animate()
{
    PORTD = anim;
    uint8_t settings;

    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
        settings = anim_settings;
    }

    if ((settings & FREEZE) == false)
    {
        if (settings & RIGHT)
        {
            anim >>= 1;

            if (anim == 0)
                anim = 0b10000000;
        }
        else
        {
            anim <<= 1;
            if (anim == 0)
                anim = 1;
        }
    }
}

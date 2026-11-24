#pragma once

#include "../funsape/peripheral/gpioPin.hpp"
#include <stdint.h>
#include <util/atomic.h>

class Encoder
{
public:
    enum class KeyEvent : uint16_t
    {
        NONE,
        PRESS,
        RELEASE,
    };

    enum class RotEvent : uint16_t
    {
        NONE,
        CLOCKWISE,
        COUNTERCLOCK,
    };

    struct InputAction
    {
        KeyEvent keyAction;
        RotEvent rotAction;
    };

    struct InputState
    {
        bool s1 = false;
        bool s2 = false;
        bool key = false;
    };

    Encoder(const GpioPin &s1, const GpioPin &s2, const GpioPin &key, uint16_t debounce = 50) : m_s1(s1),
                                                                                                m_s2(s2),
                                                                                                m_key(key),
                                                                                                m_debounce(debounce ? debounce : 1)
    {
    }

    Encoder(const Encoder &) = delete;
    Encoder &operator=(const Encoder &) = delete;

    void init()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            m_inputAct.keyAction = KeyEvent::NONE;
            m_inputAct.rotAction = RotEvent::NONE;

            m_prevInputState.s1 = m_s1.read();
            m_prevInputState.s2 = m_s2.read();
            m_prevInputState.key = !m_key.read();

            m_debounceCounter = 0;
            m_ready = true;
        }
    }

    void update()
    {
        if (!m_ready)
            return;

        InputState inputState;

        inputState.s1 = m_s1.read();
        inputState.s2 = m_s2.read();
        inputState.key = !m_key.read();

        // Rotação na borda de descida de S1
        if (m_prevInputState.s1 && !inputState.s1)
            m_inputAct.rotAction = inputState.s2 ? RotEvent::CLOCKWISE : RotEvent::COUNTERCLOCK;

        m_prevInputState.s1 = inputState.s1;
        m_prevInputState.s2 = inputState.s2;

        if (inputState.key == m_prevInputState.key) // Debounce
        {
            m_debounceCounter = 0;
        }
        else
        {
            if (++m_debounceCounter >= m_debounce)
            {
                m_prevInputState.key = inputState.key;
                m_debounceCounter = 0;

                m_inputAct.keyAction = inputState.key ? KeyEvent::PRESS : KeyEvent::RELEASE;
            }
        }
    }

    InputAction getInputAction()
    {
        InputAction act;

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            act.keyAction = m_inputAct.keyAction;
            act.rotAction = m_inputAct.rotAction;

            m_inputAct.keyAction = KeyEvent::NONE;
            m_inputAct.rotAction = RotEvent::NONE;
        }

        return act;
    }

    InputState getInputState() const
    {
        InputState state;

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            state.s1 = m_prevInputState.s1;
            state.s2 = m_prevInputState.s2;
            state.key = m_prevInputState.key;
        }

        return state;
    }

private:
    const GpioPin &m_s1;
    const GpioPin &m_s2;
    const GpioPin &m_key;

    const uint16_t m_debounce;

    volatile InputState m_prevInputState;
    volatile InputAction m_inputAct;

    uint16_t m_debounceCounter = 0;

    volatile bool m_ready = false;
};
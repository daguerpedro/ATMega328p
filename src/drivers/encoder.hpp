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

    Encoder() = default;
    Encoder(const Encoder &) = delete;
    Encoder &operator=(const Encoder &) = delete;

    void init(ioRegAddress_t gpiPortAddr, const PinIndex S1PinIndex, const PinIndex S2PinIndex, const PinIndex KeyPinIndex, bool pullUp = true)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            m_s1.init(gpiPortAddr, S1PinIndex);
            m_s2.init(gpiPortAddr, S2PinIndex);
            m_key.init(gpiPortAddr, KeyPinIndex);

            m_s1.setMode(pullUp ? PinMode::INPUT_PULLED_UP : PinMode::INPUT_FLOATING);
            m_s2.setMode(pullUp ? PinMode::INPUT_PULLED_UP : PinMode::INPUT_FLOATING);
            m_key.setMode(pullUp ? PinMode::INPUT_PULLED_UP : PinMode::INPUT_FLOATING);

            m_inputAct.keyAction = KeyEvent::NONE;
            m_inputAct.rotAction = RotEvent::NONE;

            m_prevInputState.s1 = m_s1.read();
            m_prevInputState.s2 = m_s2.read();
            m_prevInputState.key = !m_key.read();

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

        if (inputState.key != m_prevInputState.key) // Debounce
        {
            m_prevInputState.key = inputState.key;
            m_inputAct.keyAction = inputState.key ? KeyEvent::PRESS : KeyEvent::RELEASE;
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
    GpioPin m_s1;
    GpioPin m_s2;
    GpioPin m_key;

    volatile InputState m_prevInputState;
    volatile InputAction m_inputAct;

    volatile bool m_ready = false;
};
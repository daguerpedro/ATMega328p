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
        bool clicked = false;
    };

    struct InputState
    {
        bool s1 = false;
        bool s2 = false;
        bool key = false;
    };

    enum class Error : uint8_t
    {
        NONE,
        NOT_INITIALIZED,
        INVALID_PIN,
        GPIO_INIT_FAILED,
        GPIO_MODE_FAILED,
    };

    enum class Input : uint8_t
    {
        NONE,
        S1,
        S2,
        KEY,
    };

    struct ErrorInfo
    {
        Error code;
        Input input;
    };

    Encoder() = default;
    Encoder(const Encoder &) = delete;
    Encoder &operator=(const Encoder &) = delete;

    bool init(ioRegAddress_t port, PinIndex s1, PinIndex s2, PinIndex key, bool pullUp = true)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            m_ready = false;
            m_keySamples = 0;
            m_inputAct.keyAction = KeyEvent::NONE;
            m_inputAct.rotAction = RotEvent::NONE;
            m_inputAct.clicked = false;
            m_prevInputState.s1 = false;
            m_prevInputState.s2 = false;
            m_prevInputState.key = false;

            if (static_cast<uint8_t>(s1) >= 8)
                return fail(Error::INVALID_PIN, Input::S1);

            if (static_cast<uint8_t>(s2) >= 8)
                return fail(Error::INVALID_PIN, Input::S2);

            if (static_cast<uint8_t>(key) >= 8)
                return fail(Error::INVALID_PIN, Input::KEY);

            if (!m_s1.init(port, s1))
                return fail(Error::GPIO_INIT_FAILED, Input::S1);

            if (!m_s2.init(port, s2))
                return fail(Error::GPIO_INIT_FAILED, Input::S2);

            if (!m_key.init(port, key))
                return fail(Error::GPIO_INIT_FAILED, Input::KEY);

            const PinMode mode = pullUp ? PinMode::INPUT_PULLED_UP : PinMode::INPUT_FLOATING;

            if (!m_s1.setMode(mode))
                return fail(Error::GPIO_MODE_FAILED, Input::S1);

            if (!m_s2.setMode(mode))
                return fail(Error::GPIO_MODE_FAILED, Input::S2);

            if (!m_key.setMode(mode))
                return fail(Error::GPIO_MODE_FAILED, Input::KEY);

            m_prevInputState.s1 = m_s1.read();
            m_prevInputState.s2 = m_s2.read();
            m_prevInputState.key = !m_key.read();
            m_rawKey = m_prevInputState.key;
            m_pressInvalid = m_prevInputState.key;

            m_lastError = {Error::NONE, Input::NONE};
            m_ready = true;
        }

        return true;
    }

    ErrorInfo getLastErrorInfo() const
    {
        return m_lastError;
    }

    static const char *ErrorString(Error error)
    {
        switch (error)
        {
        case Error::NONE: return "No error";
        case Error::NOT_INITIALIZED: return "Encoder not initialized";
        case Error::INVALID_PIN: return "Invalid encoder pin index";
        case Error::GPIO_INIT_FAILED: return "Encoder GPIO initialization failed";
        case Error::GPIO_MODE_FAILED: return "Encoder GPIO mode configuration failed";
        }

        return "Unknown encoder error";
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

        const bool rotated = inputState.s1 != m_prevInputState.s1 || inputState.s2 != m_prevInputState.s2;
        if (rotated && (inputState.key || m_prevInputState.key))
            m_pressInvalid = true;

        m_prevInputState.s1 = inputState.s1;
        m_prevInputState.s2 = inputState.s2;

        if (inputState.key != m_rawKey)
        {
            m_rawKey = inputState.key;
            m_keySamples = 0;
        }
    }

    void sampleKey()
    {
        if (!m_ready)
            return;

        const bool pressed = !m_key.read();
        if (pressed != m_rawKey)
        {
            m_rawKey = pressed;
            m_keySamples = 0;
        }

        const bool rearming = !pressed && !m_prevInputState.key && m_pressInvalid;
        if (pressed == m_prevInputState.key && !rearming) // Debounce
        {
            m_keySamples = 0;
            return;
        }

        if (++m_keySamples == KEY_DEBOUNCE_SAMPLES)
        {
            m_keySamples = 0;
            if (rearming)
            {
                m_pressInvalid = false;
                return;
            }

            m_prevInputState.key = pressed;
            m_inputAct.keyAction = pressed ? KeyEvent::PRESS : KeyEvent::RELEASE;
            if (!pressed)
            {
                if (!m_pressInvalid)
                    m_inputAct.clicked = true;
                m_pressInvalid = false;
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
            act.clicked = m_inputAct.clicked;

            m_inputAct.keyAction = KeyEvent::NONE;
            m_inputAct.rotAction = RotEvent::NONE;
            m_inputAct.clicked = false;
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
    static constexpr uint8_t KEY_DEBOUNCE_SAMPLES = 21;

    bool fail(Error code, Input input)
    {
        m_lastError = {code, input};
        return false;
    }

    GpioPin m_s1;
    GpioPin m_s2;
    GpioPin m_key;

    volatile InputState m_prevInputState;
    volatile InputAction m_inputAct{KeyEvent::NONE, RotEvent::NONE};

    ErrorInfo m_lastError{Error::NOT_INITIALIZED, Input::NONE};

    volatile uint8_t m_keySamples = 0;
    volatile bool m_rawKey = false;
    volatile bool m_pressInvalid = false;
    volatile bool m_ready = false;
};

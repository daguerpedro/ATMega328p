#pragma once

#include "funsape/globalDefines.hpp"
#include "funsape/peripheral/gpioBus.hpp"
#include "funsape/device/sevenSegmentsMuxDisplay.hpp"

class Counter
{
private:
    GpioBus m_displayDataBus;
    GpioBus m_displatCtrlBus;

    SevenSegmentsMuxDisplay m_display;

    volatile uint16_t m_counter; //Volátil pois valor é atualizado em uma interrupção

    uint8_t m_digits[4] = {0, 0, 0, 0}; //Buffer que vai para o display, em ordem inversa

public:
    Counter(const uint16_t &initialValue) : m_counter(initialValue) {};
    ~Counter() = default;

    void init(ioRegAddress_t dataBusAddr, PinIndex dataPinIndex, ioRegAddress_t controlBusAddr, PinIndex controlPinIndex)
    {
        m_displayDataBus.init(dataBusAddr, dataPinIndex, 8);
        m_displatCtrlBus.init(controlBusAddr, controlPinIndex, 4);

        m_display.setPorts(&m_displayDataBus, &m_displatCtrlBus, LogicLevel::HIGH);
        m_display.init(SevenSegmentsDisplayType::COMMON_CATHODE);

        update();
    }

    void update()
    {
        m_digits[0] = (m_counter / 1) % 10;    //Pega o 1 digito de m_counter
        m_digits[1] = (m_counter / 10) % 10;   //Pega o 2 digito de m_counter
        m_digits[2] = (m_counter / 100) % 10;  //Pega o 3 digito de m_counter
        m_digits[3] = (m_counter / 1000) % 10; //Pega o 4 digito de m_counter

        m_display.updateDigitValues(m_digits, 0); //Empurra pro display
    };

    void display()
    {
        m_display.showNextDigit();  //Mostra o próximo digito do display
                                    //Cada dígito deve aparecer ao meno 60 vezes por seg.
                                    //Para um efeito visual contínuo
    };

    void increment(const uint16_t &amount)
    {
        m_counter += amount;    //Incrementa
        m_counter %= 10000;     //Tira o estouro
        
        update();
    };
};
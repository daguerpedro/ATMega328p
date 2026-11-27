#pragma once

#include "drivers/ds1302.hpp"
#include "drivers/encoder.hpp"
#include "drivers/dht11.hpp"

class System
{
public:
    static System &getInstance()
    {
        static System sys;
        return sys;
    };

    System(const System &) = delete;
    System &operator=(const System &) = delete;

    void startDevices();
    void startPeripherals();
    void drawScreen();

    inline void requestSample() { m_fRequestSample = true; };
    inline void requestEncoder() { encoder.update(); };

private:
    void sampleRoutine();

    System() = default;
    ~System() = default;

    DHT11::Data dhtBuffer;
    char displayBuffer[128];

    Ds1302 rtc;
    DHT11 dht11;
    Encoder encoder;

    volatile bool m_fRequestSample = false;

    enum SCREEN : uint8_t
    {
        SENSORS,
        CONFIG
    };

    SCREEN activeScreen;
    SCREEN screen;
};
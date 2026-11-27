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

    void checkFlags();

    inline void requestRTC() { m_fRequestRTC = true; }
    inline void requestSample() { m_fRequestSample = true; };
    inline void requestEncoder() { encoder.update(); };

private:
    void sampleRoutine();
    void inputRoutine();
    void rtcRoutine();
    
    System() : rtc(), dht11(), encoder() {};
    ~System() = default;

    struct RTCData
    {
        uint8_t hora;
        uint8_t min;
        uint8_t sec;
    };

    RTCData rtcBuffer{};
    DHT11::Data dhtBuffer{};
    char displayBuffer[64]{};

    Ds1302 rtc;
    DHT11 dht11;
    Encoder encoder;

    volatile bool m_fRequestRTC = false;
    volatile bool m_fRequestSample = false;
    volatile bool m_updateScreen = false;

    volatile bool sampleRequested = false;
    volatile bool rtcRequested = false;

    enum SCREEN : uint8_t
    {
        SENSORS,
        TIME,
        CONFIG
    };

    SCREEN activeScreen = SCREEN::SENSORS;
    SCREEN targetScreen = SCREEN::SENSORS;
};
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
    void run();

    inline void requestRTC() { m_fRequestRTC = true; }
    inline void requestSample() { m_fRequestSample = true; };
    inline void requestEncoder() { encoder.update(); };
    inline void sampleEncoderKey() { encoder.sampleKey(); }
private:
    void sampleRoutine();
    void inputRoutine();
    void rtcRoutine();
    void drawScreen();
    void drawSensors();
    void drawConfig();
    void checkFlags();

    System() : rtc(), dht11(), encoder() {};
    ~System() = default;

    struct RTCData
    {
        uint8_t hora;
        uint8_t min;
        uint8_t sec;
    };

    bool encoderReady = false;
    bool rtcRunning = false;
    bool rtcTimeValid = false;

    DHT11::Result dhtInitResult = DHT11::Result::NotInitialized;
    DHT11::Result dhtSampleResult = DHT11::Result::NotInitialized;

    RTCData rtcBuffer{};
    DHT11::Data dhtBuffer{};
    char displayBuffer[64]{};

    Ds1302 rtc;
    DHT11 dht11;
    Encoder encoder;

    volatile bool m_fRequestRTC = false;
    volatile bool m_fRequestSample = false;
    bool m_updateScreen = false;

    bool sampleRequested = false;
    bool rtcRequested = false;

    enum SCREEN : uint8_t
    {
        SENSORS,
        CONFIG
    };

    SCREEN activeScreen = SCREEN::SENSORS;

    bool configUnitSelected = true;
    bool fahrenheit = false;
};

//!
//! \file           ds1307.cpp
//! \brief          DS1307 module interface for the FunSAPE AVR8 Library
//! \author         Leandro Schwarz (bladabuska+funsapeavr8lib@gmail.com)
//! \date           2024-07-10
//! \version        24.07
//! \copyright      license
//! \details        DS1307 RTC module interface
//! \todo           Todo list
//!

// =============================================================================
// System file dependencies
// =============================================================================

#include "ds1307.hpp"
#if !defined(__DS1307_HPP)
#    error "Header file is corrupted!"
#elif __DS1307_HPP != 2604
#    error "Version mismatch between source and header files!"
#endif

// =============================================================================
// File exclusive - Constants
// =============================================================================

cuint8_t constDs1307DeviceAddress       = 0x68;
cuint8_t constDs1307RamSize             = 56;

// =============================================================================
// File exclusive - New data types
// =============================================================================

// NONE

// =============================================================================
// File exclusive - Global variables
// =============================================================================

// NONE

// =============================================================================
// File exclusive - Macro-functions
// =============================================================================

// NONE

// =============================================================================
// Class constructors
// =============================================================================

Ds1307::Ds1307(void)
{
    // Mark passage for debugging purpose
    debugMark(PSTR("Ds1307::Ds1307(void)"), Debug::CodeIndex::DS1307_MODULE);

    // Resets data members
    this->_clearData();

    // Returns successfully
    this->_lastError = Error::NONE;
    debugMessage(Error::NONE, Debug::CodeIndex::DS1307_MODULE);
    return;
}

Ds1307::~Ds1307(void)
{
    // Mark passage for debugging purpose
    debugMark(PSTR("Ds1307::~Ds1307(void)"), Debug::CodeIndex::DS1307_MODULE);

    // Returns successfully
    debugMessage(Error::NONE, Debug::CodeIndex::DS1307_MODULE);
    return;
}

// =============================================================================
// Class public methods
// =============================================================================

//     ///////////////////     CONTROL AND STATUS     ///////////////////     //

bool_t Ds1307::clockStart(void)
{
    // Mark passage for debugging purpose
    debugMark(PSTR("Ds1307::clockStart(void)"), Debug::CodeIndex::DS1307_MODULE);

    // Set counting status
    if(!this->_setCounting(true)) {
        // Returns error
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Returns successfully
    this->_lastError = Error::NONE;
    debugMessage(Error::NONE, Debug::CodeIndex::DS1307_MODULE);
    return true;
}

bool_t Ds1307::clockStop(void)
{
    // Mark passage for debugging purpose
    debugMark(PSTR("Ds1307::clockStop(void)"), Debug::CodeIndex::DS1307_MODULE);

    // Set counting status
    if(!this->_setCounting(false)) {
        // Returns error
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Returns successfully
    this->_lastError = Error::NONE;
    debugMessage(Error::NONE, Debug::CodeIndex::DS1307_MODULE);
    return true;
}

Error Ds1307::getLastError(void)
{
    // Returns last error
    return this->_lastError;
}

bool_t Ds1307::init(Bus *busHandler_p)
{
    // Mark passage for debugging purpose
    debugMark(PSTR("Ds1307::init(Bus *)"), Debug::CodeIndex::DS1307_MODULE);

    // Local variables
    uint8_t auxBuffer;

    // Resets data members
    this->_clearData();

    // Check function arguments for errors
    if(!isPointerValid(busHandler_p)) {
        // Returns error
        this->_lastError = Error::BUS_HANDLER_POINTER_NULL;
        debugMessage(Error::BUS_HANDLER_POINTER_NULL, Debug::CodeIndex::DS1307_MODULE);
        return false;
    } else if(busHandler_p->getBusType() != Bus::BusType::TWI) {
        // Returns error
        this->_lastError = Error::BUS_HANDLER_NOT_SUPPORTED;
        debugMessage(Error::BUS_HANDLER_NOT_SUPPORTED, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Checks if DS1307 is present and retrieves counting status
    if(!busHandler_p->setDevice(constDs1307DeviceAddress)) {
        // Returns error
        this->_lastError = busHandler_p->getLastError();
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    if(!busHandler_p->readReg((uint8_t)(Register::SECONDS), &auxBuffer, 1)) {
        // Returns error
        this->_lastError = busHandler_p->getLastError();
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Update data members
    this->_busHandler = busHandler_p;
    this->_countingHalted = isBitSet(auxBuffer, 7);

    // Mark device as initialized
    this->_initialized = true;

    // Returns successfully
    this->_lastError = Error::NONE;
    debugMessage(Error::NONE, Debug::CodeIndex::DS1307_MODULE);
    return true;
}

bool_t Ds1307::setSquareWaveGenerator(const SquareWave squareWave_p)
{
    // Mark passage for debugging purpose
    debugMark(PSTR("Ds1307::setSquareWaveGenerator(const SquareWave)"), Debug::CodeIndex::DS1307_MODULE);

    // Local variables
    bool_t _squareWaveActivation;
    uint8_t _squareWaveFrequency;
    bool_t _squareWaveOffValue;
    uint8_t auxBuffer;

    // Checks initialization
    if(!this->_isInitialized()) {
        // Returns error
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Process arguments
    switch(squareWave_p) {
    case SquareWave::OFF_LOW:
        _squareWaveActivation = false;
        _squareWaveFrequency = 0;
        _squareWaveOffValue = false;
        break;
    case SquareWave::OFF_HIGH:
        _squareWaveActivation = false;
        _squareWaveFrequency = 0;
        _squareWaveOffValue = true;
        break;
    case SquareWave::CLOCK_1_HZ:
        _squareWaveActivation = true;
        _squareWaveFrequency = 0;
        _squareWaveOffValue = true;
        break;
    case SquareWave::CLOCK_4_KHZ:
        _squareWaveActivation = true;
        _squareWaveFrequency = 1;
        _squareWaveOffValue = true;
        break;
    case SquareWave::CLOCK_8_KHZ:
        _squareWaveActivation = true;
        _squareWaveFrequency = 2;
        _squareWaveOffValue = true;
        break;
    case SquareWave::CLOCK_32_KHZ:
        _squareWaveActivation = true;
        _squareWaveFrequency = 3;
        _squareWaveOffValue = true;
        break;
    default:
        // Returns error
        this->_lastError = Error::ARGUMENT_VALUE_INVALID;
        debugMessage(Error::ARGUMENT_VALUE_INVALID, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    auxBuffer =
            (_squareWaveOffValue << 7) |
            (_squareWaveActivation << 4) |
            (_squareWaveFrequency << 0);

    // Sends data to device
    if(!this->_busHandler->setDevice(constDs1307DeviceAddress)) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    if(!this->_busHandler->writeReg((uint8_t)(Register::CONTROL), &auxBuffer, 1)) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Update data members
    this->_squareWave = squareWave_p;

    // Returns successfully
    this->_lastError = Error::NONE;
    debugMessage(Error::NONE, Debug::CodeIndex::DS1307_MODULE);
    return true;
}

//     /////////////////    DATE HANDLING FUNCTIONS     /////////////////     //

bool_t Ds1307::getDate(uint16_t *year_p, uint8_t *month_p, uint8_t *monthDay_p, uint8_t *weekDay_p)
{
    // Mark passage for debugging purpose
    debugMark(PSTR("Ds1307::getDate(uint16_t *, uint8_t *, uint8_t *, uint8_t *)"), Debug::CodeIndex::DS1307_MODULE);

    // Local variables
    uint8_t auxBuffer[4];

    // Checks initialization
    if(!this->_isInitialized()) {
        // Returns error
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // CHECK FOR ERROR - invalid pointers
    if((!isPointerValid(year_p)) || (!isPointerValid(month_p)) || (!isPointerValid(monthDay_p))) {
        // Returns error
        this->_lastError = Error::ARGUMENT_POINTER_NULL;
        debugMessage(Error::ARGUMENT_POINTER_NULL, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Reads data from device
    if(!this->_busHandler->setDevice(constDs1307DeviceAddress)) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    if(!this->_busHandler->readReg((uint8_t)(Register::DAY), auxBuffer, 4)) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Decodes the information
    if(!this->_decodeDate(auxBuffer, year_p, month_p, monthDay_p, weekDay_p)) {
        // Returns error
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Returns successfully
    this->_lastError = Error::NONE;
    debugMessage(Error::NONE, Debug::CodeIndex::DS1307_MODULE);
    return true;
}

bool_t Ds1307::setDate(uint16_t year_p, uint8_t month_p, uint8_t monthDay_p)
{
    // Mark passage for debugging purpose
    debugMark(PSTR("Ds1307::setDate(uint16_t, uint8_t, uint8_t)"), Debug::CodeIndex::DS1307_MODULE);

    // Local variables
    uint8_t auxBuffer[4];

    // Checks initialization
    if(!this->_isInitialized()) {
        // Returns error
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Encode the information
    if(!this->_encodeDate(auxBuffer, year_p, month_p, monthDay_p)) {
        // Returns error
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Sends data to device
    if(!this->_busHandler->setDevice(constDs1307DeviceAddress)) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    if(!this->_busHandler->writeReg((uint8_t)(Register::DAY), auxBuffer, 4)) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Returns successfully
    this->_lastError = Error::NONE;
    debugMessage(Error::NONE, Debug::CodeIndex::DS1307_MODULE);
    return true;
}

//     /////////////////    TIME HANDLING FUNCTIONS     /////////////////     //

bool_t Ds1307::getTime(uint8_t *hours_p, uint8_t *minutes_p, uint8_t *seconds_p, TimeFormat timeFormat_p,
        AmPmFlag *amPmFlag_p)
{
    // Mark passage for debugging purpose
    debugMark(PSTR("Ds1307::getTime(uint8_t *, uint8_t *, uint8_t *, TimeFormat, AmPmFlag *)"),
            Debug::CodeIndex::DS1307_MODULE);

    // Local variables
    uint8_t auxBuffer[3];

    // Checks initialization
    if(!this->_isInitialized()) {
        // Returns error
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // CHECK FOR ERROR - invalid pointers
    if((!isPointerValid(hours_p)) || (!isPointerValid(minutes_p)) || (!isPointerValid(seconds_p))) {
        // Returns error
        this->_lastError = Error::ARGUMENT_POINTER_NULL;
        debugMessage(Error::ARGUMENT_POINTER_NULL, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    if((timeFormat_p == TimeFormat::FORMAT_12_HOURS) && (!isPointerValid(amPmFlag_p))) {
        // Returns error
        this->_lastError = Error::ARGUMENT_POINTER_NULL;
        debugMessage(Error::ARGUMENT_POINTER_NULL, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Reads data from device
    if(!this->_busHandler->setDevice(constDs1307DeviceAddress)) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    if(!this->_busHandler->readReg((uint8_t)(Register::SECONDS), auxBuffer, 3)) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Update counting status
    this->_countingHalted = isBitSet(auxBuffer[0], 7);

    // Decodes the information
    if(!this->_decodeTime(auxBuffer, hours_p, minutes_p, seconds_p, timeFormat_p, amPmFlag_p)) {
        // Returns error
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Returns successfully
    this->_lastError = Error::NONE;
    debugMessage(Error::NONE, Debug::CodeIndex::DS1307_MODULE);
    return true;
}

bool_t Ds1307::setTime(uint8_t hours_p, uint8_t minutes_p, uint8_t seconds_p, TimeFormat timeFormat_p,
        AmPmFlag amPmFlag_p)
{
    // Mark passage for debugging purpose
    debugMark(PSTR("Ds1307::setTime(uint8_t, uint8_t, uint8_t, TimeFormat, AmPmFlag)"),
            Debug::CodeIndex::DS1307_MODULE);

    // Local variables
    uint8_t auxBuffer[3];

    // Checks initialization
    if(!this->_isInitialized()) {
        // Returns error
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Encode the information
    if(!this->_encodeTime(auxBuffer, hours_p, minutes_p, seconds_p, timeFormat_p, amPmFlag_p)) {
        // Returns error
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Sends data to device
    if(!this->_busHandler->setDevice(constDs1307DeviceAddress)) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    if(!this->_busHandler->writeReg((uint8_t)(Register::SECONDS), auxBuffer, 3)) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Returns successfully
    this->_lastError = Error::NONE;
    debugMessage(Error::NONE, Debug::CodeIndex::DS1307_MODULE);
    return true;
}

//     /////////////////    DATA HANDLING FUNCTIONS     /////////////////     //

bool_t Ds1307::getData(uint16_t *year_p, uint8_t *month_p, uint8_t *monthDay_p, uint8_t *weekDay_p,
        uint8_t *hours_p, uint8_t *minutes_p, uint8_t *seconds_p, TimeFormat timeFormat_p,
        AmPmFlag *amPmFlag_p)
{
    // Mark passage for debugging purpose
    debugMark(PSTR("Ds1307::getData(uint16_t *, uint8_t *, uint8_t *, uint8_t *, uint8_t *, uint8_t *, uint8_t *, TimeFormat, AmPmFlag *)"),
            Debug::CodeIndex::DS1307_MODULE);

    // Local variables
    uint8_t auxBuffer[7];
    uint16_t auxYear;
    uint8_t auxMonth;
    uint8_t auxMonthDay;
    uint8_t auxWeekDay;
    uint8_t auxHours;
    uint8_t auxMinutes;
    uint8_t auxSeconds;
    AmPmFlag auxAmPmFlag;

    // Checks initialization
    if(!this->_isInitialized()) {
        // Returns error
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // CHECK FOR ERROR - invalid pointers
    if((!isPointerValid(year_p)) || (!isPointerValid(month_p)) || (!isPointerValid(monthDay_p)) ||
            (!isPointerValid(hours_p)) || (!isPointerValid(minutes_p)) || (!isPointerValid(seconds_p))) {
        // Returns error
        this->_lastError = Error::ARGUMENT_POINTER_NULL;
        debugMessage(Error::ARGUMENT_POINTER_NULL, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    if((timeFormat_p == TimeFormat::FORMAT_12_HOURS) && (!isPointerValid(amPmFlag_p))) {
        // Returns error
        this->_lastError = Error::ARGUMENT_POINTER_NULL;
        debugMessage(Error::ARGUMENT_POINTER_NULL, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Reads data from device
    if(!this->_busHandler->setDevice(constDs1307DeviceAddress)) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    if(!this->_busHandler->readReg((uint8_t)(Register::SECONDS), auxBuffer, 7)) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Update counting status
    this->_countingHalted = isBitSet(auxBuffer[0], 7);

    // Decodes the information
    if(!this->_decodeDate(auxBuffer + 3, &auxYear, &auxMonth, &auxMonthDay, &auxWeekDay)) {
        // Returns error
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    if(!this->_decodeTime(auxBuffer, &auxHours, &auxMinutes, &auxSeconds, timeFormat_p, &auxAmPmFlag)) {
        // Returns error
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Retrieve data
    *year_p = auxYear;
    *month_p = auxMonth;
    *monthDay_p = auxMonthDay;
    *hours_p = auxHours;
    *minutes_p = auxMinutes;
    *seconds_p = auxSeconds;
    if(isPointerValid(weekDay_p)) {
        *weekDay_p = auxWeekDay;
    }
    if(isPointerValid(amPmFlag_p)) {
        *amPmFlag_p = auxAmPmFlag;
    }

    // Returns successfully
    this->_lastError = Error::NONE;
    debugMessage(Error::NONE, Debug::CodeIndex::DS1307_MODULE);
    return true;
}

bool_t Ds1307::setData(uint16_t year_p, uint8_t month_p, uint8_t monthDay_p,
        uint8_t hours_p, uint8_t minutes_p, uint8_t seconds_p, TimeFormat timeFormat_p, AmPmFlag amPmFlag_p)
{
    // Mark passage for debugging purpose
    debugMark(PSTR("Ds1307::setData(uint16_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, TimeFormat, AmPmFlag)"),
            Debug::CodeIndex::DS1307_MODULE);

    // Local variables
    uint8_t auxBuffer[7];

    // Checks initialization
    if(!this->_isInitialized()) {
        // Returns error
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Encode the information
    if(!this->_encodeDate(auxBuffer + 3, year_p, month_p, monthDay_p)) {
        // Returns error
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    if(!this->_encodeTime(auxBuffer, hours_p, minutes_p, seconds_p, timeFormat_p, amPmFlag_p)) {
        // Returns error
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Sends data to device
    if(!this->_busHandler->setDevice(constDs1307DeviceAddress)) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    if(!this->_busHandler->writeReg((uint8_t)(Register::SECONDS), auxBuffer, 7)) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Returns successfully
    this->_lastError = Error::NONE;
    debugMessage(Error::NONE, Debug::CodeIndex::DS1307_MODULE);
    return true;
}

//     ///////////////    RAM DATA HANDLING FUNCTIONS     ///////////////     //

bool_t Ds1307::getRamData(uint8_t position_p, uint8_t *buffer_p, uint8_t size_p)
{
    // Mark passage for debugging purpose
    debugMark(PSTR("Ds1307::getRamData(uint8_t, uint8_t *, uint8_t)"), Debug::CodeIndex::DS1307_MODULE);

    // Checks initialization
    if(!this->_isInitialized()) {
        // Returns error
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // CHECK FOR ERROR - invalid pointer
    if(!isPointerValid(buffer_p)) {
        // Returns error
        this->_lastError = Error::ARGUMENT_POINTER_NULL;
        debugMessage(Error::ARGUMENT_POINTER_NULL, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    // CHECK FOR ERROR - wrong number of bytes to receive
    if(size_p == 0) {
        // Returns error
        this->_lastError = Error::ARGUMENT_CANNOT_BE_ZERO;
        debugMessage(Error::ARGUMENT_CANNOT_BE_ZERO, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    if(size_p > constDs1307RamSize) {
        // Returns error
        this->_lastError = Error::BUFFER_SIZE_TOO_LARGE;
        debugMessage(Error::BUFFER_SIZE_TOO_LARGE, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    // CHECK FOR ERROR - position invalid
    if(position_p > (constDs1307RamSize - 1)) {
        // Returns error
        this->_lastError = Error::ARGUMENT_VALUE_INVALID;
        debugMessage(Error::ARGUMENT_VALUE_INVALID, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    // CHECK FOR ERROR - memory overlap
    if((position_p + size_p) > constDs1307RamSize) {
        // Returns error
        this->_lastError = Error::BUFFER_SIZE_TOO_LARGE;
        debugMessage(Error::BUFFER_SIZE_TOO_LARGE, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Read RAM data
    if(!this->_busHandler->setDevice(constDs1307DeviceAddress)) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    position_p += (uint8_t)(Register::RAM_START);
    if(!this->_busHandler->readReg(position_p, buffer_p, size_p)) {
        // Returns error
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Returns successfully
    this->_lastError = Error::NONE;
    debugMessage(Error::NONE, Debug::CodeIndex::DS1307_MODULE);
    return true;
}

bool_t Ds1307::setRamData(uint8_t position_p, uint8_t *buffer_p, uint8_t size_p)
{
    // Mark passage for debugging purpose
    debugMark(PSTR("Ds1307::setRamData(uint8_t, uint8_t *, uint8_t)"), Debug::CodeIndex::DS1307_MODULE);

    // Checks initialization
    if(!this->_isInitialized()) {
        // Returns error
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // CHECK FOR ERROR - invalid pointer
    if(!isPointerValid(buffer_p)) {
        // Returns error
        this->_lastError = Error::ARGUMENT_POINTER_NULL;
        debugMessage(Error::ARGUMENT_POINTER_NULL, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    // CHECK FOR ERROR - wrong number of bytes to receive
    if(size_p == 0) {
        // Returns error
        this->_lastError = Error::ARGUMENT_CANNOT_BE_ZERO;
        debugMessage(Error::ARGUMENT_CANNOT_BE_ZERO, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    if(size_p > constDs1307RamSize) {
        // Returns error
        this->_lastError = Error::BUFFER_SIZE_TOO_LARGE;
        debugMessage(Error::BUFFER_SIZE_TOO_LARGE, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    // CHECK FOR ERROR - position invalid
    if(position_p > (constDs1307RamSize - 1)) {
        // Returns error
        this->_lastError = Error::ARGUMENT_VALUE_INVALID;
        debugMessage(Error::ARGUMENT_VALUE_INVALID, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    // CHECK FOR ERROR - memory overlap
    if((position_p + size_p) > constDs1307RamSize) {
        // Returns error
        this->_lastError = Error::BUFFER_SIZE_TOO_LARGE;
        debugMessage(Error::BUFFER_SIZE_TOO_LARGE, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Write RAM data
    if(!this->_busHandler->setDevice(constDs1307DeviceAddress)) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    position_p += (uint8_t)(Register::RAM_START);
    if(!this->_busHandler->writeReg(position_p, buffer_p, size_p)) {
        // Returns error
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Returns successfully
    this->_lastError = Error::NONE;
    debugMessage(Error::NONE, Debug::CodeIndex::DS1307_MODULE);
    return true;
}

// =============================================================================
// Class private methods
// =============================================================================

void Ds1307::_clearData(void)
{
    // Mark passage for debugging purpose
    debugMark(PSTR("Ds1307::_clearData(void)"), Debug::CodeIndex::DS1307_MODULE);

    //     ////////////////    PERIPHERAL BUS HANDLER     ////////////////     //
    this->_busHandler                   = nullptr;
    //     /////////////////     CONTROL AND STATUS     /////////////////     //
    this->_countingHalted               = false;
    this->_initialized                  = false;
    this->_squareWave                   = SquareWave::OFF_LOW;

    // Returns successfully
    this->_lastError = Error::NONE;
    debugMessage(Error::NONE, Debug::CodeIndex::DS1307_MODULE);
    return;
}

bool_t Ds1307::_isInitialized(void)
{
    // Mark passage for debugging purpose
    debugMark(PSTR("Ds1307::_isInitialized(void)"), Debug::CodeIndex::DS1307_MODULE);

    // CHECK FOR ERROR - peripheral not initialized
    if(!this->_initialized) {
        // Returns error
        this->_lastError = Error::NOT_INITIALIZED;
        debugMessage(Error::NOT_INITIALIZED, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Returns successfully
    this->_lastError = Error::NONE;
    debugMessage(Error::NONE, Debug::CodeIndex::DS1307_MODULE);
    return true;
}

bool_t Ds1307::_isValidDate(uint16_t year_p, uint8_t month_p, uint8_t monthDay_p)
{
    // Mark passage for debugging purpose
    debugMark(PSTR("Ds1307::_isValidDate(uint16_t, uint8_t, uint8_t)"), Debug::CodeIndex::DS1307_MODULE);

    // Checks date values
    if((year_p < 2000) || (year_p > 2099) || (month_p < 1) || (month_p > 12) ||
            (monthDay_p < 1) || (monthDay_p > 31)) {
        // Returns error
        this->_lastError = Error::DATE_INVALID;
        debugMessage(Error::DATE_INVALID, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    if(month_p == 2) {
        if(monthDay_p > ((year_p % 4 == 0) ? 29 : 28)) {
            // Returns error
            this->_lastError = Error::DATE_INVALID;
            debugMessage(Error::DATE_INVALID, Debug::CodeIndex::DS1307_MODULE);
            return false;
        }
    } else if((month_p == 4) || (month_p == 6) || (month_p == 9) || (month_p == 11)) {
        if(monthDay_p > 30) {
            // Returns error
            this->_lastError = Error::DATE_INVALID;
            debugMessage(Error::DATE_INVALID, Debug::CodeIndex::DS1307_MODULE);
            return false;
        }
    }

    // Returns successfully
    this->_lastError = Error::NONE;
    debugMessage(Error::NONE, Debug::CodeIndex::DS1307_MODULE);
    return true;
}

bool_t Ds1307::_encodeDate(uint8_t *buffer_p, uint16_t year_p, uint8_t month_p, uint8_t monthDay_p)
{
    // Mark passage for debugging purpose
    debugMark(PSTR("Ds1307::_encodeDate(uint8_t *, uint16_t, uint8_t, uint8_t)"),
            Debug::CodeIndex::DS1307_MODULE);

    // Local variables
    uint16_t auxYear;

    // Checks date values
    if(!this->_isValidDate(year_p, month_p, monthDay_p)) {
        // Returns error
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Week day evaluation
    // For adjusted years 1999-2099, -year / 100 + year / 400 is always -15
    auxYear = year_p - (month_p < 3);
    buffer_p[0] = ((auxYear + auxYear / 4 - 15 + "-bed=pen+mad."[month_p] + monthDay_p) % 7) + 1;

    // Encode the information
    year_p -= 2000;
    buffer_p[1] = ((monthDay_p / 10) << 4) | (monthDay_p % 10);
    buffer_p[2] = ((month_p / 10) << 4) | (month_p % 10);
    buffer_p[3] = ((year_p / 10) << 4) | (year_p % 10);

    // Returns successfully
    this->_lastError = Error::NONE;
    debugMessage(Error::NONE, Debug::CodeIndex::DS1307_MODULE);
    return true;
}

bool_t Ds1307::_decodeDate(const uint8_t *buffer_p, uint16_t *year_p, uint8_t *month_p, uint8_t *monthDay_p,
        uint8_t *weekDay_p)
{
    // Mark passage for debugging purpose
    debugMark(PSTR("Ds1307::_decodeDate(const uint8_t *, uint16_t *, uint8_t *, uint8_t *, uint8_t *)"),
            Debug::CodeIndex::DS1307_MODULE);

    // Local variables
    uint16_t auxYear;
    uint8_t auxMonth;
    uint8_t auxMonthDay;
    uint8_t auxWeekDay;

    // Checks BCD values and reserved bits
    if((buffer_p[0] == 0) || (buffer_p[0] & 0xF8) || (buffer_p[1] & 0xC0) || (buffer_p[2] & 0xE0) ||
            ((buffer_p[1] & 0x0F) > 9) || ((buffer_p[2] & 0x0F) > 9) ||
            ((buffer_p[3] & 0x0F) > 9) || ((buffer_p[3] >> 4) > 9)) {
        // Returns error
        this->_lastError = Error::DATE_INVALID;
        debugMessage(Error::DATE_INVALID, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Decodes the information
    auxWeekDay = buffer_p[0];
    auxMonthDay = ((buffer_p[1] >> 4) * 10) + (buffer_p[1] & 0x0F);
    auxMonth = ((buffer_p[2] >> 4) * 10) + (buffer_p[2] & 0x0F);
    auxYear = ((buffer_p[3] >> 4) * 10) + (buffer_p[3] & 0x0F);
    auxYear += 2000;

    // Checks date values
    if(!this->_isValidDate(auxYear, auxMonth, auxMonthDay)) {
        // Returns error
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Retrieve data
    *year_p = auxYear;
    *month_p = auxMonth;
    *monthDay_p = auxMonthDay;
    if(isPointerValid(weekDay_p)) {
        *weekDay_p = auxWeekDay;
    }

    // Returns successfully
    this->_lastError = Error::NONE;
    debugMessage(Error::NONE, Debug::CodeIndex::DS1307_MODULE);
    return true;
}

bool_t Ds1307::_encodeTime(uint8_t *buffer_p, uint8_t hours_p, uint8_t minutes_p, uint8_t seconds_p,
        TimeFormat timeFormat_p, AmPmFlag amPmFlag_p)
{
    // Mark passage for debugging purpose
    debugMark(PSTR("Ds1307::_encodeTime(uint8_t *, uint8_t, uint8_t, uint8_t, TimeFormat, AmPmFlag)"),
            Debug::CodeIndex::DS1307_MODULE);

    // Checks time values
    if((minutes_p > 59) || (seconds_p > 59) ||
            ((timeFormat_p == TimeFormat::FORMAT_12_HOURS) && ((hours_p == 0) || (hours_p > 12))) ||
            ((timeFormat_p == TimeFormat::FORMAT_24_HOURS) && (hours_p > 23))) {
        // Returns error
        this->_lastError = Error::TIME_INVALID;
        debugMessage(Error::TIME_INVALID, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Encode the information
    buffer_p[0] = ((seconds_p / 10) << 4) | (seconds_p % 10);
    buffer_p[0] |= this->_countingHalted << 7;
    buffer_p[1] = ((minutes_p / 10) << 4) | (minutes_p % 10);
    buffer_p[2] = ((hours_p / 10) << 4) | (hours_p % 10);
    if(timeFormat_p == TimeFormat::FORMAT_12_HOURS) {
        buffer_p[2] |= 0x40;
        buffer_p[2] |= ((amPmFlag_p == AmPmFlag::PM) ? (1 << 5) : 0);
    }

    // Returns successfully
    this->_lastError = Error::NONE;
    debugMessage(Error::NONE, Debug::CodeIndex::DS1307_MODULE);
    return true;
}

bool_t Ds1307::_decodeTime(const uint8_t *buffer_p, uint8_t *hours_p, uint8_t *minutes_p, uint8_t *seconds_p,
        TimeFormat timeFormat_p, AmPmFlag *amPmFlag_p)
{
    // Mark passage for debugging purpose
    debugMark(PSTR("Ds1307::_decodeTime(const uint8_t *, uint8_t *, uint8_t *, uint8_t *, TimeFormat, AmPmFlag *)"),
            Debug::CodeIndex::DS1307_MODULE);

    // Local variables
    uint8_t auxSeconds;
    uint8_t auxMinutes;
    uint8_t auxHours;
    TimeFormat auxTimeFormat;
    AmPmFlag auxAmPmFlag;

    // Checks BCD values and reserved bits
    if((buffer_p[1] & 0x80) || (buffer_p[2] & 0x80) ||
            ((buffer_p[0] & 0x0F) > 9) || ((buffer_p[1] & 0x0F) > 9) ||
            ((buffer_p[2] & 0x0F) > 9)) {
        // Returns error
        this->_lastError = Error::TIME_INVALID;
        debugMessage(Error::TIME_INVALID, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Decodes the information
    auxSeconds = (((buffer_p[0] & 0x70) >> 4) * 10) + (buffer_p[0] & 0x0F);
    auxMinutes = ((buffer_p[1] >> 4) * 10) + (buffer_p[1] & 0x0F);
    if(isBitSet(buffer_p[2], 6)) {  // 12 hour-mode
        auxHours = (((buffer_p[2] & 0x10) >> 4) * 10) + (buffer_p[2] & 0x0F);
        auxTimeFormat = TimeFormat::FORMAT_12_HOURS;
        auxAmPmFlag = (isBitSet(buffer_p[2], 5)) ? AmPmFlag::PM : AmPmFlag::AM;
    } else {                        // 24 hour-mode
        auxHours = (((buffer_p[2] & 0x30) >> 4) * 10) + (buffer_p[2] & 0x0F);
        auxTimeFormat = TimeFormat::FORMAT_24_HOURS;
        auxAmPmFlag = AmPmFlag::AM;
    }

    // Checks time values
    if((auxSeconds > 59) || (auxMinutes > 59) ||
            ((auxTimeFormat == TimeFormat::FORMAT_12_HOURS) && ((auxHours == 0) || (auxHours > 12))) ||
            ((auxTimeFormat == TimeFormat::FORMAT_24_HOURS) && (auxHours > 23))) {
        // Returns error
        this->_lastError = Error::TIME_INVALID;
        debugMessage(Error::TIME_INVALID, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Converts between time formats
    if(auxTimeFormat != timeFormat_p) {
        if(timeFormat_p == TimeFormat::FORMAT_24_HOURS) {
            if(auxAmPmFlag == AmPmFlag::AM) {
                if(auxHours == 12) {
                    auxHours = 0;
                }
            } else {
                if(auxHours != 12) {
                    auxHours += 12;
                }
            }
        } else {
            auxAmPmFlag = (auxHours >= 12) ? AmPmFlag::PM : AmPmFlag::AM;
            if(auxHours == 0) {
                auxHours = 12;
            } else if(auxHours > 12) {
                auxHours -= 12;
            }
        }
    }

    // Retrieve data
    *hours_p = auxHours;
    *minutes_p = auxMinutes;
    *seconds_p = auxSeconds;
    if(isPointerValid(amPmFlag_p)) {
        *amPmFlag_p = auxAmPmFlag;
    }

    // Returns successfully
    this->_lastError = Error::NONE;
    debugMessage(Error::NONE, Debug::CodeIndex::DS1307_MODULE);
    return true;
}

bool_t Ds1307::_setCounting(bool_t counting_p)
{
    // Mark passage for debugging purpose
    debugMark(PSTR("Ds1307::_setCounting(bool_t)"), Debug::CodeIndex::DS1307_MODULE);

    // Local variables
    uint8_t auxBuffer;
    bool isCounting;

    // Checks initialization
    if(!this->_isInitialized()) {
        // Returns error
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }

    // Read register data from device
    if(!this->_busHandler->setDevice(constDs1307DeviceAddress)) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    if(!this->_busHandler->readReg((uint8_t)(Register::SECONDS), &auxBuffer, 1)) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
        return false;
    }
    isCounting = isBitClr(auxBuffer, 7);

    // Update buffer
    if(isCounting != counting_p) {
        if(counting_p) {
            clrBit(auxBuffer, 7);
        } else {
            setBit(auxBuffer, 7);
        }
        // Writes data at device
        if(!this->_busHandler->setDevice(constDs1307DeviceAddress)) {
            // Returns error
            this->_lastError = this->_busHandler->getLastError();
            debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
            return false;
        }
        if(!this->_busHandler->writeReg((uint8_t)(Register::SECONDS), &auxBuffer, 1)) {
            // Returns error
            this->_lastError = this->_busHandler->getLastError();
            debugMessage(this->_lastError, Debug::CodeIndex::DS1307_MODULE);
            return false;
        }
    }

    // Update data members
    this->_countingHalted = !counting_p;

    // Returns successfully
    this->_lastError = Error::NONE;
    debugMessage(Error::NONE, Debug::CodeIndex::DS1307_MODULE);
    return true;
}

// =============================================================================
// Class protected methods
// =============================================================================

// NONE

// =============================================================================
// General public functions definitions
// =============================================================================

// NONE

// =============================================================================
// Interrupt callback functions
// =============================================================================

// NONE

// =============================================================================
// Interrupt handlers
// =============================================================================

// NONE

// =============================================================================
// END OF FILE
// =============================================================================

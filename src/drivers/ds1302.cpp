//!
//! \file           ds1302.cpp
//! \brief          DS1302 driver adapted from the FunSAPE AVR8 DS1307 driver
//! \author         Leandro Schwarz (bladabuska+funsapeavr8lib@gmail.com)
//! \date           2024-07-10
//! \version        24.07
//! \copyright      license
//! \details        DS1302 RTC module interface, adapted from DS1307.
//! \todo           Todo list
//!

// =============================================================================
// System file dependencies
// =============================================================================

#include "ds1302.hpp"
#if !defined(__DS1302_HPP)
#    error "Header file is corrupted!"
#elif __DS1302_HPP != 2604
#    error "Version mismatch between source and header files!"
#endif

// =============================================================================
// File exclusive - Constants
// =============================================================================

cuint8_t constDs1302RamSize             = 31;

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

Ds1302::Ds1302(void)
{

    // Resets data members
    this->_clearData();

    // Returns successfully
    this->_lastError = {Error::NONE, DiagnosticStage::NONE, 0};
    return;
}

Ds1302::~Ds1302(void)
{

    // Returns successfully
    return;
}

// =============================================================================
// Class public methods
// =============================================================================

//     ///////////////////     CONTROL AND STATUS     ///////////////////     //

bool_t Ds1302::clockStart(void)
{

    // Set counting status
    if(!this->_setCounting(true)) {
        // Returns error
        return false;
    }

    // Returns successfully
    this->_lastError = {Error::NONE, DiagnosticStage::NONE, 0};
    return true;
}

bool_t Ds1302::clockStop(void)
{

    // Set counting status
    if(!this->_setCounting(false)) {
        // Returns error
        return false;
    }

    // Returns successfully
    this->_lastError = {Error::NONE, DiagnosticStage::NONE, 0};
    return true;
}

Ds1302::Error Ds1302::getLastError(void)
{
    // Returns last error
    return this->_lastError.code;
}

Ds1302::ErrorInfo Ds1302::getLastErrorInfo() const
{
    return _lastError;
}

const char* Ds1302::ErrorString(Error err)
{
    switch (err)
    {
        case Error::NONE:                    return "Sem Erro";
        case Error::NOT_INITIALIZED:         return "Não Iniciado";
        case Error::ARGUMENT_POINTER_NULL:   return "Ponteiro Nulo";
        case Error::ARGUMENT_VALUE_INVALID:  return "Valor de Argumento Inválido";
        case Error::ARGUMENT_CANNOT_BE_ZERO: return "Argumento Não Pode Ser Zero";
        case Error::BUFFER_SIZE_TOO_LARGE:   return "Tamanho do Buffer Muito Grande";
        case Error::DATE_INVALID:            return "Data Inválida";
        case Error::TIME_INVALID:            return "Hora Inválida";
        case Error::COMMUNICATION_FAILED:    return "Falha de Comunicação";
    }

    return "Erro Desconhecido";
}

bool_t Ds1302::init(ioRegAddress_t port_p, PinIndex ce_p, PinIndex clk_p, PinIndex io_p)
{
    _clearData();
    if (static_cast<uint8_t>(ce_p) >= 8 || static_cast<uint8_t>(clk_p) >= 8 ||
            static_cast<uint8_t>(io_p) >= 8 || ce_p == clk_p || ce_p == io_p || clk_p == io_p ||
            !_ce.init(port_p, ce_p) || !_clk.init(port_p, clk_p) || !_io.init(port_p, io_p)) {
        _lastError = {Error::ARGUMENT_VALUE_INVALID, DiagnosticStage::NONE, 0};
        return false;
    }
    _ce.low();
    _clk.low();
    _ce.setMode(PinMode::OUTPUT_PUSH_PULL);
    _clk.setMode(PinMode::OUTPUT_PUSH_PULL);
    _io.setMode(PinMode::INPUT_FLOATING);
    _delay_ms(10);
    uint8_t wp, seconds;
    _read(0x8E, &wp, 1);
    if (wp & 0x7F) {
        _lastError = {Error::COMMUNICATION_FAILED, DiagnosticStage::INITIAL_CONTROL_READ, wp};
        return false;
    }
    // Verify both logic states of WP; a disconnected I/O may read all zeros.
    uint8_t test = 0x80, check;
    _write(0x8E, &test, 1);
    _read(0x8E, &check, 1);
    _write(0x8E, &wp, 1);
    if (check != 0x80) {
        _lastError = {Error::COMMUNICATION_FAILED, DiagnosticStage::WRITE_PROTECTION_TEST, check};
        return false;
    }
    // Disable trickle charging, including when a previous firmware enabled it.
    uint8_t zero = 0;
    if (!_writeChecked(0x90, &zero, 1)) return false;
    _read(0x80, &seconds, 1);
    _countingHalted = (seconds & 0x80) != 0;
    _initialized = true;
    _lastError = {Error::NONE, DiagnosticStage::NONE, 0};
    return true;
}

//     /////////////////    DATE HANDLING FUNCTIONS     /////////////////     //

bool_t Ds1302::getDate(uint16_t *year_p, uint8_t *month_p, uint8_t *monthDay_p, uint8_t *weekDay_p)
{

    // Local variables
    uint8_t auxBuffer[4];

    // Checks initialization
    if(!this->_isInitialized()) {
        // Returns error
        return false;
    }

    // CHECK FOR ERROR - invalid pointers
    if((!isPointerValid(year_p)) || (!isPointerValid(month_p)) || (!isPointerValid(monthDay_p))) {
        // Returns error
        this->_lastError = {Error::ARGUMENT_POINTER_NULL, DiagnosticStage::NONE, 0};
        return false;
    }

    // Reads data from device
    uint8_t clock[8];
    if (!_readClock(clock)) return false;
    for (uint8_t i = 0; i < 4; ++i) auxBuffer[i] = clock[i+3];

    // Decodes the information
    if(!this->_decodeDate(auxBuffer, year_p, month_p, monthDay_p, weekDay_p)) {
        // Returns error
        return false;
    }

    // Returns successfully
    this->_lastError = {Error::NONE, DiagnosticStage::NONE, 0};
    return true;
}

bool_t Ds1302::setDate(uint16_t year_p, uint8_t month_p, uint8_t monthDay_p)
{

    // Local variables
    uint8_t auxBuffer[4];

    // Checks initialization
    if(!this->_isInitialized()) {
        // Returns error
        return false;
    }

    // Encode the information
    if(!this->_encodeDate(auxBuffer, year_p, month_p, monthDay_p)) {
        // Returns error
        return false;
    }

    // Sends data to device
    if (!_writeChecked(0x86, auxBuffer, 4)) return false;

    // Returns successfully
    this->_lastError = {Error::NONE, DiagnosticStage::NONE, 0};
    return true;
}

//     /////////////////    TIME HANDLING FUNCTIONS     /////////////////     //

bool_t Ds1302::getTime(uint8_t *hours_p, uint8_t *minutes_p, uint8_t *seconds_p, TimeFormat timeFormat_p,
        AmPmFlag *amPmFlag_p)
{

    // Local variables
    uint8_t auxBuffer[3];

    // Checks initialization
    if(!this->_isInitialized()) {
        // Returns error
        return false;
    }

    // CHECK FOR ERROR - invalid pointers
    if((!isPointerValid(hours_p)) || (!isPointerValid(minutes_p)) || (!isPointerValid(seconds_p))) {
        // Returns error
        this->_lastError = {Error::ARGUMENT_POINTER_NULL, DiagnosticStage::NONE, 0};
        return false;
    }
    if((timeFormat_p == TimeFormat::FORMAT_12_HOURS) && (!isPointerValid(amPmFlag_p))) {
        // Returns error
        this->_lastError = {Error::ARGUMENT_POINTER_NULL, DiagnosticStage::NONE, 0};
        return false;
    }

    // Reads data from device
    uint8_t clock[8];
    if (!_readClock(clock)) return false;
    for (uint8_t i = 0; i < 3; ++i) auxBuffer[i] = clock[i];

    // Update counting status
    this->_countingHalted = isBitSet(auxBuffer[0], 7);

    // Decodes the information
    if(!this->_decodeTime(auxBuffer, hours_p, minutes_p, seconds_p, timeFormat_p, amPmFlag_p)) {
        // Returns error
        return false;
    }

    // Returns successfully
    this->_lastError = {Error::NONE, DiagnosticStage::NONE, 0};
    return true;
}

bool_t Ds1302::setTime(uint8_t hours_p, uint8_t minutes_p, uint8_t seconds_p, TimeFormat timeFormat_p,
        AmPmFlag amPmFlag_p)
{

    // Local variables
    uint8_t auxBuffer[3];

    // Checks initialization
    if(!this->_isInitialized()) {
        // Returns error
        return false;
    }

    uint8_t currentSeconds;
    _read(0x80, &currentSeconds, 1);
    _countingHalted = (currentSeconds & 0x80) != 0;
    // Encode the information
    if(!this->_encodeTime(auxBuffer, hours_p, minutes_p, seconds_p, timeFormat_p, amPmFlag_p)) {
        // Returns error
        return false;
    }

    // Sends data to device
    if (!_writeChecked(0x80, auxBuffer, 3)) return false;

    // Returns successfully
    this->_lastError = {Error::NONE, DiagnosticStage::NONE, 0};
    return true;
}

//     /////////////////    DATA HANDLING FUNCTIONS     /////////////////     //

bool_t Ds1302::getData(uint16_t *year_p, uint8_t *month_p, uint8_t *monthDay_p, uint8_t *weekDay_p,
        uint8_t *hours_p, uint8_t *minutes_p, uint8_t *seconds_p, TimeFormat timeFormat_p,
        AmPmFlag *amPmFlag_p)
{

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
        return false;
    }

    // CHECK FOR ERROR - invalid pointers
    if((!isPointerValid(year_p)) || (!isPointerValid(month_p)) || (!isPointerValid(monthDay_p)) ||
            (!isPointerValid(hours_p)) || (!isPointerValid(minutes_p)) || (!isPointerValid(seconds_p))) {
        // Returns error
        this->_lastError = {Error::ARGUMENT_POINTER_NULL, DiagnosticStage::NONE, 0};
        return false;
    }
    if((timeFormat_p == TimeFormat::FORMAT_12_HOURS) && (!isPointerValid(amPmFlag_p))) {
        // Returns error
        this->_lastError = {Error::ARGUMENT_POINTER_NULL, DiagnosticStage::NONE, 0};
        return false;
    }

    // Reads data from device
    uint8_t clock[8];
    if (!_readClock(clock)) return false;
    for (uint8_t i = 0; i < 7; ++i) auxBuffer[i] = clock[i];

    // Update counting status
    this->_countingHalted = isBitSet(auxBuffer[0], 7);

    // Decodes the information
    if(!this->_decodeDate(auxBuffer + 3, &auxYear, &auxMonth, &auxMonthDay, &auxWeekDay)) {
        // Returns error
        return false;
    }
    if(!this->_decodeTime(auxBuffer, &auxHours, &auxMinutes, &auxSeconds, timeFormat_p, &auxAmPmFlag)) {
        // Returns error
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
        *amPmFlag_p = (timeFormat_p == TimeFormat::FORMAT_24_HOURS) ?
                ((auxHours >= 12) ? AmPmFlag::PM : AmPmFlag::AM) : auxAmPmFlag;
    }

    // Returns successfully
    this->_lastError = {Error::NONE, DiagnosticStage::NONE, 0};
    return true;
}

bool_t Ds1302::setData(uint16_t year_p, uint8_t month_p, uint8_t monthDay_p,
        uint8_t hours_p, uint8_t minutes_p, uint8_t seconds_p, TimeFormat timeFormat_p, AmPmFlag amPmFlag_p)
{

    // Local variables
    uint8_t auxBuffer[7];

    // Checks initialization
    if(!this->_isInitialized()) {
        // Returns error
        return false;
    }

    // Encode the information
    if(!this->_encodeDate(auxBuffer + 3, year_p, month_p, monthDay_p)) {
        // Returns error
        return false;
    }
    uint8_t currentSeconds;
    _read(0x80, &currentSeconds, 1);
    _countingHalted = (currentSeconds & 0x80) != 0;
    if(!this->_encodeTime(auxBuffer, hours_p, minutes_p, seconds_p, timeFormat_p, amPmFlag_p)) {
        // Returns error
        return false;
    }

    // Sends data to device
    if (!_writeChecked(0xBE, auxBuffer, 7)) return false;

    // Returns successfully
    this->_lastError = {Error::NONE, DiagnosticStage::NONE, 0};
    return true;
}

//     ///////////////    RAM DATA HANDLING FUNCTIONS     ///////////////     //

bool_t Ds1302::getRamData(uint8_t position_p, uint8_t *buffer_p, uint8_t size_p)
{

    // Checks initialization
    if(!this->_isInitialized()) {
        // Returns error
        return false;
    }

    // CHECK FOR ERROR - invalid pointer
    if(!isPointerValid(buffer_p)) {
        // Returns error
        this->_lastError = {Error::ARGUMENT_POINTER_NULL, DiagnosticStage::NONE, 0};
        return false;
    }
    // CHECK FOR ERROR - wrong number of bytes to receive
    if(size_p == 0) {
        // Returns error
        this->_lastError = {Error::ARGUMENT_CANNOT_BE_ZERO, DiagnosticStage::NONE, 0};
        return false;
    }
    if(size_p > constDs1302RamSize) {
        // Returns error
        this->_lastError = {Error::BUFFER_SIZE_TOO_LARGE, DiagnosticStage::NONE, 0};
        return false;
    }
    // CHECK FOR ERROR - position invalid
    if(position_p > (constDs1302RamSize - 1)) {
        // Returns error
        this->_lastError = {Error::ARGUMENT_VALUE_INVALID, DiagnosticStage::NONE, 0};
        return false;
    }
    // CHECK FOR ERROR - memory overlap
    if((position_p + size_p) > constDs1302RamSize) {
        // Returns error
        this->_lastError = {Error::BUFFER_SIZE_TOO_LARGE, DiagnosticStage::NONE, 0};
        return false;
    }

    // Read RAM data
    for (uint8_t i = 0; i < size_p; ++i) _read(0xC0 + 2 * (position_p + i), buffer_p + i, 1);

    // Returns successfully
    this->_lastError = {Error::NONE, DiagnosticStage::NONE, 0};
    return true;
}

bool_t Ds1302::setRamData(uint8_t position_p, uint8_t *buffer_p, uint8_t size_p)
{

    // Checks initialization
    if(!this->_isInitialized()) {
        // Returns error
        return false;
    }

    // CHECK FOR ERROR - invalid pointer
    if(!isPointerValid(buffer_p)) {
        // Returns error
        this->_lastError = {Error::ARGUMENT_POINTER_NULL, DiagnosticStage::NONE, 0};
        return false;
    }
    // CHECK FOR ERROR - wrong number of bytes to receive
    if(size_p == 0) {
        // Returns error
        this->_lastError = {Error::ARGUMENT_CANNOT_BE_ZERO, DiagnosticStage::NONE, 0};
        return false;
    }
    if(size_p > constDs1302RamSize) {
        // Returns error
        this->_lastError = {Error::BUFFER_SIZE_TOO_LARGE, DiagnosticStage::NONE, 0};
        return false;
    }
    // CHECK FOR ERROR - position invalid
    if(position_p > (constDs1302RamSize - 1)) {
        // Returns error
        this->_lastError = {Error::ARGUMENT_VALUE_INVALID, DiagnosticStage::NONE, 0};
        return false;
    }
    // CHECK FOR ERROR - memory overlap
    if((position_p + size_p) > constDs1302RamSize) {
        // Returns error
        this->_lastError = {Error::BUFFER_SIZE_TOO_LARGE, DiagnosticStage::NONE, 0};
        return false;
    }

    // Write RAM data
    if (!_writeChecked(0xC0 + 2 * position_p, buffer_p, size_p)) return false;

    // Returns successfully
    this->_lastError = {Error::NONE, DiagnosticStage::NONE, 0};
    return true;
}

// =============================================================================
// Class private methods
// =============================================================================

void Ds1302::_clearData(void)
{

    //     ////////////////    PERIPHERAL BUS HANDLER     ////////////////     //
    //     /////////////////     CONTROL AND STATUS     /////////////////     //
    this->_countingHalted               = false;
    this->_initialized                  = false;

    // Returns successfully
    this->_lastError = {Error::NONE, DiagnosticStage::NONE, 0};
    return;
}

bool_t Ds1302::_isInitialized(void)
{

    // CHECK FOR ERROR - peripheral not initialized
    if(!this->_initialized) {
        // Returns error
        this->_lastError = {Error::NOT_INITIALIZED, DiagnosticStage::NONE, 0};
        return false;
    }

    // Returns successfully
    this->_lastError = {Error::NONE, DiagnosticStage::NONE, 0};
    return true;
}

bool_t Ds1302::_isValidDate(uint16_t year_p, uint8_t month_p, uint8_t monthDay_p)
{

    // Checks date values
    if((year_p < 2000) || (year_p > 2099) || (month_p < 1) || (month_p > 12) ||
            (monthDay_p < 1) || (monthDay_p > 31)) {
        // Returns error
        this->_lastError = {Error::DATE_INVALID, DiagnosticStage::NONE, 0};
        return false;
    }
    if(month_p == 2) {
        if(monthDay_p > ((year_p % 4 == 0) ? 29 : 28)) {
            // Returns error
            this->_lastError = {Error::DATE_INVALID, DiagnosticStage::NONE, 0};
            return false;
        }
    } else if((month_p == 4) || (month_p == 6) || (month_p == 9) || (month_p == 11)) {
        if(monthDay_p > 30) {
            // Returns error
            this->_lastError = {Error::DATE_INVALID, DiagnosticStage::NONE, 0};
            return false;
        }
    }

    // Returns successfully
    this->_lastError = {Error::NONE, DiagnosticStage::NONE, 0};
    return true;
}

bool_t Ds1302::_encodeDate(uint8_t *buffer_p, uint16_t year_p, uint8_t month_p, uint8_t monthDay_p)
{

    // Local variables
    uint16_t auxYear;

    // Checks date values
    if(!this->_isValidDate(year_p, month_p, monthDay_p)) {
        // Returns error
        return false;
    }

    // Week day evaluation
    auxYear = year_p - (month_p < 3);
    buffer_p[2] = ((auxYear + auxYear / 4 - auxYear / 100 + auxYear / 400 + "-bed=pen+mad."[month_p] + monthDay_p) % 7) + 1;

    // Encode the information
    year_p -= 2000;
    buffer_p[0] = ((monthDay_p / 10) << 4) | (monthDay_p % 10);
    buffer_p[1] = ((month_p / 10) << 4) | (month_p % 10);
    buffer_p[3] = ((year_p / 10) << 4) | (year_p % 10);

    // Returns successfully
    this->_lastError = {Error::NONE, DiagnosticStage::NONE, 0};
    return true;
}

bool_t Ds1302::_decodeDate(const uint8_t *buffer_p, uint16_t *year_p, uint8_t *month_p, uint8_t *monthDay_p,
        uint8_t *weekDay_p)
{

    // Local variables
    uint16_t auxYear;
    uint8_t auxMonth;
    uint8_t auxMonthDay;
    uint8_t auxWeekDay;

    // Checks BCD values and reserved bits
    if((buffer_p[2] == 0) || (buffer_p[2] & 0xF8) || (buffer_p[0] & 0xC0) || (buffer_p[1] & 0xE0) ||
            ((buffer_p[0] & 0x0F) > 9) || ((buffer_p[1] & 0x0F) > 9) ||
            ((buffer_p[3] & 0x0F) > 9) || ((buffer_p[3] >> 4) > 9)) {
        // Returns error
        this->_lastError = {Error::DATE_INVALID, DiagnosticStage::NONE, 0};
        return false;
    }

    // Decodes the information
    auxWeekDay = buffer_p[2];
    auxMonthDay = ((buffer_p[0] >> 4) * 10) + (buffer_p[0] & 0x0F);
    auxMonth = ((buffer_p[1] >> 4) * 10) + (buffer_p[1] & 0x0F);
    auxYear = ((buffer_p[3] >> 4) * 10) + (buffer_p[3] & 0x0F);
    auxYear += 2000;

    // Checks date values
    if(!this->_isValidDate(auxYear, auxMonth, auxMonthDay)) {
        // Returns error
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
    this->_lastError = {Error::NONE, DiagnosticStage::NONE, 0};
    return true;
}

bool_t Ds1302::_encodeTime(uint8_t *buffer_p, uint8_t hours_p, uint8_t minutes_p, uint8_t seconds_p,
        TimeFormat timeFormat_p, AmPmFlag amPmFlag_p)
{

    // Checks time values
    if((minutes_p > 59) || (seconds_p > 59) ||
            ((timeFormat_p == TimeFormat::FORMAT_12_HOURS) && ((hours_p == 0) || (hours_p > 12))) ||
            ((timeFormat_p == TimeFormat::FORMAT_24_HOURS) && (hours_p > 23))) {
        // Returns error
        this->_lastError = {Error::TIME_INVALID, DiagnosticStage::NONE, 0};
        return false;
    }

    // Encode the information
    buffer_p[0] = ((seconds_p / 10) << 4) | (seconds_p % 10);
    buffer_p[0] |= this->_countingHalted << 7;
    buffer_p[1] = ((minutes_p / 10) << 4) | (minutes_p % 10);
    buffer_p[2] = ((hours_p / 10) << 4) | (hours_p % 10);
    if(timeFormat_p == TimeFormat::FORMAT_12_HOURS) {
        buffer_p[2] |= 0x80;
        buffer_p[2] |= ((amPmFlag_p == AmPmFlag::PM) ? (1 << 5) : 0);
    }

    // Returns successfully
    this->_lastError = {Error::NONE, DiagnosticStage::NONE, 0};
    return true;
}

bool_t Ds1302::_decodeTime(const uint8_t *buffer_p, uint8_t *hours_p, uint8_t *minutes_p, uint8_t *seconds_p,
        TimeFormat timeFormat_p, AmPmFlag *amPmFlag_p)
{

    // Local variables
    uint8_t auxSeconds;
    uint8_t auxMinutes;
    uint8_t auxHours;
    TimeFormat auxTimeFormat;
    AmPmFlag auxAmPmFlag;

    // Checks BCD values and reserved bits
    if((buffer_p[1] & 0x80) || (buffer_p[2] & 0x40) ||
            ((buffer_p[0] & 0x0F) > 9) || ((buffer_p[1] & 0x0F) > 9) ||
            ((buffer_p[2] & 0x0F) > 9)) {
        // Returns error
        this->_lastError = {Error::TIME_INVALID, DiagnosticStage::NONE, 0};
        return false;
    }

    // Decodes the information
    auxSeconds = (((buffer_p[0] & 0x70) >> 4) * 10) + (buffer_p[0] & 0x0F);
    auxMinutes = ((buffer_p[1] >> 4) * 10) + (buffer_p[1] & 0x0F);
    if(isBitSet(buffer_p[2], 7)) {  // 12 hour-mode
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
        this->_lastError = {Error::TIME_INVALID, DiagnosticStage::NONE, 0};
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
        *amPmFlag_p = (timeFormat_p == TimeFormat::FORMAT_24_HOURS) ?
                ((auxHours >= 12) ? AmPmFlag::PM : AmPmFlag::AM) : auxAmPmFlag;
    }

    // Returns successfully
    this->_lastError = {Error::NONE, DiagnosticStage::NONE, 0};
    return true;
}

bool_t Ds1302::_setCounting(bool_t counting_p)
{
    if (!_isInitialized()) return false;
    uint8_t value;
    _read(0x80, &value, 1);
    if (bool(value & 0x80) == counting_p) {
        value = (value & 0x7F) | (counting_p ? 0 : 0x80);
        if (!_writeChecked(0x80, &value, 1)) return false;
    }
    _countingHalted = !counting_p;
    _lastError = {Error::NONE, DiagnosticStage::NONE, 0};
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

// DS1302 three-wire transport. Calls must be serialized in the main loop.
void Ds1302::_begin()
{
    _ce.low();
    _clk.low();
    _io.low();
    _io.setMode(PinMode::OUTPUT_PUSH_PULL);
    _delay_us(10);
    _ce.high();
    _delay_us(10);
}

void Ds1302::_end()
{
    _delay_us(10);
    _ce.low();
    _io.setMode(PinMode::INPUT_FLOATING);
    _clk.low();
    _delay_us(10);
}

void Ds1302::_send(uint8_t value, bool release)
{
    for (uint8_t i = 0; i < 8; ++i) {
        if (value & 1) {
            _io.high();
        } else {
            _io.low();
        }
        _delay_us(10);
        _clk.high();
        _delay_us(10);
        // First read bit appears on the falling edge of command bit 7.
        if (release && i == 7) {
            _io.setMode(PinMode::INPUT_FLOATING);
        }
        _clk.low();
        _delay_us(10);
        value >>= 1;
    }
}

uint8_t Ds1302::_receive()
{
    uint8_t value = 0;
    for (uint8_t i = 0; i < 8; ++i) {
        if (_io.read()) value |= 1U << i;
        _clk.high();
        _delay_us(10);
        _clk.low();
        _delay_us(10);
    }
    return value;
}

void Ds1302::_read(uint8_t command, uint8_t *buffer, uint8_t size)
{
    _begin();
    _send(command | 1, true);
    for (uint8_t i = 0; i < size; ++i) buffer[i] = _receive();
    _end();
}

void Ds1302::_write(uint8_t command, const uint8_t *buffer, uint8_t size)
{
    _begin();
    _send(command & 0xFE);
    for (uint8_t i = 0; i < size; ++i) _send(buffer[i]);
    _end();
}

bool_t Ds1302::_readClock(uint8_t *buffer)
{
    _read(0xBE, buffer, 8);
    if (buffer[7] & 0x7F) {
        _lastError = {Error::COMMUNICATION_FAILED, DiagnosticStage::CLOCK_READ, buffer[7]};
        return false;
    }
    return true;
}

bool_t Ds1302::_writeChecked(uint8_t command, const uint8_t *buffer, uint8_t size)
{
    uint8_t zero = 0, wp, check;
    _read(0x8E, &wp, 1);
    _write(0x8E, &zero, 1);
    _read(0x8E, &check, 1);
    if (check != 0) {
        wp &= 0x80;
        _write(0x8E, &wp, 1);
        _lastError = {Error::COMMUNICATION_FAILED, DiagnosticStage::WRITE_PROTECTION_DISABLE, check};
        return false;
    }
    if (command == 0xBE) {
        uint8_t data[8];
        for (uint8_t i = 0; i < 7; ++i) data[i] = buffer[i];
        data[7] = 0;
        _write(command, data, 8);
    } else {
        for (uint8_t i = 0; i < size; ++i) _write(command + 2*i, buffer+i, 1);
    }
    // Verify writes immediately; a clock rollover during verification can cause failure.
    ErrorInfo verification = {Error::NONE, DiagnosticStage::NONE, 0};
    for (uint8_t i = 0; i < size; ++i) {
        uint8_t reg = (command == 0xBE ? 0x80 : command) + 2*i;
        _read(reg, &check, 1);
        if (check != buffer[i]) {
            verification = {Error::COMMUNICATION_FAILED, DiagnosticStage::WRITE_VERIFY, check};
        }
    }
    wp &= 0x80;
    _write(0x8E, &wp, 1);
    _read(0x8E, &check, 1);
    if (check != wp) {
        _lastError = {Error::COMMUNICATION_FAILED, DiagnosticStage::WRITE_PROTECTION_RESTORE, check};
        return false;
    }
    _lastError = verification;
    return verification.code == Error::NONE;
}

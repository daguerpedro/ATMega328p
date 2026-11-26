//!
//! \file           ds1302.hpp
//! \brief          DS1302 driver adapted from the FunSAPE AVR8 DS1307 driver
//! \author         Leandro Schwarz (bladabuska+funsapeavr8lib@gmail.com)
//! \date           2024-07-10
//! \version        24.07
//! \copyright      license
//! \details        DS1302 RTC module interface, adapted from DS1307.
//! \todo           Todo list
//!

// =============================================================================
// Include guard (START)
// =============================================================================

#ifndef __DS1302_HPP
#define __DS1302_HPP                        2604

// =============================================================================
// Dependencies
// =============================================================================

//     /////////////////    GLOBAL DEFINITIONS FILE     /////////////////     //

#include "../funsape/globalDefines.hpp"

//     ///////////////////     STANDARD C LIBRARY     ///////////////////     //

// NONE

//     ////////////////////    AVR LIBRARY FILES     ////////////////////     //

// NONE

// =============================================================================
// Undefining previous definitions
// =============================================================================

// NONE

// =============================================================================
// Constant definitions
// =============================================================================

// NONE

// =============================================================================
// New data types
// =============================================================================

// NONE

// =============================================================================
// Interrupt callback functions
// =============================================================================

// NONE

// =============================================================================
// Public functions declarations
// =============================================================================

// NONE

// =============================================================================
// Classes
// =============================================================================

//!
//! \brief          Ds1302 class
//! \details        Ds1302 class.
//!
class Ds1302
{
    // -------------------------------------------------------------------------
    // New data types ----------------------------------------------------------
public:

    // Initialization diagnostics: stage and last raw register value.
    uint8_t diagnosticStage = 0;
    uint8_t diagnosticValue = 0;

    enum class Error : uint8_t {
        NONE, NOT_INITIALIZED, ARGUMENT_POINTER_NULL,
        ARGUMENT_VALUE_INVALID, ARGUMENT_CANNOT_BE_ZERO,
        BUFFER_SIZE_TOO_LARGE, DATE_INVALID, TIME_INVALID,
        COMMUNICATION_FAILED
    };

    //     //////////////////////    TIME FORMAT     //////////////////////     //

    //!
    //! \brief      Time format
    //! \details    Time format used to represent hours.
    //!
    enum class TimeFormat : bool_t {
        FORMAT_24_HOURS                 = false,    //!< 24-hours format (0-23)
        FORMAT_12_HOURS                 = true,     //!< 12-hours format (1-12 AM/PM)
    };

    //!
    //! \brief      AM/PM flag
    //! \details    Indicates the period of the day in 12-hours format.
    //!
    enum class AmPmFlag : bool_t {
        AM                              = false,
        PM                              = true,
    };

private:

    //!
    //! \brief          Brief description
    //! \details        Long description
    //!
    enum class Register : uint8_t {
        SECONDS = 0x80, MINUTES = 0x82, HOURS = 0x84,
        DATE = 0x86, MONTH = 0x88, DAY = 0x8A, YEAR = 0x8C,
        CONTROL = 0x8E, TRICKLE = 0x90, RAM_START = 0xC0,
        CLOCK_BURST = 0xBE, RAM_BURST = 0xFE
    };

protected:

    // NONE

    // -------------------------------------------------------------------------
    // Constructors ------------------------------------------------------------
public:

    //!
    //! \brief      Ds1302 class constructor
    //! \details    Creates a Ds1302 object.
    //!
    Ds1302(
            void
    );

    //!
    //! \brief      Ds1302 class destructor
    //! \details    Destroys a Ds1302 object.
    //!
    ~Ds1302(
            void
    );

private:

    // NONE

protected:

    // NONE

    // -------------------------------------------------------------------------
    // Methods - Inherited methods ---------------------------------------------
public:

    // NONE

private:

    // NONE

protected:

    // NONE

    // -------------------------------------------------------------------------
    // Methods - Class own methods ---------------------------------------------
public:

    //     /////////////////     CONTROL AND STATUS     /////////////////     //

    //!
    //! \brief          Brief description
    //! \details        Long description
    //! \return true
    //! \return false
    //!
    bool_t clockStart(
            void
    );

    //!
    //! \brief          Brief description
    //! \details        Long description
    //! \return true
    //! \return false
    //!
    bool_t clockStop(
            void
    );

    //!
    //! \brief          Brief description
    //! \details        Long description
    //! \return         Error              Return info
    //!
    Error getLastError(
            void
    );

    // All three pins must be on the same GPIO port.
    bool_t init(volatile uint8_t &port_p, volatile uint8_t &ddr_p,
            volatile uint8_t &pin_p, uint8_t ce_p, uint8_t clk_p, uint8_t io_p);
    Ds1302(const Ds1302 &) = delete;
    Ds1302 &operator=(const Ds1302 &) = delete;

    //     ///////////////    DATE HANDLING FUNCTIONS     ///////////////     //

    //!
    //! \brief      Reads the current date from device.
    //! \details    The function reads the current date from device.
    //! \param[out] year_p      pointer to the variable where the year will be saved
    //!                             in four digits format (2000-2099).
    //! \param[out] month_p     pointer to the variable where the month will be
    //!                             saved (1-12).
    //! \param[out] monthDay_p  pointer to the variable where the day of the month
    //!                             will be saved (1-31).
    //! \param[out] weekDay_p   pointer to the variable where the day of the week
    //!                             will be saved (1-7), or nullptr if not needed.
    //!                             The value is read directly from the device.
    //! \return true
    //! \return false
    //!
    bool_t getDate(
            uint16_t *year_p,
            uint8_t *month_p,
            uint8_t *monthDay_p,
            uint8_t *weekDay_p = nullptr
    );

    //!
    //! \brief      Set the date at the device.
    //! \details    The function sets the date at the device.
    //! \param[in]  year_p      variable containing the year (2000-2099).
    //! \param[in]  month_p     variable containing the month (1-12).
    //! \param[in]  monthDay_p  variable containing the day of the month (1-31).
    //! \note       The day of the week is calculated automatically (Sunday = 1,
    //!                 Saturday = 7).
    //! \return true
    //! \return false
    //!
    bool_t setDate(
            uint16_t year_p,
            uint8_t month_p,
            uint8_t monthDay_p
    );

    //     ///////////////    TIME HANDLING FUNCTIONS     ///////////////     //

    //!
    //! \brief      Reads the current time from the device.
    //! \details    Reads the current time from the device.
    //! \param[out] hours_p     pointer to the variable where the hours must be
    //!                 saved (0-23 or 1-12, depending on the time format).
    //! \param[out] minutes_p   pointer to the variable where the minutes must be
    //!                 saved (0-59).
    //! \param[out] seconds_p   pointer to the variable where the seconds must be
    //!                 saved (0-59).
    //! \param[in] timeFormat_p    Time format.
    //! \param[out] amPmFlag_p   pointer to the variable where the AM/PM value will be
    //!                 saved. Required in 12-hours format; not meaningful in
    //!                 24-hours format.
    //! \return true
    //! \return false
    //!
    bool_t getTime(
            uint8_t *hours_p,
            uint8_t *minutes_p,
            uint8_t *seconds_p,
            Ds1302::TimeFormat timeFormat_p = Ds1302::TimeFormat::FORMAT_24_HOURS,
            Ds1302::AmPmFlag *amPmFlag_p = nullptr
    );

    //!
    //! \brief      Set the time at the device.
    //! \details    The function sets the time at the device.
    //! \param[in]  hours_p     variable containing the hours (0-23 or 1-12).
    //! \param[in]  minutes_p   variable containing the minutes (0-59).
    //! \param[in]  seconds_p   variable containing the seconds (0-59).
    //! \param[in]  timeFormat_p    \ref Ds1302::TimeFormat variable
    //!                                 containing the time format.
    //! \param[in]  amPmFlag_p      \ref Ds1302::AmPmFlag variable containing
    //!                                 the AM/PM Flag.
    //! \return true
    //! \return false
    //!
    bool_t setTime(
            uint8_t hours_p,
            uint8_t minutes_p,
            uint8_t seconds_p,
            Ds1302::TimeFormat timeFormat_p = Ds1302::TimeFormat::FORMAT_24_HOURS,
            Ds1302::AmPmFlag amPmFlag_p = Ds1302::AmPmFlag::AM
    );

    //     ///////////////    DATA HANDLING FUNCTIONS     ///////////////     //

    //!
    //! \brief      Reads the date and time.
    //! \details    Reads all seven date and time registers in a single transfer.
    //! \param[out] year_p          Year (2000-2099).
    //! \param[out] month_p         Month (1-12).
    //! \param[out] monthDay_p      Day of the month (1-31).
    //! \param[out] weekDay_p       Day of the week (1-7).
    //! \param[out] hours_p         Hours (0-23 or 1-12).
    //! \param[out] minutes_p       Minutes (0-59).
    //! \param[out] seconds_p       Seconds (0-59).
    //! \param[in] timeFormat_p    Time format.
    //! \param[out] amPmFlag_p      AM/PM flag.
    //! \note       weekDay_p may be nullptr. amPmFlag_p is required in 12-hours format.
    //! \return true
    //! \return false
    //!
    bool_t getData(
            uint16_t *year_p,
            uint8_t *month_p,
            uint8_t *monthDay_p,
            uint8_t *weekDay_p,
            uint8_t *hours_p,
            uint8_t *minutes_p,
            uint8_t *seconds_p,
            TimeFormat timeFormat_p = Ds1302::TimeFormat::FORMAT_24_HOURS,
            AmPmFlag *amPmFlag_p = nullptr
    );

    //!
    //! \brief      Writes the date and time.
    //! \details    Writes all seven date and time registers in a single transfer.
    //!                 The day of the week is calculated automatically
    //!                 (Sunday = 1, Saturday = 7).
    //! \param[in] year_p          Year (2000-2099).
    //! \param[in] month_p         Month (1-12).
    //! \param[in] monthDay_p      Day of the month (1-31).
    //! \param[in] hours_p         Hours (0-23 or 1-12).
    //! \param[in] minutes_p       Minutes (0-59).
    //! \param[in] seconds_p       Seconds (0-59).
    //! \param[in] timeFormat_p    Time format.
    //! \param[in] amPmFlag_p      AM/PM flag.
    //! \return true
    //! \return false
    //!
    bool_t setData(
            uint16_t year_p,
            uint8_t month_p,
            uint8_t monthDay_p,
            uint8_t hours_p,
            uint8_t minutes_p,
            uint8_t seconds_p,
            TimeFormat timeFormat_p = Ds1302::TimeFormat::FORMAT_24_HOURS,
            AmPmFlag amPmFlag_p = Ds1302::AmPmFlag::AM
    );

    //     /////////////    RAM DATA HANDLING FUNCTIONS     /////////////     //

    //!
    //! \brief      Reads the device internal RAM.
    //! \details    Reads an amount of bytes from the device internal RAM.
    //! \param[in]  position_p  RAM start position address to be read (0-30).
    //! \param[out] buffer_p    pointer to the location to store the data read.
    //! \param[in]  size_p      number of bytes to read (1-31).
    //! \return true
    //! \return false
    //!
    bool_t getRamData(
            uint8_t position_p,
            uint8_t *buffer_p,
            uint8_t size_p = 1
    );

    //!
    //! \brief      Writes at the device internal RAM.
    //! \details    Writes an amount of bytes at the device internal RAM.
    //! \param[in]  position_p  RAM start position address to be written (0-30).
    //! \param[in]  buffer_p    pointer to the location containing the data do
    //!                             be written.
    //! \param[in] size_p      number of bytes to write (1-31).
    //! \return true
    //! \return false
    //!
    bool_t setRamData(
            uint8_t position_p,
            uint8_t *buffer_p,
            uint8_t size_p = 1
    );

private:

    void _clearData(
            void
    );

    bool_t _decodeDate(
            const uint8_t *buffer_p,
            uint16_t *year_p,
            uint8_t *month_p,
            uint8_t *monthDay_p,
            uint8_t *weekDay_p
    );

    bool_t _decodeTime(
            const uint8_t *buffer_p,
            uint8_t *hours_p,
            uint8_t *minutes_p,
            uint8_t *seconds_p,
            TimeFormat timeFormat_p,
            AmPmFlag *amPmFlag_p
    );

    bool_t _encodeDate(
            uint8_t *buffer_p,
            uint16_t year_p,
            uint8_t month_p,
            uint8_t monthDay_p
    );

    bool_t _encodeTime(
            uint8_t *buffer_p,
            uint8_t hours_p,
            uint8_t minutes_p,
            uint8_t seconds_p,
            TimeFormat timeFormat_p,
            AmPmFlag amPmFlag_p
    );

    bool_t _isValidDate(
            uint16_t year_p,
            uint8_t month_p,
            uint8_t monthDay_p
    );

    bool_t _isInitialized(
            void
    );

    bool_t _setCounting(
            bool_t counting_p
    );

protected:

    // NONE

    // -------------------------------------------------------------------------
    // Properties --------------------------------------------------------------
public:

    // NONE

private:

    //     /////////////////     DEVICE BUS HANDLER     /////////////////     //

    volatile uint8_t *_port, *_ddr, *_pin;
    uint8_t _ce, _clk, _io;
    void _begin();
    void _end();
    void _send(uint8_t value, bool release = false);
    uint8_t _receive();
    void _read(uint8_t command, uint8_t *buffer, uint8_t size);
    void _write(uint8_t command, const uint8_t *buffer, uint8_t size);
    bool_t _writeChecked(uint8_t command, const uint8_t *buffer, uint8_t size);
    bool_t _readClock(uint8_t *buffer);

    //     /////////////////     CONTROL AND STATUS     /////////////////     //

    bool_t          _countingHalted             : 1;
    bool_t          _initialized                : 1;
    Error           _lastError;

protected:

    // NONE

}; // class Ds1302

// =============================================================================
// Inlined class functions
// =============================================================================

// NONE

// =============================================================================
// External global variables
// =============================================================================

// NONE

// =============================================================================
// Include guard (END)
// =============================================================================

#endif  // __DS1302_HPP

// =============================================================================
// END OF FILE - ds1302.hpp
// =============================================================================

/**
 * @file main.cpp
 * @author Mikhail Kalina (apollo.mk58@gmail.com)
 * @brief Main file with setup and loop entry points
 * @version 0.1
 * @date 2024-04-05
 *
 * @copyright Copyright (c) 2024
 *
 */

// Lib headers
#include <Debug.hpp>
#include <SystemTime.hpp>

// Source headers
#include "Battery.hpp"
#include "Board.h"
#include "FileSD.hpp"
#include "FwVersion.hpp"
#include "InternalStorage.hpp"
#include "Measurements/MeasureManager.h"
#include "Serial/SerialManager.hpp"

#if (LOG_LEVEL > LOG_LEVEL_NONE)
// Log level declaration
uint8_t LogsOutput::logLevelMax = LOG_LEVEL;
#endif // #if (LOG_LEVEL > LOG_LEVEL_NONE)

// Modules settings EEPROM addresses declaration
std::array<size_t, static_cast<size_t>(SettingsModules::Count)> InternalStorage::settingsAddressList;

/**
 * @brief Register serial read command handlers
 */
void registerSerialReadHandlers()
{
    static char dataString[Serials::SerialDevice::dataMaxLength];

    LOG_TRACE("Register serial read common handlers");

    Serials::Manager::subscribeToRead(Serials::CommandId::Date,
                                      [](const char **responseString)
                                      {
                                          assert(sizeof(dataString) >= sizeof(SystemTime::DateString));

                                          SystemTime::getStringDate(dataString);

                                          *responseString = dataString;
                                      });

    Serials::Manager::subscribeToRead(Serials::CommandId::Time,
                                      [](const char **responseString)
                                      {
                                          assert(sizeof(dataString) >= sizeof(SystemTime::TimeString));

                                          SystemTime::getStringTime(dataString);

                                          *responseString = dataString;
                                      });

    Serials::Manager::subscribeToRead(Serials::CommandId::LogLevel,
                                      [](const char **responseString)
                                      {
                                          snprintf(dataString, sizeof(dataString), "%u", LOG_GET_LEVEL());
                                          *responseString = dataString;
                                      });

    Serials::Manager::subscribeToRead(Serials::CommandId::FwVersion,
                                      [](const char **responseString)
                                      {
                                          const char *fwString = FwVersion::getVersionString();
                                          *responseString = fwString;
                                      });

    Serials::Manager::subscribeToRead(Serials::CommandId::BatteryStatus,
                                      [](const char **responseString)
                                      {
                                          const auto &batteryStatus = Battery::readStatus();

                                          snprintf(dataString, sizeof(dataString), "%umV %u%%",
                                                   batteryStatus.voltage, batteryStatus.level);
                                          *responseString = dataString;
                                      });
}

/**
 * @brief Register serial write command handlers
 */
void registerSerialWriteHandlers()
{
    LOG_TRACE("Register serial write common handlers");

    Serials::Manager::subscribeToWrite(Serials::CommandId::Date,
                                       [](const char *dataString)
                                       {
                                           SystemTime::setStringDate(dataString);
                                       });

    Serials::Manager::subscribeToWrite(Serials::CommandId::Time,
                                       [](const char *dataString)
                                       {
                                           SystemTime::setStringTime(dataString);
                                       });

    Serials::Manager::subscribeToWrite(Serials::CommandId::LogLevel,
                                       [](const char *dataString)
                                       {
                                           uint8_t logLevel = atoi(dataString);
                                           LOG_SET_LEVEL(logLevel);
                                       });
}

/**
 * @brief Setup preliminary stuff before starting the main loop
 */
void setup()
{
    // Setup the board first
    Board::setup();

    // Initialize internal storage
    InternalStorage::initialize();

    // Initialize battery reading
    Battery::initialize();

    // Initialize serial manager
    Serials::Manager::initialize();
    // Register local serial handlers
    registerSerialReadHandlers();
    registerSerialWriteHandlers();

    // Initialize system time with RTC
    bool status = SystemTime::initialize(Wire);
    if (status == false)
    {
        LOG_ERROR("System time initialization failed");
    }

    // Start SD file system
    status = FileSD::startFileSystem(Board::SpiConfig::frequency);
    if (status == false)
    {
        LOG_ERROR("SD initialization failed");
    }

    status = Measurements::Manager::initialize();
    if (status == false)
    {
        LOG_ERROR("Measurements initialization failed");
    }

    LOG_INFO("Setup done");
}

/**
 * @brief The main loop function
 */
void loop()
{
    // Receive and handle serial commands from serial devices (if available)
    Serials::Manager::process();

    // Perform sensor input data processing (if needed)
    Measurements::Manager::process();
}

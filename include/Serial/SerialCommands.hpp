#pragma once

#include <assert.h>
#include <stdint.h>

namespace Serials
{
    namespace AccessMask
    {
        constexpr uint8_t none = 0;           // No access code
        constexpr uint8_t read = (1 << 0);    // Read access bitmask
        constexpr uint8_t write = (1 << 1);   // Write access bitmask
        constexpr uint8_t execute = (1 << 2); // Execute access bitmask

    } // namespace AccessMask

    /**
     * @brief Serial command identifiers
     */
    enum class CommandId
    {
        SlaveAddress, // 0: Set/Get serial device slave address
        Date,         // 1: Set/Get current date
        Time,         // 2: Set/Get current time
        SerialSelect, // 3: Set/Get serial interface selection

        Commands // Total number of serial commands
    };

    /**
     * @brief Command structure
     */
    struct Command
    {
        CommandId id;       // Command identifier
        const char *string; // Command string
        uint8_t accessMask; // Access bitmask
    };

    // List of commands
    constexpr Command commandsList[] = {
        {
            .id = CommandId::SlaveAddress,
            .string = "ADDR",
            .accessMask = AccessMask::read | AccessMask::write,
        },
        {
            .id = CommandId::Date,
            .string = "DATE",
            .accessMask = AccessMask::read | AccessMask::write,
        },
        {
            .id = CommandId::Time,
            .string = "TIME",
            .accessMask = AccessMask::read | AccessMask::write,
        },
        {
            .id = CommandId::SerialSelect,
            .string = "SERS",
            .accessMask = AccessMask::read | AccessMask::write,
        },
    };
    static_assert(sizeof(commandsList) / sizeof(*commandsList) == static_cast<size_t>(CommandId::Commands),
                  "Commands list doesn't match to commands count!");
} // namespace Serials

// ==========================================================
// REGISTER MAP HEADER FILE
// ==========================================================
//
// This class simulates a simple device register map.
//
// A register map is basically a table:
//
// address -> value
//
// Example:
//
// 100 -> device status
// 101 -> voltage
// 102 -> power
// 103 -> export limit
//
// The inverter can write values into these addresses,
// and other parts of the program can read them later.
// ==========================================================

#pragma once


// Gives us fixed-size integer types such as:
//
// std::uint16_t
// std::int32_t
//
#include <cstdint>


// Gives us std::optional.
//
// optional means:
//
// "There may be a value,
// or there may be no value."
//
#include <optional>


// Gives us std::unordered_map.
//
// This is a container that stores:
//
// key -> value
//
// In this project:
//
// register address -> register value
//
#include <unordered_map>


// ==========================================================
// REGISTER MAP CLASS
// ==========================================================

class RegisterMap
{
public:

    // ======================================================
    // WRITE TO A REGISTER
    // ======================================================
    //
    // Store a value at a register address.
    //
    // Example:
    //
    // write(102, 5000);
    //
    // means:
    //
    // register 102 = 5000
    //
    // In the inverter:
    //
    // register 102 represents power in watts,
    //
    // so:
    //
    // 5000 means 5000 W = 5 kW.
    //
    // Returns true if the write succeeds.
    //
    bool write(
        std::uint16_t address,
        std::int32_t value
    );


    // ======================================================
    // READ FROM A REGISTER
    // ======================================================
    //
    // Read the value stored at a register address.
    //
    // Example:
    //
    // read(102)
    //
    // might return:
    //
    // 4990
    //
    // If the requested register does not exist,
    // there is no valid value to return.
    //
    // Therefore the function returns std::optional.
    //
    std::optional<std::int32_t> read(
        std::uint16_t address
    ) const;


    // ======================================================
    // CHECK WHETHER A REGISTER EXISTS
    // ======================================================
    //
    // Returns:
    //
    // true  -> register exists
    // false -> register does not exist
    //
    bool contains(
        std::uint16_t address
    ) const;


private:

    // ======================================================
    // INTERNAL REGISTER STORAGE
    // ======================================================
    //
    // This is where all register addresses and values
    // are actually stored.
    //
    // std::unordered_map<KeyType, ValueType>
    //
    // Here:
    //
    // KeyType   = std::uint16_t
    //             register address
    //
    // ValueType = std::int32_t
    //             register value
    //
    // Example:
    //
    // registers_[102] = 5000;
    //
    // would mean:
    //
    // address 102 -> value 5000
    //
    std::unordered_map<
        std::uint16_t,
        std::int32_t
    > registers_;
};
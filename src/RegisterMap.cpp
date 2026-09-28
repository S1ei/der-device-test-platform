// ==========================================================
// REGISTER MAP IMPLEMENTATION FILE
// ==========================================================
//
// This file contains the actual behaviour of the RegisterMap.
//
// The RegisterMap stores pairs like:
//
// address -> value
//
// Example:
//
// 100 -> status
// 101 -> voltage
// 102 -> power
// 103 -> export limit
//
// ==========================================================

#include "RegisterMap.h"


// ==========================================================
// WRITE TO A REGISTER
// ==========================================================
//
// Store a value at a particular register address.
//
// Example:
//
// write(102, 5000);
//
// means:
//
// register 102 = 5000
//
// If address 102 already exists, its old value is replaced.
// If address 102 does not exist yet, it is created.
//

bool RegisterMap::write(
    std::uint16_t address,
    std::int32_t value
)
{
    // Store "value" using "address" as the key.
    //
    // Example:
    //
    // registers_[102] = 5000;
    //
    // gives:
    //
    // 102 -> 5000
    //
    registers_[address] = value;


    // In this simplified model, writing always succeeds.
    return true;
}


// ==========================================================
// READ FROM A REGISTER
// ==========================================================
//
// Try to read the value stored at a particular address.
//
// Because the address might not exist, the function returns:
//
// std::optional<std::int32_t>
//
// meaning:
//
// either:
//     there is an integer value
//
// or:
//     there is no value
//

std::optional<std::int32_t> RegisterMap::read(
    std::uint16_t address
) const
{
    // Search the unordered_map for this address.
    //
    // Example:
    //
    // address = 102
    //
    // find(102) searches for register 102.
    //
    const auto it =
        registers_.find(address);


    // registers_.end() represents:
    //
    // "the address was not found"
    //
    // So if the iterator equals end(),
    // the requested register does not exist.
    //
    if (it == registers_.end())
    {
        // std::nullopt means:
        //
        // "there is no value"
        //
        return std::nullopt;
    }


    // If the register exists,
    // return the value stored in it.
    //
    // it->first  = address
    // it->second = value
    //
    // Example:
    //
    // 102 -> 5000
    //
    // it->first  = 102
    // it->second = 5000
    //
    return it->second;
}


// ==========================================================
// CHECK WHETHER A REGISTER EXISTS
// ==========================================================
//
// Return true if the register address exists.
//
// Return false if it does not.
//

bool RegisterMap::contains(
    std::uint16_t address
) const
{
    // Search for the address.
    //
    // If find(address) is NOT equal to end(),
    // then the address exists.
    //
    return
        registers_.find(address)
        != registers_.end();
}
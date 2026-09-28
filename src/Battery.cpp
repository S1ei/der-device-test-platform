// ==========================================================
// BATTERY IMPLEMENTATION FILE
// ==========================================================
//
// This file contains the actual behaviour of the Battery class.
//
// It handles:
//
// - battery construction
// - charging / discharging commands
// - SOC calculation
// - efficiency losses
// - low/high SOC protection
// - communication loss
// - register-map telemetry
//
// ==========================================================


#include "Battery.h"

// std::clamp()
#include <algorithm>

// std::lround()
#include <cmath>


// ==========================================================
// PRIVATE REGISTER ADDRESSES
// ==========================================================
//
// These constants are only used inside Battery.cpp.
//
// The battery uses a different range of register addresses
// from the inverter.
//
// Battery register map:
//
// 200 -> device status
// 201 -> state of charge x10
// 202 -> actual battery power in watts
// 203 -> commanded power setpoint in watts
//
// ==========================================================

namespace {

constexpr std::uint16_t REG_STATUS = 200;

constexpr std::uint16_t REG_SOC_X10 = 201;

constexpr std::uint16_t REG_POWER_W = 202;

constexpr std::uint16_t REG_SETPOINT_W = 203;

} // end anonymous namespace



// ==========================================================
// BATTERY CONSTRUCTOR
// ==========================================================
//
// This runs automatically when a Battery object is created.
//
// Example:
//
// Battery battery(
//     "SIM-BAT-001",
//     13.5,
//     5.0,
//     5.0,
//     50.0,
//     0.95,
//     0.96
// );
//
// means:
//
// device ID              = SIM-BAT-001
// battery capacity       = 13.5 kWh
// maximum charging power = 5 kW
// maximum discharge      = 5 kW
// starting SOC           = 50%
// charging efficiency    = 95%
// discharge efficiency   = 96%
//

Battery::Battery(
    const std::string& deviceId,
    double capacityKWh,
    double maxChargeKW,
    double maxDischargeKW,
    double initialSocPercent,
    double chargeEfficiency,
    double dischargeEfficiency
)
    // ------------------------------------------------------
    // CALL THE PARENT DerDevice CONSTRUCTOR
    // ------------------------------------------------------
    //
    // Pass the device ID to the base class.
    //
    : DerDevice(deviceId),


      // Store battery capacity.
      capacityKWh_(capacityKWh),


      // Store maximum charging power.
      maxChargeKW_(maxChargeKW),


      // Store maximum discharging power.
      maxDischargeKW_(maxDischargeKW),


      // ----------------------------------------------------
      // INITIAL STATE OF CHARGE
      // ----------------------------------------------------
      //
      // std::clamp(value, minimum, maximum)
      //
      // makes sure SOC stays between:
      //
      // 0% and 100%
      //
      // Example:
      //
      // initialSocPercent = 120
      //
      // becomes:
      //
      // 100
      //
      socPercent_(
          std::clamp(
              initialSocPercent,
              0.0,
              100.0
          )
      ),


      // ----------------------------------------------------
      // CHARGING EFFICIENCY
      // ----------------------------------------------------
      //
      // Keep efficiency between 0.01 and 1.0.
      //
      // 1.0 = 100%
      // 0.95 = 95%
      //
      chargeEfficiency_(
          std::clamp(
              chargeEfficiency,
              0.01,
              1.0
          )
      ),


      // ----------------------------------------------------
      // DISCHARGING EFFICIENCY
      // ----------------------------------------------------
      //
      // Same idea as charging efficiency.
      //
      dischargeEfficiency_(
          std::clamp(
              dischargeEfficiency,
              0.01,
              1.0
          )
      )
{
    // Publish starting values into the register map.
    publishRegisters();
}



// ==========================================================
// SET BATTERY POWER SETPOINT
// ==========================================================
//
// This function tries to command the battery to charge
// or discharge at a certain power.
//
// Sign convention:
//
// positive power -> DISCHARGE
// negative power -> CHARGE
//
// Examples:
//
// +3.0 kW -> discharge at 3 kW
// -3.0 kW -> charge at 3 kW
//
// Returns:
//
// true  -> command accepted
// false -> command rejected
//

bool Battery::setPowerSetpointKW(double setpointKW)
{
    // ------------------------------------------------------
    // CHECK 1: COMMUNICATION AVAILABLE?
    // ------------------------------------------------------
    //
    // If communication is unavailable,
    // commands cannot be accepted.
    //
    if (!communicationAvailable_)
    {
        return false;
    }


    // ------------------------------------------------------
    // CHECK 2: POWER LIMIT
    // ------------------------------------------------------
    //
    // Charging is negative.
    //
    // Therefore the lowest allowed value is:
    //
    // -maxChargeKW_
    //
    // Discharging is positive.
    //
    // Therefore the highest allowed value is:
    //
    // +maxDischargeKW_
    //
    // Example:
    //
    // max charge    = 5 kW
    // max discharge = 5 kW
    //
    // allowed range:
    //
    // -5 kW <= setpoint <= +5 kW
    //
    if (
        setpointKW < -maxChargeKW_ ||
        setpointKW > maxDischargeKW_
    )
    {
        return false;
    }


    // ------------------------------------------------------
    // CHECK 3: SOC PROTECTION
    // ------------------------------------------------------
    //
    // Positive power means discharge.
    // Negative power means charge.
    //
    // Do not allow discharge if SOC is 5% or lower.
    //
    // Do not allow charging if SOC is 95% or higher.
    //
    if (
        (socPercent_ <= 5.0 && setpointKW > 0.0)
        ||
        (socPercent_ >= 95.0 && setpointKW < 0.0)
    )
    {
        return false;
    }


    // If all checks pass,
    // store the requested battery power.
    powerSetpointKW_ = setpointKW;


    // Update register telemetry.
    publishRegisters();


    return true;
}



// ==========================================================
// SIMULATE TELEMETRY
// ==========================================================
//
// DerDevice required every child class to implement
// simulateTelemetry().
//
// For the battery, this simply advances the simulation
// by 60 seconds.
//
// 60 seconds = 1 minute.
//

void Battery::simulateTelemetry()
{
    simulateStep(60.0);
}



// ==========================================================
// SIMULATE BATTERY FOR A TIME STEP
// ==========================================================
//
// This is the main battery simulation function.
//
// dtSeconds tells us how much time passes.
//
// Example:
//
// simulateStep(600.0)
//
// means:
//
// simulate 600 seconds = 10 minutes.
//
// The battery SOC is then updated using:
//
// E = P * t
//
// while including charge/discharge efficiency.
//

void Battery::simulateStep(double dtSeconds)
{
    // ------------------------------------------------------
    // CHECK 1: VALID TIME STEP
    // ------------------------------------------------------
    //
    // Zero or negative time does not make sense here.
    //
    if (dtSeconds <= 0.0)
    {
        return;
    }


    // ------------------------------------------------------
    // CHECK 2: COMMUNICATION LOSS
    // ------------------------------------------------------
    //
    // If communication is unavailable:
    //
    // - set status to CommunicationLost
    // - update registers
    // - stop processing this simulation step
    //
    if (!communicationAvailable_)
    {
        status_ = Status::CommunicationLost;

        publishRegisters();

        return;
    }


    // ------------------------------------------------------
    // CHECK 3: IS THE BATTERY RUNNING?
    // ------------------------------------------------------
    //
    // If the battery is not Running,
    // actual battery power becomes zero.
    //
    if (status_ != Status::Running)
    {
        powerKW_ = 0.0;

        publishRegisters();

        return;
    }


    // ------------------------------------------------------
    // APPLY POWER COMMAND
    // ------------------------------------------------------
    //
    // In this simple model, once the command has been
    // accepted, actual battery power equals the setpoint.
    //
    powerKW_ = powerSetpointKW_;


    // ------------------------------------------------------
    // CONVERT TIME FROM SECONDS TO HOURS
    // ------------------------------------------------------
    //
    // Battery capacity is measured in kWh.
    //
    // Therefore time must be in hours when using:
    //
    // E = P * t
    //
    // 3600 seconds = 1 hour.
    //
    const double dtHours =
        dtSeconds / 3600.0;


    // ======================================================
    // DISCHARGING
    // ======================================================
    //
    // Positive battery power means discharge.
    //
    if (powerKW_ > 0.0)
    {
        // --------------------------------------------------
        // CALCULATE ENERGY REMOVED FROM BATTERY
        // --------------------------------------------------
        //
        // To deliver power externally, slightly more energy
        // must be removed from the battery because discharge
        // efficiency is below 100%.
        //
        // Formula:
        //
        //                      P
        // battery energy = --------- * time
        //                 efficiency
        //
        // Example:
        //
        // output power = 3 kW
        // efficiency   = 0.96
        //
        // battery internally supplies:
        //
        // 3 / 0.96
        // = 3.125 kW equivalent
        //
        const double batteryEnergyRemovedKWh =
            (
                powerKW_
                /
                dischargeEfficiency_
            )
            *
            dtHours;


        // --------------------------------------------------
        // CONVERT ENERGY REMOVED INTO SOC CHANGE
        // --------------------------------------------------
        //
        // Example:
        //
        // battery capacity = 13.5 kWh
        //
        // if 0.5 kWh is removed:
        //
        // SOC decrease:
        //
        // 0.5 / 13.5 * 100
        //
        // ≈ 3.70%
        //
        socPercent_ -=
            (
                batteryEnergyRemovedKWh
                /
                capacityKWh_
            )
            *
            100.0;
    }


    // ======================================================
    // CHARGING
    // ======================================================
    //
    // Negative battery power means charging.
    //
    else if (powerKW_ < 0.0)
    {
        // --------------------------------------------------
        // CALCULATE STORED ENERGY
        // --------------------------------------------------
        //
        // powerKW_ is negative while charging.
        //
        // Therefore:
        //
        // -powerKW_
        //
        // converts it into a positive power magnitude.
        //
        // Example:
        //
        // powerKW_ = -3.0
        //
        // -powerKW_ = 3.0
        //
        //
        // Charging efficiency means not all incoming energy
        // is stored.
        //
        // Formula:
        //
        // stored energy =
        //
        // charging power
        // × efficiency
        // × time
        //
        const double storedEnergyKWh =
            (-powerKW_)
            *
            chargeEfficiency_
            *
            dtHours;


        // Increase SOC based on the energy stored.
        socPercent_ +=
            (
                storedEnergyKWh
                /
                capacityKWh_
            )
            *
            100.0;
    }


    // ------------------------------------------------------
    // KEEP SOC BETWEEN 0% AND 100%
    // ------------------------------------------------------
    //
    // Even if a large simulation step would mathematically
    // push SOC below 0 or above 100,
    // clamp it to a physically sensible range.
    //
    socPercent_ =
        std::clamp(
            socPercent_,
            0.0,
            100.0
        );


    // Update battery telemetry registers.
    publishRegisters();
}



// ==========================================================
// GETTER FUNCTIONS
// ==========================================================
//
// These simply return battery information.
//

double Battery::capacityKWh() const
{
    return capacityKWh_;
}


double Battery::socPercent() const
{
    return socPercent_;
}


double Battery::powerKW() const
{
    return powerKW_;
}


double Battery::powerSetpointKW() const
{
    return powerSetpointKW_;
}


double Battery::chargeEfficiency() const
{
    return chargeEfficiency_;
}


double Battery::dischargeEfficiency() const
{
    return dischargeEfficiency_;
}


const RegisterMap& Battery::registers() const
{
    return registers_;
}



// ==========================================================
// PUBLISH BATTERY REGISTERS
// ==========================================================
//
// This copies important battery values into the
// simulated register map.
//
// Register 200 -> device status
// Register 201 -> SOC x10
// Register 202 -> actual power in watts
// Register 203 -> power setpoint in watts
//

void Battery::publishRegisters()
{
    // ------------------------------------------------------
    // REGISTER 200: STATUS
    // ------------------------------------------------------
    //
    // Convert the Status enum into an integer.
    //
    registers_.write(
        REG_STATUS,

        static_cast<std::int32_t>(
            status_
        )
    );


    // ------------------------------------------------------
    // REGISTER 201: SOC
    // ------------------------------------------------------
    //
    // SOC is multiplied by 10 so one decimal place can
    // be represented using an integer.
    //
    // Example:
    //
    // SOC = 48.7%
    //
    // stored value:
    //
    // 487
    //
    registers_.write(
        REG_SOC_X10,

        static_cast<std::int32_t>(
            std::lround(
                socPercent_ * 10.0
            )
        )
    );


    // ------------------------------------------------------
    // REGISTER 202: ACTUAL POWER
    // ------------------------------------------------------
    //
    // Convert:
    //
    // kW -> W
    //
    // Example:
    //
    // +3.0 kW -> +3000 W
    //
    // -3.0 kW -> -3000 W
    //
    registers_.write(
        REG_POWER_W,

        static_cast<std::int32_t>(
            std::lround(
                powerKW_ * 1000.0
            )
        )
    );


    // ------------------------------------------------------
    // REGISTER 203: POWER SETPOINT
    // ------------------------------------------------------
    //
    // Convert the requested power:
    //
    // kW -> W
    //
    registers_.write(
        REG_SETPOINT_W,

        static_cast<std::int32_t>(
            std::lround(
                powerSetpointKW_ * 1000.0
            )
        )
    );
}
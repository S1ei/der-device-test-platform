// ==========================================================
// EV CHARGER IMPLEMENTATION FILE
// ==========================================================
//
// This file contains the actual behaviour of the EVCharger.
//
// It handles:
//
// - charger construction
// - current-limit commands
// - first-order current response
// - simulated current measurement bias
// - power calculation using P = V * I
// - communication-loss handling
// - register-map telemetry
//
// ==========================================================


#include "EVCharger.h"

// std::clamp()
#include <algorithm>

// std::exp() and std::lround()
#include <cmath>


// ==========================================================
// PRIVATE CONSTANTS
// ==========================================================
//
// These values are only used inside EVCharger.cpp.
//
// Charger register map:
//
// 300 -> charger status
// 301 -> measured current x10
// 302 -> charging power in watts
// 303 -> current limit x10
//
// ==========================================================

namespace {

constexpr std::uint16_t REG_STATUS = 300;

constexpr std::uint16_t REG_CURRENT_X10 = 301;

constexpr std::uint16_t REG_POWER_W = 302;

constexpr std::uint16_t REG_LIMIT_X10 = 303;


// ----------------------------------------------------------
// SIMULATED CURRENT SENSOR BIAS
// ----------------------------------------------------------
//
// This represents a small fixed measurement error.
//
// The simulated current sensor always reads:
//
// actual current - 0.05 A
//
// Example:
//
// actualCurrentA_ = 16.00 A
//
// measuredCurrentA_ = 15.95 A
//
// This makes the measurement slightly more realistic
// while remaining completely repeatable.
//
constexpr double CURRENT_SENSOR_BIAS_A = -0.05;

} // end anonymous namespace



// ==========================================================
// EV CHARGER CONSTRUCTOR
// ==========================================================
//
// This runs automatically when an EVCharger object is created.
//
// Example:
//
// EVCharger charger(
//     "SIM-EVSE-001",
//     230.0,
//     32.0
// );
//
// means:
//
// device ID       = SIM-EVSE-001
// nominal voltage = 230 V
// maximum current = 32 A
//

EVCharger::EVCharger(
    const std::string& deviceId,
    double nominalVoltageV,
    double maxCurrentA
)
    // Pass the device ID to the DerDevice base class.
    : DerDevice(deviceId),

      // Store nominal supply voltage.
      nominalVoltageV_(nominalVoltageV),

      // Store maximum allowed charger current.
      maxCurrentA_(maxCurrentA)
{
    // Publish the initial values into the register map.
    publishRegisters();
}



// ==========================================================
// SET CURRENT LIMIT
// ==========================================================
//
// Try to change the charger current limit.
//
// Example:
//
// setCurrentLimitA(16.0);
//
// means:
//
// request a charging current of 16 A.
//
// Returns:
//
// true  -> accepted
// false -> rejected
//

bool EVCharger::setCurrentLimitA(double currentA)
{
    // Reject the command if ANY of these are true:
    //
    // 1. communication is unavailable
    // 2. current is negative
    // 3. current is above charger rating
    //
    if (
        !communicationAvailable_
        ||
        currentA < 0.0
        ||
        currentA > maxCurrentA_
    )
    {
        return false;
    }


    // Store the valid current-limit command.
    currentLimitA_ = currentA;


    // Update register telemetry.
    publishRegisters();


    return true;
}



// ==========================================================
// SET FIRST-ORDER TIME CONSTANT
// ==========================================================
//
// This controls how quickly the charger current approaches
// the commanded current limit.
//
// Smaller tau -> faster response
// Larger tau  -> slower response
//
// Only positive values are accepted.
//

void EVCharger::setTimeConstantSeconds(double tauSeconds)
{
    if (tauSeconds > 0.0)
    {
        timeConstantSeconds_ = tauSeconds;
    }
}



// ==========================================================
// SIMULATE TELEMETRY
// ==========================================================
//
// DerDevice requires every child class to provide its own
// simulateTelemetry() implementation.
//
// For the EV charger:
//
// simulateTelemetry()
//
// means:
//
// simulate 1 second.
//

void EVCharger::simulateTelemetry()
{
    simulateStep(1.0);
}



// ==========================================================
// MAIN EV CHARGER SIMULATION FUNCTION
// ==========================================================
//
// dtSeconds tells the simulation how much time should pass.
//
// Example:
//
// simulateStep(0.5);
//
// means:
//
// move the charger simulation forward by 0.5 seconds.
//

void EVCharger::simulateStep(double dtSeconds)
{
    // ------------------------------------------------------
    // CHECK 1: VALID TIME STEP
    // ------------------------------------------------------
    //
    // Do nothing if time is zero or negative.
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
    // - change state to CommunicationLost
    // - update register telemetry
    // - stop processing this simulation step
    //
    if (!communicationAvailable_)
    {
        status_ = Status::CommunicationLost;

        publishRegisters();

        return;
    }


    // ------------------------------------------------------
    // CHECK 3: IS THE CHARGER RUNNING?
    // ------------------------------------------------------
    //
    // Only calculate charging current if the device
    // is in the Running state.
    //
    if (status_ == Status::Running)
    {
        // ==================================================
        // FIRST-ORDER CURRENT RESPONSE
        // ==================================================
        //
        // The charger current does not instantly jump
        // from 0 A to the commanded current.
        //
        // Instead it gradually approaches the target.
        //
        // Continuous model:
        //
        //               I_command - I
        // dI/dt = ---------------------------
        //                    tau
        //
        //
        // I_command = currentLimitA_
        //
        // I         = actualCurrentA_
        //
        // tau       = timeConstantSeconds_
        //
        //
        // This is the same type of first-order response
        // used by the inverter power model.
        //


        // --------------------------------------------------
        // CALCULATE alpha
        // --------------------------------------------------
        //
        // Convert the continuous first-order model into
        // an exact discrete update:
        //
        // alpha = 1 - e^(-dt/tau)
        //
        const double alpha =
            1.0
            -
            std::exp(
                -dtSeconds
                /
                timeConstantSeconds_
            );


        // --------------------------------------------------
        // UPDATE ACTUAL CURRENT
        // --------------------------------------------------
        //
        // New current =
        //
        // old current
        //
        // +
        //
        // alpha * (target - old current)
        //
        //
        // Example:
        //
        // target = 16 A
        // actual = 0 A
        // alpha  = 0.28
        //
        // new actual current:
        //
        // 0 + 0.28(16 - 0)
        //
        // = 4.48 A
        //
        actualCurrentA_ +=
            alpha
            *
            (
                currentLimitA_
                -
                actualCurrentA_
            );


        // ==================================================
        // SIMULATED CURRENT MEASUREMENT
        // ==================================================
        //
        // The ideal current is actualCurrentA_.
        //
        // The reported current is measuredCurrentA_.
        //
        // A fixed sensor bias of -0.05 A is added.
        //
        // Example:
        //
        // actual current = 16.00 A
        // sensor bias    = -0.05 A
        //
        // measured       = 15.95 A
        //
        //
        // std::clamp() ensures the reported current remains
        // between:
        //
        // 0 A
        //
        // and
        //
        // maxCurrentA_
        //
        measuredCurrentA_ =
            std::clamp(
                actualCurrentA_
                    + CURRENT_SENSOR_BIAS_A,

                0.0,

                maxCurrentA_
            );


        // ==================================================
        // CALCULATE CHARGING POWER
        // ==================================================
        //
        // Electrical power:
        //
        // P = V * I
        //
        //
        // nominalVoltageV_ is in volts.
        //
        // measuredCurrentA_ is in amps.
        //
        // Therefore:
        //
        // V * A = watts
        //
        //
        // Divide by 1000 to convert:
        //
        // watts -> kilowatts
        //
        // Example:
        //
        // V = 230 V
        // I = 16 A
        //
        // P = 230 * 16
        //   = 3680 W
        //   = 3.68 kW
        //
        powerKW_ =
            nominalVoltageV_
            *
            measuredCurrentA_
            /
            1000.0;
    }
    else
    {
        // --------------------------------------------------
        // CHARGER NOT RUNNING
        // --------------------------------------------------
        //
        // If the charger is stopped or otherwise not running,
        // force all current and power values to zero.
        //
        actualCurrentA_ = 0.0;

        measuredCurrentA_ = 0.0;

        powerKW_ = 0.0;
    }


    // After every simulation step,
    // publish current values to the register map.
    publishRegisters();
}



// ==========================================================
// GETTER FUNCTIONS
// ==========================================================
//
// These functions simply return charger information.
//

double EVCharger::nominalVoltageV() const
{
    return nominalVoltageV_;
}


double EVCharger::currentLimitA() const
{
    return currentLimitA_;
}


double EVCharger::measuredCurrentA() const
{
    return measuredCurrentA_;
}


double EVCharger::powerKW() const
{
    return powerKW_;
}


double EVCharger::timeConstantSeconds() const
{
    return timeConstantSeconds_;
}


const RegisterMap& EVCharger::registers() const
{
    return registers_;
}



// ==========================================================
// PUBLISH EV CHARGER REGISTERS
// ==========================================================
//
// Copy important charger values into its simulated
// register map.
//
// Register 300 -> device status
// Register 301 -> measured current x10
// Register 302 -> power in watts
// Register 303 -> current limit x10
//

void EVCharger::publishRegisters()
{
    // ------------------------------------------------------
    // REGISTER 300: STATUS
    // ------------------------------------------------------
    //
    registers_.write(
        REG_STATUS,

        static_cast<std::int32_t>(
            status_
        )
    );


    // ------------------------------------------------------
    // REGISTER 301: MEASURED CURRENT
    // ------------------------------------------------------
    //
    // Multiply by 10 so one decimal place can be stored
    // using an integer.
    //
    // Example:
    //
    // 15.9 A
    //
    // becomes:
    //
    // 159
    //
    registers_.write(
        REG_CURRENT_X10,

        static_cast<std::int32_t>(
            std::lround(
                measuredCurrentA_
                *
                10.0
            )
        )
    );


    // ------------------------------------------------------
    // REGISTER 302: POWER
    // ------------------------------------------------------
    //
    // Convert:
    //
    // kW -> W
    //
    // Example:
    //
    // 3.67 kW
    //
    // becomes approximately:
    //
    // 3670 W
    //
    registers_.write(
        REG_POWER_W,

        static_cast<std::int32_t>(
            std::lround(
                powerKW_
                *
                1000.0
            )
        )
    );


    // ------------------------------------------------------
    // REGISTER 303: CURRENT LIMIT
    // ------------------------------------------------------
    //
    // Multiply by 10 to preserve one decimal place.
    //
    // Example:
    //
    // current limit = 16.0 A
    //
    // stored as:
    //
    // 160
    //
    registers_.write(
        REG_LIMIT_X10,

        static_cast<std::int32_t>(
            std::lround(
                currentLimitA_
                *
                10.0
            )
        )
    );
}
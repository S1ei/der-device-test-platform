// ==========================================================
// INVERTER IMPLEMENTATION FILE
// ==========================================================
//
// This file contains the actual behaviour of the Inverter class.
//
// In main.cpp we did things such as:
//
// inverter.setExportLimitKW(5.0);
// inverter.simulateStep(1.0);
// inverter.setGridVoltageV(255.0);
//
// This file explains what those functions actually do.
// ==========================================================


#include "Inverter.h"

// std::clamp()
#include <algorithm>

// std::array
#include <array>

// std::exp() and std::lround()
#include <cmath>


// ==========================================================
// PRIVATE CONSTANTS USED ONLY IN THIS FILE
// ==========================================================
//
// This anonymous namespace means these values/functions
// are only intended to be used inside Inverter.cpp.
//

namespace {


// ----------------------------------------------------------
// SIMULATED REGISTER ADDRESSES
// ----------------------------------------------------------
//
// These numbers act like addresses in a simple register map.
//
// Think of the register map as a table:
//
// Address 100 -> inverter status
// Address 101 -> grid voltage
// Address 102 -> output power
// Address 103 -> export limit
//
//
// constexpr means:
//
// "This value is constant and known at compile time."
//
// std::uint16_t means:
//
// unsigned 16-bit integer.
//
// These register numbers are all positive integers,
// so an unsigned integer makes sense.
//

constexpr std::uint16_t REG_STATUS = 100;

constexpr std::uint16_t REG_VOLTAGE_X10 = 101;

constexpr std::uint16_t REG_POWER_W = 102;

constexpr std::uint16_t REG_EXPORT_LIMIT_W = 103;


// ----------------------------------------------------------
// SIMULATED MEASUREMENT NOISE
// ----------------------------------------------------------
//
// Real sensors do not usually report perfectly exact values.
//
// Instead of using random noise, this project uses a fixed
// repeating sequence so every test gives the same result.
//
// This is called deterministic noise.
//
// There are 8 values in the array.
//
// Units: kW
//
// Example:
// -0.010 kW = -10 W
//  0.004 kW = +4 W
//

constexpr std::array<double, 8> MEASUREMENT_NOISE_KW{
    -0.010,
     0.004,
    -0.006,
     0.008,
    -0.003,
     0.006,
    -0.008,
     0.002
};

} // end anonymous namespace



// ==========================================================
// CONSTRUCTOR
// ==========================================================
//
// This function runs automatically whenever we create
// an Inverter object.
//
// Example from main.cpp:
//
// Inverter inverter("SIM-INV-001", 10.0);
//
// deviceId       = "SIM-INV-001"
// ratedPowerKW   = 10.0
//
// The ": DerDevice(deviceId)" part calls the constructor
// of the parent DerDevice class.
//
// ratedPowerKW_(ratedPowerKW)
//
// means:
//
// store the value passed into the constructor inside the
// inverter's ratedPowerKW_ member variable.
//

Inverter::Inverter(
    const std::string& deviceId,
    double ratedPowerKW
)
    : DerDevice(deviceId),
      ratedPowerKW_(ratedPowerKW)
{
    // Immediately publish the starting values
    // into the simulated register map.
    publishRegisters();
}



// ==========================================================
// SET EXPORT LIMIT
// ==========================================================
//
// This function tries to change the inverter's export limit.
//
// Example:
//
// inverter.setExportLimitKW(5.0);
//
// returns:
//
// true  -> command accepted
// false -> command rejected
//

bool Inverter::setExportLimitKW(double limitKW)
{
    // Reject the command if ANY of these are true:
    //
    // 1. communication is unavailable
    // 2. requested limit is negative
    // 3. requested limit is above inverter rating
    //
    // || means OR.
    //
    if (
        !communicationAvailable_ ||
        limitKW < 0.0 ||
        limitKW > ratedPowerKW_
    )
    {
        return false;
    }


    // If the command is valid, store the new export limit.
    exportLimitKW_ = limitKW;


    // Update the simulated register map
    // so telemetry reflects the new setting.
    publishRegisters();


    // Tell the caller that the command succeeded.
    return true;
}



// ==========================================================
// SET GRID VOLTAGE
// ==========================================================
//
// This changes the simulated grid voltage.
//
// Example:
//
// inverter.setGridVoltageV(255.0);
//
// means:
//
// simulate the inverter seeing 255 V at its grid connection.
//

void Inverter::setGridVoltageV(double voltageV)
{
    voltageV_ = voltageV;

    // Update telemetry registers.
    publishRegisters();
}



// ==========================================================
// SET DEMONSTRATION OVER-VOLTAGE THRESHOLD
// ==========================================================
//
// This lets the simulator change the voltage at which
// over-voltage protection activates.
//
// The value is only accepted if it is positive.
//

void Inverter::setDemoOverVoltageThresholdV(
    double thresholdV
)
{
    if (thresholdV > 0.0)
    {
        demoOverVoltageThresholdV_ = thresholdV;
    }
}



// ==========================================================
// SET TIME CONSTANT
// ==========================================================
//
// The time constant controls how quickly power approaches
// the commanded/export-limit value.
//
// Smaller tau -> faster response
// Larger tau  -> slower response
//
// Only positive time constants are allowed.
//

void Inverter::setTimeConstantSeconds(
    double tauSeconds
)
{
    if (tauSeconds > 0.0)
    {
        timeConstantSeconds_ = tauSeconds;
    }
}



// ==========================================================
// CLEAR FAULT
// ==========================================================
//
// This attempts to reset the inverter after a fault.
//
// It returns:
//
// true  -> fault successfully cleared
// false -> fault cannot be cleared
//

bool Inverter::clearFault()
{
    // Do NOT allow the fault to clear if:
    //
    // 1. communication is unavailable
    //
    // OR
    //
    // 2. grid voltage is still above the
    //    over-voltage threshold
    //
    if (
        !communicationAvailable_ ||
        voltageV_ > demoOverVoltageThresholdV_
    )
    {
        return false;
    }


    // Clear the over-voltage fault flag.
    overVoltageFault_ = false;


    // After clearing a fault, put the inverter
    // into the Stopped state.
    //
    // It must be started again separately.
    status_ = Status::Stopped;


    // Reset internal actual power to zero.
    actualPowerKW_ = 0.0;


    // Reset measured power to zero.
    powerKW_ = 0.0;


    // Update the simulated register map.
    publishRegisters();


    return true;
}



// ==========================================================
// GETTER FUNCTIONS
// ==========================================================
//
// These functions let other parts of the program READ
// inverter information.
//
// The word "const" at the end means:
//
// "This function promises not to modify the inverter."
//

double Inverter::ratedPowerKW() const
{
    return ratedPowerKW_;
}


double Inverter::exportLimitKW() const
{
    return exportLimitKW_;
}


double Inverter::powerKW() const
{
    return powerKW_;
}


double Inverter::voltageV() const
{
    return voltageV_;
}


double Inverter::timeConstantSeconds() const
{
    return timeConstantSeconds_;
}


bool Inverter::overVoltageFault() const
{
    return overVoltageFault_;
}


const RegisterMap& Inverter::registers() const
{
    return registers_;
}



// ==========================================================
// SIMULATE TELEMETRY
// ==========================================================
//
// This is just a convenient shortcut.
//
// Calling:
//
// inverter.simulateTelemetry();
//
// is equivalent to:
//
// inverter.simulateStep(1.0);
//
// meaning simulate one second.
//

void Inverter::simulateTelemetry()
{
    simulateStep(1.0);
}



// ==========================================================
// MAIN INVERTER SIMULATION FUNCTION
// ==========================================================
//
// This is the most important function in Inverter.cpp.
//
// dtSeconds tells the simulator how much time should pass.
//
// Example:
//
// simulateStep(1.0)
//
// means:
//
// advance the inverter simulation by 1 second.
//

void Inverter::simulateStep(double dtSeconds)
{
    // ------------------------------------------------------
    // CHECK 1: INVALID TIME STEP
    // ------------------------------------------------------
    //
    // We cannot simulate zero or negative time.
    //
    if (dtSeconds <= 0.0)
    {
        return;
    }


    // ------------------------------------------------------
    // CHECK 2: COMMUNICATION LOSS
    // ------------------------------------------------------
    //
    // If communication is unavailable, change the
    // device state to CommunicationLost.
    //
    if (!communicationAvailable_)
    {
        status_ = Status::CommunicationLost;

        publishRegisters();

        return;
    }


    // ------------------------------------------------------
    // CHECK 3: OVER-VOLTAGE
    // ------------------------------------------------------
    //
    // Compare measured grid voltage with the configured
    // demonstration protection threshold.
    //
    // The result of this comparison is true or false.
    //
    overVoltageFault_ =
        voltageV_ > demoOverVoltageThresholdV_;


    // If an over-voltage fault exists:
    //
    // 1. change state to Faulted
    // 2. force actual output to zero
    // 3. force measured output to zero
    // 4. update telemetry
    //
    if (overVoltageFault_)
    {
        status_ = Status::Faulted;

        actualPowerKW_ = 0.0;

        powerKW_ = 0.0;

        publishRegisters();

        return;
    }


    // ------------------------------------------------------
    // NORMAL RUNNING OPERATION
    // ------------------------------------------------------
    //
    // Only produce power if the inverter is currently
    // in the Running state.
    //
    if (status_ == Status::Running)
    {
        // ==================================================
        // FIRST-ORDER POWER RESPONSE
        // ==================================================
        //
        // The continuous model is:
        //
        //             P_command - P
        // dP/dt = -------------------------
        //                  tau
        //
        //
        // P_command = exportLimitKW_
        //
        // P          = current actualPowerKW_
        //
        // tau        = timeConstantSeconds_
        //
        //
        // This means:
        //
        // the further current power is from the target,
        // the faster it moves toward the target.
        //
        // As it gets closer to the target,
        // the change becomes smaller.
        //


        // --------------------------------------------------
        // CALCULATE alpha
        // --------------------------------------------------
        //
        // This equation converts the continuous first-order
        // model into a discrete simulation update.
        //
        // alpha = 1 - e^(-dt/tau)
        //
        // std::exp(x) calculates e^x.
        //
        const double alpha =
            1.0 -
            std::exp(
                -dtSeconds /
                timeConstantSeconds_
            );


        // --------------------------------------------------
        // UPDATE ACTUAL POWER
        // --------------------------------------------------
        //
        // New power =
        //
        // old power
        //
        // +
        //
        // alpha * (target - old power)
        //
        //
        // Example:
        //
        // old power = 0 kW
        // target    = 5 kW
        // alpha     = 0.28
        //
        // new power =
        //
        // 0 + 0.28(5 - 0)
        //
        // = 1.4 kW
        //
        actualPowerKW_ +=
            alpha *
            (
                exportLimitKW_ -
                actualPowerKW_
            );


        // ==================================================
        // SIMULATED SENSOR MEASUREMENT
        // ==================================================
        //
        // actualPowerKW_
        //
        // represents the ideal internal simulated power.
        //
        // powerKW_
        //
        // represents what a sensor/telemetry system reports.
        //
        // A small deterministic measurement error is added.
        //
        // std::clamp(value, minimum, maximum)
        //
        // prevents the reported value from going:
        //
        // below 0 kW
        //
        // or
        //
        // above the inverter's rated power.
        //
        powerKW_ =
            std::clamp(
                actualPowerKW_
                    + nextMeasurementNoiseKW(),

                0.0,

                ratedPowerKW_
            );
    }
    else
    {
        // If the inverter is NOT running,
        // force power output to zero.
        actualPowerKW_ = 0.0;

        powerKW_ = 0.0;
    }


    // After every simulation step,
    // publish the new values into the register map.
    publishRegisters();
}



// ==========================================================
// GET NEXT MEASUREMENT NOISE SAMPLE
// ==========================================================
//
// This returns one value from the fixed noise array.
//
// Each call moves to the next value.
//
// After reaching the end of the array,
// it loops back to the beginning.
//

double Inverter::nextMeasurementNoiseKW()
{
    // sampleIndex_ tells us which noise sample to use.
    //
    // % is the modulo operator.
    //
    // It returns the remainder after division.
    //
    // This allows the index to wrap around.
    //
    // Example with array size 8:
    //
    // 0 % 8 = 0
    // 1 % 8 = 1
    // ...
    // 7 % 8 = 7
    // 8 % 8 = 0
    // 9 % 8 = 1
    //
    const double noise =
        MEASUREMENT_NOISE_KW[
            sampleIndex_
            %
            MEASUREMENT_NOISE_KW.size()
        ];


    // Move to the next sample for next time.
    ++sampleIndex_;


    // Return the selected noise value.
    return noise;
}



// ==========================================================
// PUBLISH TELEMETRY TO REGISTER MAP
// ==========================================================
//
// This function copies important inverter values into
// the simulated register map.
//
// This is similar to how a real embedded device may expose
// data through numbered registers.
//
// Register 100 -> status
// Register 101 -> voltage
// Register 102 -> power
// Register 103 -> export limit
//

void Inverter::publishRegisters()
{
    // ------------------------------------------------------
    // REGISTER 100: DEVICE STATUS
    // ------------------------------------------------------
    //
    // status_ is an enum such as:
    //
    // Stopped
    // Running
    // Faulted
    // CommunicationLost
    //
    // static_cast converts it into an integer so it can
    // be stored in the register.
    //
    registers_.write(
        REG_STATUS,
        static_cast<std::int32_t>(
            status_
        )
    );


    // ------------------------------------------------------
    // REGISTER 101: GRID VOLTAGE
    // ------------------------------------------------------
    //
    // Voltage is stored multiplied by 10.
    //
    // Example:
    //
    // 230.0 V
    //
    // becomes:
    //
    // 2300
    //
    // This lets us store one decimal place while using
    // an integer register.
    //
    // std::lround() rounds to the nearest whole number.
    //
    registers_.write(
        REG_VOLTAGE_X10,

        static_cast<std::int32_t>(
            std::lround(
                voltageV_ * 10.0
            )
        )
    );


    // ------------------------------------------------------
    // REGISTER 102: OUTPUT POWER
    // ------------------------------------------------------
    //
    // powerKW_ is stored in kW.
    //
    // Multiply by 1000 to convert:
    //
    // kW -> W
    //
    // Example:
    //
    // 4.99 kW -> 4990 W
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
    // REGISTER 103: EXPORT LIMIT
    // ------------------------------------------------------
    //
    // Same conversion:
    //
    // kW -> W
    //
    // Example:
    //
    // 5.0 kW -> 5000 W
    //
    registers_.write(
        REG_EXPORT_LIMIT_W,

        static_cast<std::int32_t>(
            std::lround(
                exportLimitKW_ * 1000.0
            )
        )
    );
}
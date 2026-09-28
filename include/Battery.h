// ==========================================================
// BATTERY HEADER FILE
// ==========================================================
//
// This file DECLARES the Battery class.
//
// Battery is another type of DER device.
//
// It inherits the common behaviour from DerDevice:
//
// - device ID
// - status
// - communication availability
// - start()
// - stop()
//
// Then Battery adds battery-specific behaviour:
//
// - energy capacity
// - state of charge (SOC)
// - charging power limit
// - discharging power limit
// - charging efficiency
// - discharging efficiency
// - power setpoint
// - register telemetry
//
// ==========================================================

#pragma once


// Battery inherits from DerDevice.
#include "DerDevice.h"

// Battery publishes telemetry into a register map.
#include "RegisterMap.h"


// ==========================================================
// BATTERY CLASS
// ==========================================================
//
// "Battery : public DerDevice"
//
// means:
//
// Battery is a specialised type of DerDevice.
//
//              DerDevice
//                  |
//                  v
//               Battery
//
// ==========================================================

class Battery : public DerDevice
{
public:

    // ======================================================
    // CONSTRUCTOR
    // ======================================================
    //
    // Runs when a Battery object is created.
    //
    // Example from main.cpp:
    //
    // Battery battery(
    //     "SIM-BAT-001",
    //     13.5,
    //     5.0,
    //     5.0,
    //     50.0
    // );
    //
    // deviceId
    //     -> identifier for the battery
    //
    // capacityKWh
    //     -> total battery energy capacity
    //
    // maxChargeKW
    //     -> maximum charging power
    //
    // maxDischargeKW
    //     -> maximum discharging power
    //
    // initialSocPercent
    //     -> starting state of charge
    //
    // chargeEfficiency
    //     -> efficiency while charging
    //
    // dischargeEfficiency
    //     -> efficiency while discharging
    //
    Battery(
        const std::string& deviceId,
        double capacityKWh,
        double maxChargeKW,
        double maxDischargeKW,

        // Default starting SOC = 50%
        double initialSocPercent = 50.0,

        // Default charging efficiency = 95%
        double chargeEfficiency = 0.95,

        // Default discharging efficiency = 96%
        double dischargeEfficiency = 0.96
    );


    // ======================================================
    // SET BATTERY POWER COMMAND
    // ======================================================
    //
    // Change the battery power setpoint.
    //
    // In this project:
    //
    // positive power -> battery discharging
    //
    // negative power -> battery charging
    //
    // Example:
    //
    // setPowerSetpointKW(3.0)
    //
    // means discharge at 3 kW.
    //
    // setPowerSetpointKW(-3.0)
    //
    // means charge at 3 kW.
    //
    // Returns:
    //
    // true  -> command accepted
    // false -> command rejected
    //
    bool setPowerSetpointKW(double setpointKW);


    // ======================================================
    // TELEMETRY SIMULATION
    // ======================================================
    //
    // Battery provides its own implementation of the
    // pure virtual function from DerDevice.
    //
    void simulateTelemetry() override;


    // Advance the battery simulation by dtSeconds.
    //
    // Example:
    //
    // simulateStep(600.0)
    //
    // means simulate 600 seconds = 10 minutes.
    //
    void simulateStep(double dtSeconds);


    // ======================================================
    // GETTER FUNCTIONS
    // ======================================================

    // Return total battery energy capacity.
    //
    // Unit: kWh
    //
    double capacityKWh() const;


    // Return current battery state of charge.
    //
    // Unit: %
    //
    double socPercent() const;


    // Return current battery power.
    //
    // Unit: kW
    //
    double powerKW() const;


    // Return the requested power setpoint.
    //
    // Unit: kW
    //
    double powerSetpointKW() const;


    // Return charging efficiency.
    //
    // Example:
    //
    // 0.95 = 95%
    //
    double chargeEfficiency() const;


    // Return discharging efficiency.
    //
    // Example:
    //
    // 0.96 = 96%
    //
    double dischargeEfficiency() const;


    // Return read-only access to the battery register map.
    const RegisterMap& registers() const;


private:

    // ======================================================
    // PRIVATE HELPER FUNCTION
    // ======================================================
    //
    // Copy the battery's current values into its
    // simulated register map.
    //
    void publishRegisters();


    // ======================================================
    // BATTERY MEMBER VARIABLES
    // ======================================================

    // ------------------------------------------------------
    // ENERGY CAPACITY
    // ------------------------------------------------------
    //
    // Total amount of energy the battery can store.
    //
    // Example:
    //
    // 13.5 kWh
    //
    double capacityKWh_;


    // ------------------------------------------------------
    // MAXIMUM CHARGING POWER
    // ------------------------------------------------------
    //
    // Highest allowed charging rate.
    //
    // Example:
    //
    // 5 kW
    //
    double maxChargeKW_;


    // ------------------------------------------------------
    // MAXIMUM DISCHARGING POWER
    // ------------------------------------------------------
    //
    // Highest allowed discharging rate.
    //
    // Example:
    //
    // 5 kW
    //
    double maxDischargeKW_;


    // ------------------------------------------------------
    // STATE OF CHARGE
    // ------------------------------------------------------
    //
    // Current battery energy level as a percentage.
    //
    // Example:
    //
    // 50.0 means 50% charged.
    //
    double socPercent_;


    // ------------------------------------------------------
    // CHARGING EFFICIENCY
    // ------------------------------------------------------
    //
    // Represents energy losses while charging.
    //
    // Example:
    //
    // 0.95 = 95% efficient
    //
    double chargeEfficiency_;


    // ------------------------------------------------------
    // DISCHARGING EFFICIENCY
    // ------------------------------------------------------
    //
    // Represents energy losses while delivering power.
    //
    // Example:
    //
    // 0.96 = 96% efficient
    //
    double dischargeEfficiency_;


    // ------------------------------------------------------
    // POWER SETPOINT
    // ------------------------------------------------------
    //
    // Requested battery power.
    //
    // Starts at 0 kW.
    //
    // positive -> discharge
    // negative -> charge
    //
    double powerSetpointKW_{0.0};


    // ------------------------------------------------------
    // ACTUAL BATTERY POWER
    // ------------------------------------------------------
    //
    // Current simulated battery power.
    //
    // Starts at 0 kW.
    //
    double powerKW_{0.0};


    // ------------------------------------------------------
    // REGISTER MAP
    // ------------------------------------------------------
    //
    // Stores battery telemetry in numbered registers.
    //
    RegisterMap registers_;
};
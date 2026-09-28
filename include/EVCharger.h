// ==========================================================
// EV CHARGER HEADER FILE
// ==========================================================
//
// This file DECLARES the EVCharger class.
//
// EVCharger is another type of DER device.
//
// It inherits common behaviour from DerDevice:
//
// - device ID
// - status
// - communication availability
// - start()
// - stop()
//
// Then EVCharger adds charger-specific behaviour:
//
// - nominal voltage
// - maximum current
// - commanded current limit
// - actual current
// - measured current
// - charging power
// - first-order current response
// - register telemetry
//
// ==========================================================

#pragma once


// EVCharger inherits from DerDevice.
#include "DerDevice.h"

// EVCharger stores telemetry in a RegisterMap.
#include "RegisterMap.h"


// ==========================================================
// EV CHARGER CLASS
// ==========================================================
//
// "EVCharger : public DerDevice"
//
// means:
//
// EVCharger is a specialised type of DerDevice.
//
//              DerDevice
//                  |
//                  v
//              EVCharger
//
// ==========================================================

class EVCharger : public DerDevice
{
public:

    // ======================================================
    // CONSTRUCTOR
    // ======================================================
    //
    // Runs whenever an EVCharger object is created.
    //
    // Example:
    //
    // EVCharger charger(
    //     "SIM-EVSE-001",
    //     230.0,
    //     32.0
    // );
    //
    // deviceId
    //     -> device identifier
    //
    // nominalVoltageV
    //     -> charger supply voltage
    //
    // maxCurrentA
    //     -> maximum charger current
    //
    EVCharger(
        const std::string& deviceId,
        double nominalVoltageV,
        double maxCurrentA
    );


    // ======================================================
    // SET CURRENT LIMIT
    // ======================================================
    //
    // Try to change the EV charger's current limit.
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
    // true  -> command accepted
    // false -> command rejected
    //
    bool setCurrentLimitA(double currentA);


    // ======================================================
    // SET TIME CONSTANT
    // ======================================================
    //
    // Controls how quickly charging current approaches
    // the requested current limit.
    //
    // Smaller tau -> faster response
    // Larger tau  -> slower response
    //
    void setTimeConstantSeconds(double tauSeconds);


    // ======================================================
    // TELEMETRY SIMULATION
    // ======================================================
    //
    // Implements the pure virtual function from DerDevice.
    //
    void simulateTelemetry() override;


    // Advance the EV charger simulation by dtSeconds.
    //
    // Example:
    //
    // simulateStep(0.5)
    //
    // means simulate 0.5 seconds.
    //
    void simulateStep(double dtSeconds);


    // ======================================================
    // GETTER FUNCTIONS
    // ======================================================

    // Return charger nominal voltage.
    //
    // Unit: V
    //
    double nominalVoltageV() const;


    // Return the commanded current limit.
    //
    // Unit: A
    //
    double currentLimitA() const;


    // Return the simulated measured current.
    //
    // Unit: A
    //
    double measuredCurrentA() const;


    // Return charger power.
    //
    // Unit: kW
    //
    double powerKW() const;


    // Return current-response time constant.
    //
    // Unit: seconds
    //
    double timeConstantSeconds() const;


    // Return read-only access to the charger register map.
    const RegisterMap& registers() const;


private:

    // ======================================================
    // PRIVATE HELPER FUNCTION
    // ======================================================
    //
    // Copy the charger's current values into its
    // simulated register map.
    //
    void publishRegisters();


    // ======================================================
    // EV CHARGER MEMBER VARIABLES
    // ======================================================

    // ------------------------------------------------------
    // NOMINAL VOLTAGE
    // ------------------------------------------------------
    //
    // Supply voltage used by the charger.
    //
    // Example:
    //
    // 230 V
    //
    double nominalVoltageV_;


    // ------------------------------------------------------
    // MAXIMUM CURRENT
    // ------------------------------------------------------
    //
    // Highest charging current allowed.
    //
    // Example:
    //
    // 32 A
    //
    double maxCurrentA_;


    // ------------------------------------------------------
    // CURRENT LIMIT COMMAND
    // ------------------------------------------------------
    //
    // The requested charging current.
    //
    // Starts at 0 A.
    //
    double currentLimitA_{0.0};


    // ------------------------------------------------------
    // ACTUAL CURRENT
    // ------------------------------------------------------
    //
    // Internal ideal simulated current.
    //
    // This value gradually moves toward currentLimitA_
    // using a first-order response.
    //
    double actualCurrentA_{0.0};


    // ------------------------------------------------------
    // MEASURED CURRENT
    // ------------------------------------------------------
    //
    // The current reported by the simulated charger.
    //
    // Depending on the implementation, this may include
    // measurement effects or simply follow actual current.
    //
    double measuredCurrentA_{0.0};


    // ------------------------------------------------------
    // CHARGING POWER
    // ------------------------------------------------------
    //
    // Calculated charging power.
    //
    // Likely based on:
    //
    // P = V * I
    //
    // and converted from watts to kilowatts.
    //
    double powerKW_{0.0};


    // ------------------------------------------------------
    // FIRST-ORDER TIME CONSTANT
    // ------------------------------------------------------
    //
    // Controls how quickly current approaches the command.
    //
    // Default:
    //
    // tau = 1.5 seconds
    //
    double timeConstantSeconds_{1.5};


    // ------------------------------------------------------
    // REGISTER MAP
    // ------------------------------------------------------
    //
    // Stores simulated charger telemetry.
    //
    RegisterMap registers_;
};
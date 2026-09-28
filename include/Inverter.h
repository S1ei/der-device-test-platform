// ==========================================================
// INVERTER HEADER FILE
// ==========================================================
//
// This file DECLARES the Inverter class.
//
// Think of it as the "blueprint" for an inverter.
//
// It tells C++:
//
// - what functions an Inverter has
// - what data an Inverter stores
// - what is public
// - what is private
//
// The actual function code is written in Inverter.cpp.
// ==========================================================


// ----------------------------------------------------------
// #pragma once
// ----------------------------------------------------------
//
// Prevents this header file from being included more than once
// during compilation.
//
// Without this, C++ could accidentally see the Inverter class
// definition multiple times.
//
#pragma once


// ----------------------------------------------------------
// OTHER PROJECT HEADER FILES
// ----------------------------------------------------------
//
// DerDevice.h contains the parent/base DER device class.
//
// RegisterMap.h contains the simulated register map.
//
#include "DerDevice.h"
#include "RegisterMap.h"


// ==========================================================
// INVERTER CLASS
// ==========================================================
//
// "class Inverter : public DerDevice"
//
// means:
//
// Inverter is a specialised type of DerDevice.
//
// Inheritance:
//
//              DerDevice
//                  |
//                  v
//              Inverter
//
// Therefore Inverter automatically receives the public/
// protected behaviour provided by DerDevice.
//
// Then we add inverter-specific behaviour here.
//

class Inverter : public DerDevice
{
public:

    // ======================================================
    // CONSTRUCTOR
    // ======================================================
    //
    // Runs when we create an inverter.
    //
    // Example:
    //
    // Inverter inverter("SIM-INV-001", 10.0);
    //
    // deviceId       = "SIM-INV-001"
    // ratedPowerKW   = 10.0 kW
    //
    Inverter(
        const std::string& deviceId,
        double ratedPowerKW
    );


    // ======================================================
    // COMMAND / CONTROL FUNCTIONS
    // ======================================================

    // Try to change the export limit.
    //
    // Returns:
    //
    // true  -> accepted
    // false -> rejected
    //
    bool setExportLimitKW(double limitKW);


    // Change the simulated grid voltage.
    void setGridVoltageV(double voltageV);


    // Change the demonstration over-voltage threshold.
    void setDemoOverVoltageThresholdV(double thresholdV);


    // Change the inverter's first-order response time constant.
    void setTimeConstantSeconds(double tauSeconds);


    // Try to clear an inverter fault.
    //
    // Returns true if the fault can be cleared.
    bool clearFault();


    // ======================================================
    // GETTER FUNCTIONS
    // ======================================================
    //
    // These allow other parts of the program to READ
    // inverter information.
    //
    // The "const" at the end means the function promises
    // not to change the inverter.
    //

    // Return the inverter's rated power.
    double ratedPowerKW() const;


    // Return the current export limit.
    double exportLimitKW() const;


    // Return the measured/reported output power.
    double powerKW() const;


    // Return the current simulated grid voltage.
    double voltageV() const;


    // Return the first-order model time constant.
    double timeConstantSeconds() const;


    // Return whether an over-voltage fault exists.
    bool overVoltageFault() const;


    // ======================================================
    // REGISTER MAP
    // ======================================================
    //
    // Return the inverter's register map.
    //
    // The "&" means we return a reference rather than making
    // an entire copy of the RegisterMap object.
    //
    // The first "const" means whoever receives this reference
    // cannot modify the register map through this function.
    //
    const RegisterMap& registers() const;


    // ======================================================
    // SIMULATION FUNCTIONS
    // ======================================================

    // Simulate/update telemetry.
    //
    // "override" means DerDevice already defines a function
    // called simulateTelemetry(), and Inverter provides its
    // own version of that function.
    //
    void simulateTelemetry() override;


    // Advance the inverter simulation by dtSeconds.
    //
    // Example:
    //
    // simulateStep(1.0);
    //
    // means simulate 1 second.
    //
    void simulateStep(double dtSeconds);


private:

    // ======================================================
    // PRIVATE HELPER FUNCTIONS
    // ======================================================
    //
    // These functions are only meant to be used internally
    // by the Inverter class.
    //
    // Code outside the class cannot call them directly.
    //

    // Copy current inverter values into the register map.
    void publishRegisters();


    // Return the next deterministic measurement-noise value.
    double nextMeasurementNoiseKW();


    // ======================================================
    // PRIVATE MEMBER VARIABLES
    // ======================================================
    //
    // These variables store the inverter's internal state.
    //
    // They belong to each individual Inverter object.
    //


    // Maximum rated inverter power.
    //
    // Example:
    //
    // 10.0 kW
    //
    double ratedPowerKW_;


    // Current export limit.
    //
    // Starts at 0 kW.
    //
    double exportLimitKW_{0.0};


    // Ideal internal simulated power.
    //
    // This is the power before measurement noise is added.
    //
    double actualPowerKW_{0.0};


    // Reported/measured inverter power.
    //
    // Small deterministic measurement noise may be added
    // to actualPowerKW_ to produce this value.
    //
    double powerKW_{0.0};


    // Simulated grid voltage.
    //
    // Starts at 230 V.
    //
    double voltageV_{230.0};


    // Demonstration over-voltage threshold.
    //
    // Starts at 253 V.
    //
    double demoOverVoltageThresholdV_{253.0};


    // First-order response time constant.
    //
    // Starts at 3 seconds.
    //
    double timeConstantSeconds_{3.0};


    // Stores whether an over-voltage fault currently exists.
    //
    // false = no fault
    // true  = fault present
    //
    bool overVoltageFault_{false};


    // Keeps track of which measurement-noise sample
    // should be used next.
    //
    // Starts from sample 0.
    //
    std::size_t sampleIndex_{0};


    // The inverter's simulated register map.
    RegisterMap registers_;
};
// ==========================================================
// DER DEVICE BASE CLASS
// ==========================================================
//
// This file defines the common behaviour shared by all
// Distributed Energy Resource devices in the project.
//
// Examples of DER devices:
//
// - Inverter
// - Battery
// - EV Charger
//
// Instead of rewriting common code for every device,
// we put shared features inside DerDevice.
//
// Then:
//
// Inverter   inherits from DerDevice
// Battery    inherits from DerDevice
// EVCharger  inherits from DerDevice
//
// ==========================================================


// ----------------------------------------------------------
// #pragma once
// ----------------------------------------------------------
//
// Prevents this header file from being included more than
// once during compilation.
//
#pragma once


// std::string is used for storing text such as device IDs.
#include <string>


// ==========================================================
// BASE CLASS: DerDevice
// ==========================================================
//
// DerDevice represents the general/common idea of a DER.
//
// It stores things that all DER devices have, such as:
//
// - device ID
// - operating status
// - communication availability
//
// It also defines common functions such as:
//
// - start()
// - stop()
// - setCommunicationAvailable()
//
// Some functions are intentionally left for child classes
// to implement themselves.
//
// Example:
//
// DerDevice
//     |
//     +---- Inverter
//     +---- Battery
//     +---- EVCharger
//

class DerDevice
{
public:

    // ======================================================
    // DEVICE STATUS
    // ======================================================
    //
    // enum class creates a set of named possible states.
    //
    // A DER device can only have one of these statuses:
    //
    // Stopped
    // Running
    // Faulted
    // CommunicationLost
    //
    enum class Status
    {
        Stopped,
        Running,
        Faulted,
        CommunicationLost
    };


    // ======================================================
    // CONSTRUCTOR
    // ======================================================
    //
    // This runs whenever a DerDevice-based object is created.
    //
    // Example:
    //
    // Inverter inverter("SIM-INV-001", 10.0);
    //
    // The Inverter constructor passes:
    //
    // "SIM-INV-001"
    //
    // into this DerDevice constructor.
    //
    explicit DerDevice(std::string deviceId);


    // ======================================================
    // VIRTUAL DESTRUCTOR
    // ======================================================
    //
    // A destructor runs when an object is destroyed.
    //
    // "virtual" is important because DerDevice is a base class.
    //
    // It ensures that if a child object such as Inverter
    // is deleted through a DerDevice pointer/reference,
    // the correct destructor is used.
    //
    // "= default" means:
    //
    // use C++'s normal automatically generated destructor.
    //
    virtual ~DerDevice() = default;


    // ======================================================
    // GETTER FUNCTIONS
    // ======================================================

    // Return the device ID.
    //
    // Example:
    //
    // "SIM-INV-001"
    //
    // The & means return a reference instead of copying
    // the whole string.
    //
    const std::string& deviceId() const;


    // Return the current device status.
    //
    // Example:
    //
    // Status::Running
    //
    Status status() const;


    // Return whether communication is available.
    //
    // true  = communication available
    // false = communication lost
    //
    bool communicationAvailable() const;


    // ======================================================
    // COMMUNICATION CONTROL
    // ======================================================

    // Change whether communication is available.
    //
    // Example:
    //
    // setCommunicationAvailable(false);
    //
    // simulates communication loss.
    //
    void setCommunicationAvailable(bool available);


    // ======================================================
    // START / STOP FUNCTIONS
    // ======================================================
    //
    // "virtual" means child classes are allowed to replace
    // these functions with their own versions if required.
    //

    // Start the DER device.
    virtual void start();


    // Stop the DER device.
    virtual void stop();


    // ======================================================
    // TELEMETRY SIMULATION
    // ======================================================
    //
    // Every DER device must provide its own implementation
    // of simulateTelemetry().
    //
    // "= 0" makes this a PURE VIRTUAL FUNCTION.
    //
    // That means DerDevice itself does NOT provide the final
    // implementation.
    //
    // Child classes must implement it.
    //
    // For example:
    //
    // Inverter::simulateTelemetry()
    // Battery::simulateTelemetry()
    // EVCharger::simulateTelemetry()
    //
    virtual void simulateTelemetry() = 0;


    // ======================================================
    // STATUS TO TEXT FUNCTION
    // ======================================================
    //
    // Converts a Status value into readable text.
    //
    // Example:
    //
    // Status::Running
    //
    // becomes:
    //
    // "RUNNING"
    //
    // "static" means we can call this using the class name
    // without needing a specific DerDevice object.
    //
    // Example:
    //
    // DerDevice::statusToString(status);
    //
    static std::string statusToString(Status status);


protected:

    // ======================================================
    // PROTECTED MEMBER VARIABLES
    // ======================================================
    //
    // "protected" is similar to private, but child classes
    // are allowed to access these variables directly.
    //
    // Therefore:
    //
    // Inverter
    // Battery
    // EVCharger
    //
    // can use these variables.
    //
    // But unrelated outside code cannot directly modify them.
    //


    // Unique device identifier.
    //
    // Example:
    //
    // "SIM-INV-001"
    //
    std::string deviceId_;


    // Current operating status.
    //
    // Every device starts in the Stopped state.
    //
    Status status_{Status::Stopped};


    // Whether communication with the device is available.
    //
    // Every device starts with communication available.
    //
    bool communicationAvailable_{true};
};
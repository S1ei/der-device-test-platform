// ==========================================================
// DER DEVICE IMPLEMENTATION FILE
// ==========================================================
//
// This file contains the actual behaviour of the DerDevice
// base class.
//
// DerDevice stores behaviour that is shared by:
//
// - Inverter
// - Battery
// - EVCharger
//
// Examples:
//
// - storing the device ID
// - starting / stopping a device
// - tracking communication availability
// - storing the device status
// - converting status values into readable text
//
// ==========================================================


#include "DerDevice.h"

// std::move()
#include <utility>


// ==========================================================
// CONSTRUCTOR
// ==========================================================
//
// This constructor runs whenever a DerDevice-based object
// is created.
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
// The job here is to store that device ID inside deviceId_.
//

DerDevice::DerDevice(std::string deviceId)
    : deviceId_(std::move(deviceId))
{
}


// ==========================================================
// GET DEVICE ID
// ==========================================================
//
// Return the stored device ID.
//
// Example:
//
// inverter.deviceId()
//
// might return:
//
// "SIM-INV-001"
//
// The "&" means we return a reference to the existing string
// instead of creating a full copy.
//
// The final "const" means this function does not change
// the object.
//

const std::string& DerDevice::deviceId() const
{
    return deviceId_;
}


// ==========================================================
// GET DEVICE STATUS
// ==========================================================
//
// Return the current operating state.
//
// Possible values include:
//
// Status::Stopped
// Status::Running
// Status::Faulted
// Status::CommunicationLost
//

DerDevice::Status DerDevice::status() const
{
    return status_;
}


// ==========================================================
// CHECK COMMUNICATION AVAILABILITY
// ==========================================================
//
// Return:
//
// true  -> communication is available
// false -> communication is unavailable
//

bool DerDevice::communicationAvailable() const
{
    return communicationAvailable_;
}


// ==========================================================
// SET COMMUNICATION AVAILABILITY
// ==========================================================
//
// This function allows us to simulate communication
// being available or unavailable.
//
// Example:
//
// inverter.setCommunicationAvailable(false);
//
// means:
//
// simulate loss of communication with the inverter.
//

void DerDevice::setCommunicationAvailable(bool available)
{
    // Store the new communication condition.
    communicationAvailable_ = available;


    // ------------------------------------------------------
    // COMMUNICATION LOST
    // ------------------------------------------------------
    //
    // If available == false:
    //
    // !available becomes true
    //
    // and the device status becomes CommunicationLost.
    //
    if (!available)
    {
        status_ = Status::CommunicationLost;
    }


    // ------------------------------------------------------
    // COMMUNICATION RESTORED
    // ------------------------------------------------------
    //
    // If communication becomes available again AND
    // the device was previously in CommunicationLost,
    //
    // move the device into the Stopped state.
    //
    // It does NOT automatically start running again.
    //
    else if (status_ == Status::CommunicationLost)
    {
        status_ = Status::Stopped;
    }
}


// ==========================================================
// START DEVICE
// ==========================================================
//
// Start the DER device.
//
// The device only starts if communication is available.
//
// If communication is lost, calling start() does nothing.
//

void DerDevice::start()
{
    if (communicationAvailable_)
    {
        status_ = Status::Running;
    }
}


// ==========================================================
// STOP DEVICE
// ==========================================================
//
// Stop the DER device.
//
// The device only changes to Stopped if communication
// is available.
//

void DerDevice::stop()
{
    if (communicationAvailable_)
    {
        status_ = Status::Stopped;
    }
}


// ==========================================================
// CONVERT STATUS TO TEXT
// ==========================================================
//
// The internal status is stored as an enum:
//
// Status::Stopped
// Status::Running
// Status::Faulted
// Status::CommunicationLost
//
// This function converts those enum values into text
// that can be printed or written into a CSV file.
//
// Example:
//
// Status::Running
//
// becomes:
//
// "RUNNING"
//

std::string DerDevice::statusToString(Status status)
{
    // switch() checks which possible Status value we have.
    switch (status)
    {
        case Status::Stopped:
            return "STOPPED";

        case Status::Running:
            return "RUNNING";

        case Status::Faulted:
            return "FAULTED";

        case Status::CommunicationLost:
            return "COMM_LOST";
    }


    // This is a safety fallback.
    //
    // In normal operation one of the cases above
    // should always match.
    return "UNKNOWN";
}
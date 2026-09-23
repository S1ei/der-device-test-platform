#include "DerDevice.h"
#include <utility>

DerDevice::DerDevice(std::string deviceId)
    : deviceId_(std::move(deviceId)) {}

const std::string& DerDevice::deviceId() const {
    return deviceId_;
}

DerDevice::Status DerDevice::status() const {
    return status_;
}

bool DerDevice::communicationAvailable() const {
    return communicationAvailable_;
}

void DerDevice::setCommunicationAvailable(bool available) {
    communicationAvailable_ = available;
    if (!available) {
        status_ = Status::CommunicationLost;
    } else if (status_ == Status::CommunicationLost) {
        status_ = Status::Stopped;
    }
}

void DerDevice::start() {
    if (communicationAvailable_) {
        status_ = Status::Running;
    }
}

void DerDevice::stop() {
    if (communicationAvailable_) {
        status_ = Status::Stopped;
    }
}

std::string DerDevice::statusToString(Status status) {
    switch (status) {
        case Status::Stopped: return "STOPPED";
        case Status::Running: return "RUNNING";
        case Status::Faulted: return "FAULTED";
        case Status::CommunicationLost: return "COMM_LOST";
    }
    return "UNKNOWN";
}

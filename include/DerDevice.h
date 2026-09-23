#pragma once

#include <string>

class DerDevice {
public:
    enum class Status {
        Stopped,
        Running,
        Faulted,
        CommunicationLost
    };

    explicit DerDevice(std::string deviceId);
    virtual ~DerDevice() = default;

    const std::string& deviceId() const;
    Status status() const;
    bool communicationAvailable() const;

    void setCommunicationAvailable(bool available);
    virtual void start();
    virtual void stop();
    virtual void simulateTelemetry() = 0;

    static std::string statusToString(Status status);

protected:
    std::string deviceId_;
    Status status_{Status::Stopped};
    bool communicationAvailable_{true};
};

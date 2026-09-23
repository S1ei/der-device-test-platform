#include "EVCharger.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr std::uint16_t REG_STATUS = 300;
constexpr std::uint16_t REG_CURRENT_X10 = 301;
constexpr std::uint16_t REG_POWER_W = 302;
constexpr std::uint16_t REG_LIMIT_X10 = 303;
constexpr double CURRENT_SENSOR_BIAS_A = -0.05;
}

EVCharger::EVCharger(const std::string& deviceId,
                     double nominalVoltageV,
                     double maxCurrentA)
    : DerDevice(deviceId),
      nominalVoltageV_(nominalVoltageV),
      maxCurrentA_(maxCurrentA) {
    publishRegisters();
}

bool EVCharger::setCurrentLimitA(double currentA) {
    if (!communicationAvailable_ || currentA < 0.0 || currentA > maxCurrentA_) {
        return false;
    }
    currentLimitA_ = currentA;
    publishRegisters();
    return true;
}

void EVCharger::setTimeConstantSeconds(double tauSeconds) {
    if (tauSeconds > 0.0) {
        timeConstantSeconds_ = tauSeconds;
    }
}

void EVCharger::simulateTelemetry() {
    simulateStep(1.0);
}

void EVCharger::simulateStep(double dtSeconds) {
    if (dtSeconds <= 0.0) {
        return;
    }

    if (!communicationAvailable_) {
        status_ = Status::CommunicationLost;
        publishRegisters();
        return;
    }

    if (status_ == Status::Running) {
        const double alpha = 1.0 - std::exp(-dtSeconds / timeConstantSeconds_);
        actualCurrentA_ += alpha * (currentLimitA_ - actualCurrentA_);
        measuredCurrentA_ = std::clamp(actualCurrentA_ + CURRENT_SENSOR_BIAS_A,
                                       0.0,
                                       maxCurrentA_);
        powerKW_ = nominalVoltageV_ * measuredCurrentA_ / 1000.0;
    } else {
        actualCurrentA_ = 0.0;
        measuredCurrentA_ = 0.0;
        powerKW_ = 0.0;
    }

    publishRegisters();
}

double EVCharger::nominalVoltageV() const { return nominalVoltageV_; }
double EVCharger::currentLimitA() const { return currentLimitA_; }
double EVCharger::measuredCurrentA() const { return measuredCurrentA_; }
double EVCharger::powerKW() const { return powerKW_; }
double EVCharger::timeConstantSeconds() const { return timeConstantSeconds_; }
const RegisterMap& EVCharger::registers() const { return registers_; }

void EVCharger::publishRegisters() {
    registers_.write(REG_STATUS, static_cast<std::int32_t>(status_));
    registers_.write(REG_CURRENT_X10, static_cast<std::int32_t>(std::lround(measuredCurrentA_ * 10.0)));
    registers_.write(REG_POWER_W, static_cast<std::int32_t>(std::lround(powerKW_ * 1000.0)));
    registers_.write(REG_LIMIT_X10, static_cast<std::int32_t>(std::lround(currentLimitA_ * 10.0)));
}

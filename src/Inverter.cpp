#include "Inverter.h"
#include <algorithm>
#include <array>
#include <cmath>

namespace {
constexpr std::uint16_t REG_STATUS = 100;
constexpr std::uint16_t REG_VOLTAGE_X10 = 101;
constexpr std::uint16_t REG_POWER_W = 102;
constexpr std::uint16_t REG_EXPORT_LIMIT_W = 103;

constexpr std::array<double, 8> MEASUREMENT_NOISE_KW{
    -0.010, 0.004, -0.006, 0.008, -0.003, 0.006, -0.008, 0.002
};
}

Inverter::Inverter(const std::string& deviceId, double ratedPowerKW)
    : DerDevice(deviceId), ratedPowerKW_(ratedPowerKW) {
    publishRegisters();
}

bool Inverter::setExportLimitKW(double limitKW) {
    if (!communicationAvailable_ || limitKW < 0.0 || limitKW > ratedPowerKW_) {
        return false;
    }
    exportLimitKW_ = limitKW;
    publishRegisters();
    return true;
}

void Inverter::setGridVoltageV(double voltageV) {
    voltageV_ = voltageV;
    publishRegisters();
}

void Inverter::setDemoOverVoltageThresholdV(double thresholdV) {
    if (thresholdV > 0.0) {
        demoOverVoltageThresholdV_ = thresholdV;
    }
}

void Inverter::setTimeConstantSeconds(double tauSeconds) {
    if (tauSeconds > 0.0) {
        timeConstantSeconds_ = tauSeconds;
    }
}

bool Inverter::clearFault() {
    if (!communicationAvailable_ || voltageV_ > demoOverVoltageThresholdV_) {
        return false;
    }
    overVoltageFault_ = false;
    status_ = Status::Stopped;
    actualPowerKW_ = 0.0;
    powerKW_ = 0.0;
    publishRegisters();
    return true;
}

double Inverter::ratedPowerKW() const { return ratedPowerKW_; }
double Inverter::exportLimitKW() const { return exportLimitKW_; }
double Inverter::powerKW() const { return powerKW_; }
double Inverter::voltageV() const { return voltageV_; }
double Inverter::timeConstantSeconds() const { return timeConstantSeconds_; }
bool Inverter::overVoltageFault() const { return overVoltageFault_; }
const RegisterMap& Inverter::registers() const { return registers_; }

void Inverter::simulateTelemetry() {
    simulateStep(1.0);
}

void Inverter::simulateStep(double dtSeconds) {
    if (dtSeconds <= 0.0) {
        return;
    }

    if (!communicationAvailable_) {
        status_ = Status::CommunicationLost;
        publishRegisters();
        return;
    }

    overVoltageFault_ = voltageV_ > demoOverVoltageThresholdV_;
    if (overVoltageFault_) {
        status_ = Status::Faulted;
        actualPowerKW_ = 0.0;
        powerKW_ = 0.0;
        publishRegisters();
        return;
    }

    if (status_ == Status::Running) {
        // First-order response: dP/dt = (P_command - P) / tau.
        // The exact discrete update below is stable for any positive dt.
        const double alpha = 1.0 - std::exp(-dtSeconds / timeConstantSeconds_);
        actualPowerKW_ += alpha * (exportLimitKW_ - actualPowerKW_);

        // Small deterministic sensor noise keeps the model repeatable for testing.
        powerKW_ = std::clamp(actualPowerKW_ + nextMeasurementNoiseKW(),
                              0.0,
                              ratedPowerKW_);
    } else {
        actualPowerKW_ = 0.0;
        powerKW_ = 0.0;
    }

    publishRegisters();
}

double Inverter::nextMeasurementNoiseKW() {
    const double noise = MEASUREMENT_NOISE_KW[sampleIndex_ % MEASUREMENT_NOISE_KW.size()];
    ++sampleIndex_;
    return noise;
}

void Inverter::publishRegisters() {
    registers_.write(REG_STATUS, static_cast<std::int32_t>(status_));
    registers_.write(REG_VOLTAGE_X10, static_cast<std::int32_t>(std::lround(voltageV_ * 10.0)));
    registers_.write(REG_POWER_W, static_cast<std::int32_t>(std::lround(powerKW_ * 1000.0)));
    registers_.write(REG_EXPORT_LIMIT_W, static_cast<std::int32_t>(std::lround(exportLimitKW_ * 1000.0)));
}

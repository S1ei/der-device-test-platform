#include "Battery.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr std::uint16_t REG_STATUS = 200;
constexpr std::uint16_t REG_SOC_X10 = 201;
constexpr std::uint16_t REG_POWER_W = 202;
constexpr std::uint16_t REG_SETPOINT_W = 203;
}

Battery::Battery(const std::string& deviceId,
                 double capacityKWh,
                 double maxChargeKW,
                 double maxDischargeKW,
                 double initialSocPercent,
                 double chargeEfficiency,
                 double dischargeEfficiency)
    : DerDevice(deviceId),
      capacityKWh_(capacityKWh),
      maxChargeKW_(maxChargeKW),
      maxDischargeKW_(maxDischargeKW),
      socPercent_(std::clamp(initialSocPercent, 0.0, 100.0)),
      chargeEfficiency_(std::clamp(chargeEfficiency, 0.01, 1.0)),
      dischargeEfficiency_(std::clamp(dischargeEfficiency, 0.01, 1.0)) {
    publishRegisters();
}

bool Battery::setPowerSetpointKW(double setpointKW) {
    if (!communicationAvailable_) {
        return false;
    }
    if (setpointKW < -maxChargeKW_ || setpointKW > maxDischargeKW_) {
        return false;
    }
    // Positive power = discharge. Negative power = charge.
    if ((socPercent_ <= 5.0 && setpointKW > 0.0) ||
        (socPercent_ >= 95.0 && setpointKW < 0.0)) {
        return false;
    }
    powerSetpointKW_ = setpointKW;
    publishRegisters();
    return true;
}

void Battery::simulateTelemetry() {
    simulateStep(60.0); // one minute convenience step
}

void Battery::simulateStep(double dtSeconds) {
    if (dtSeconds <= 0.0) {
        return;
    }

    if (!communicationAvailable_) {
        status_ = Status::CommunicationLost;
        publishRegisters();
        return;
    }

    if (status_ != Status::Running) {
        powerKW_ = 0.0;
        publishRegisters();
        return;
    }

    powerKW_ = powerSetpointKW_;
    const double dtHours = dtSeconds / 3600.0;

    if (powerKW_ > 0.0) {
        // To deliver P to the load, the battery must lose slightly more energy
        // because discharge efficiency is less than 100%.
        const double batteryEnergyRemovedKWh =
            (powerKW_ / dischargeEfficiency_) * dtHours;
        socPercent_ -= (batteryEnergyRemovedKWh / capacityKWh_) * 100.0;
    } else if (powerKW_ < 0.0) {
        // Charging input is negative by convention. Only a fraction of the
        // incoming energy is stored in the battery.
        const double storedEnergyKWh =
            (-powerKW_) * chargeEfficiency_ * dtHours;
        socPercent_ += (storedEnergyKWh / capacityKWh_) * 100.0;
    }

    socPercent_ = std::clamp(socPercent_, 0.0, 100.0);
    publishRegisters();
}

double Battery::capacityKWh() const { return capacityKWh_; }
double Battery::socPercent() const { return socPercent_; }
double Battery::powerKW() const { return powerKW_; }
double Battery::powerSetpointKW() const { return powerSetpointKW_; }
double Battery::chargeEfficiency() const { return chargeEfficiency_; }
double Battery::dischargeEfficiency() const { return dischargeEfficiency_; }
const RegisterMap& Battery::registers() const { return registers_; }

void Battery::publishRegisters() {
    registers_.write(REG_STATUS, static_cast<std::int32_t>(status_));
    registers_.write(REG_SOC_X10, static_cast<std::int32_t>(std::lround(socPercent_ * 10.0)));
    registers_.write(REG_POWER_W, static_cast<std::int32_t>(std::lround(powerKW_ * 1000.0)));
    registers_.write(REG_SETPOINT_W, static_cast<std::int32_t>(std::lround(powerSetpointKW_ * 1000.0)));
}

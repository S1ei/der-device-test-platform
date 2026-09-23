#pragma once

#include "DerDevice.h"
#include "RegisterMap.h"

class Battery : public DerDevice {
public:
    Battery(const std::string& deviceId,
            double capacityKWh,
            double maxChargeKW,
            double maxDischargeKW,
            double initialSocPercent = 50.0,
            double chargeEfficiency = 0.95,
            double dischargeEfficiency = 0.96);

    bool setPowerSetpointKW(double setpointKW);
    void simulateTelemetry() override;
    void simulateStep(double dtSeconds);

    double capacityKWh() const;
    double socPercent() const;
    double powerKW() const;
    double powerSetpointKW() const;
    double chargeEfficiency() const;
    double dischargeEfficiency() const;
    const RegisterMap& registers() const;

private:
    void publishRegisters();

    double capacityKWh_;
    double maxChargeKW_;
    double maxDischargeKW_;
    double socPercent_;
    double chargeEfficiency_;
    double dischargeEfficiency_;
    double powerSetpointKW_{0.0};
    double powerKW_{0.0};
    RegisterMap registers_;
};

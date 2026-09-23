#pragma once

#include "DerDevice.h"
#include "RegisterMap.h"

class EVCharger : public DerDevice {
public:
    EVCharger(const std::string& deviceId,
              double nominalVoltageV,
              double maxCurrentA);

    bool setCurrentLimitA(double currentA);
    void setTimeConstantSeconds(double tauSeconds);
    void simulateTelemetry() override;
    void simulateStep(double dtSeconds);

    double nominalVoltageV() const;
    double currentLimitA() const;
    double measuredCurrentA() const;
    double powerKW() const;
    double timeConstantSeconds() const;
    const RegisterMap& registers() const;

private:
    void publishRegisters();

    double nominalVoltageV_;
    double maxCurrentA_;
    double currentLimitA_{0.0};
    double actualCurrentA_{0.0};
    double measuredCurrentA_{0.0};
    double powerKW_{0.0};
    double timeConstantSeconds_{1.5};
    RegisterMap registers_;
};

#pragma once

#include "DerDevice.h"
#include "RegisterMap.h"

class Inverter : public DerDevice {
public:
    Inverter(const std::string& deviceId, double ratedPowerKW);

    bool setExportLimitKW(double limitKW);
    void setGridVoltageV(double voltageV);
    void setDemoOverVoltageThresholdV(double thresholdV);
    void setTimeConstantSeconds(double tauSeconds);
    bool clearFault();

    double ratedPowerKW() const;
    double exportLimitKW() const;
    double powerKW() const;
    double voltageV() const;
    double timeConstantSeconds() const;
    bool overVoltageFault() const;

    const RegisterMap& registers() const;
    void simulateTelemetry() override;
    void simulateStep(double dtSeconds);

private:
    void publishRegisters();
    double nextMeasurementNoiseKW();

    double ratedPowerKW_;
    double exportLimitKW_{0.0};
    double actualPowerKW_{0.0};
    double powerKW_{0.0};
    double voltageV_{230.0};
    double demoOverVoltageThresholdV_{253.0};
    double timeConstantSeconds_{3.0};
    bool overVoltageFault_{false};
    std::size_t sampleIndex_{0};
    RegisterMap registers_;
};

#include "Battery.h"
#include "CsvReporter.h"
#include "EVCharger.h"
#include "Inverter.h"
#include "TestFramework.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {

TestResult makeResult(const std::string& device,
                      const std::string& testName,
                      const std::string& expected,
                      const std::string& actual,
                      bool passed) {
    return {device, testName, expected, actual, passed};
}

void runInverterTests(TestSuite& suite) {
    Inverter inverter("SIM-INV-001", 10.0);
    inverter.start();

    const bool accepted = inverter.setExportLimitKW(5.0);
    inverter.simulateStep(1.0);
    const bool transientPass = accepted && inverter.powerKW() > 0.0 && inverter.powerKW() < 5.0;
    suite.add(makeResult(
        inverter.deviceId(),
        "First-order transient response",
        "Power rises toward 5.00 kW without an instantaneous jump",
        formatDouble(inverter.powerKW()) + " kW after 1.0 s",
        transientPass));

    for (int i = 0; i < 20; ++i) {
        inverter.simulateStep(1.0);
    }
    const bool trackingPass = withinTolerance(inverter.powerKW(), 5.0, 0.10);
    suite.add(makeResult(
        inverter.deviceId(),
        "Settled export-limit tracking",
        "5.00 kW +/- 0.10 kW after settling",
        formatDouble(inverter.powerKW()) + " kW",
        trackingPass));

    const bool rejected = !inverter.setExportLimitKW(12.0);
    suite.add(makeResult(
        inverter.deviceId(),
        "Reject export limit above device rating",
        "Command rejected (> 10.00 kW rating)",
        rejected ? "Rejected" : "Accepted",
        rejected));

    inverter.setGridVoltageV(255.0);
    inverter.simulateStep(0.5);
    const bool faultPass = inverter.overVoltageFault() &&
                           inverter.status() == DerDevice::Status::Faulted &&
                           withinTolerance(inverter.powerKW(), 0.0, 0.001);
    suite.add(makeResult(
        inverter.deviceId(),
        "Demo over-voltage fault response",
        "FAULTED and 0.00 kW above configured demo threshold",
        DerDevice::statusToString(inverter.status()) + ", " +
            formatDouble(inverter.powerKW()) + " kW",
        faultPass));

    const bool cannotClearWhileHigh = !inverter.clearFault();
    inverter.setGridVoltageV(230.0);
    const bool cleared = inverter.clearFault();
    inverter.start();
    for (int i = 0; i < 20; ++i) {
        inverter.simulateStep(1.0);
    }
    const bool recoveryPass = cannotClearWhileHigh && cleared &&
                              inverter.status() == DerDevice::Status::Running &&
                              withinTolerance(inverter.powerKW(), 5.0, 0.10);
    suite.add(makeResult(
        inverter.deviceId(),
        "Fault reset and recovery",
        "Fault cannot clear at high voltage; recovers after voltage returns normal",
        std::string(cannotClearWhileHigh ? "Blocked high-V reset; " : "Unexpected reset; ") +
            DerDevice::statusToString(inverter.status()) + ", " +
            formatDouble(inverter.powerKW()) + " kW",
        recoveryPass));

    const auto powerRegister = inverter.registers().read(102);
    const bool registerPass = powerRegister.has_value() &&
                              std::abs(*powerRegister - 5000) <= 100;
    suite.add(makeResult(
        inverter.deviceId(),
        "Telemetry published to Modbus-style register map",
        "Register 102 approximately 5000 W after settling",
        powerRegister ? std::to_string(*powerRegister) + " W" : "Register missing",
        registerPass));

    inverter.setCommunicationAvailable(false);
    const bool commandRejectedDuringCommsLoss = !inverter.setExportLimitKW(3.0);
    inverter.simulateStep(1.0);
    const bool commsPass = commandRejectedDuringCommsLoss &&
                           inverter.status() == DerDevice::Status::CommunicationLost;
    suite.add(makeResult(
        inverter.deviceId(),
        "Communication loss handling",
        "Command rejected and status COMM_LOST",
        std::string(commandRejectedDuringCommsLoss ? "Rejected, " : "Accepted, ") +
            DerDevice::statusToString(inverter.status()),
        commsPass));
}

void runBatteryTests(TestSuite& suite) {
    Battery dischargeBattery("SIM-BAT-001", 13.5, 5.0, 5.0, 50.0);
    dischargeBattery.start();
    const bool dischargeAccepted = dischargeBattery.setPowerSetpointKW(3.0);
    dischargeBattery.simulateStep(600.0); // 10 minutes

    const double expectedDischargeSoc =
        50.0 - (((3.0 / dischargeBattery.dischargeEfficiency()) * (600.0 / 3600.0)) /
                dischargeBattery.capacityKWh()) * 100.0;
    const bool dischargePass = dischargeAccepted &&
                               withinTolerance(dischargeBattery.socPercent(), expectedDischargeSoc, 0.02);
    suite.add(makeResult(
        dischargeBattery.deviceId(),
        "Discharge energy and efficiency model",
        "SOC follows E = P*t with discharge efficiency",
        "SOC " + formatDouble(dischargeBattery.socPercent()) + "% (expected " +
            formatDouble(expectedDischargeSoc) + "%)",
        dischargePass));

    Battery chargeBattery("SIM-BAT-002", 13.5, 5.0, 5.0, 50.0);
    chargeBattery.start();
    const bool chargeAccepted = chargeBattery.setPowerSetpointKW(-3.0);
    chargeBattery.simulateStep(600.0);

    const double expectedChargeSoc =
        50.0 + (((3.0 * chargeBattery.chargeEfficiency()) * (600.0 / 3600.0)) /
                chargeBattery.capacityKWh()) * 100.0;
    const bool chargePass = chargeAccepted &&
                            withinTolerance(chargeBattery.socPercent(), expectedChargeSoc, 0.02);
    suite.add(makeResult(
        chargeBattery.deviceId(),
        "Charging energy and efficiency model",
        "SOC increases using charging efficiency",
        "SOC " + formatDouble(chargeBattery.socPercent()) + "% (expected " +
            formatDouble(expectedChargeSoc) + "%)",
        chargePass));

    const bool badSetpointRejected = !chargeBattery.setPowerSetpointKW(8.0);
    suite.add(makeResult(
        chargeBattery.deviceId(),
        "Reject battery power above rating",
        "8.00 kW command rejected (> 5.00 kW max discharge)",
        badSetpointRejected ? "Rejected" : "Accepted",
        badSetpointRejected));

    Battery lowSocBattery("SIM-BAT-LOW", 13.5, 5.0, 5.0, 4.0);
    lowSocBattery.start();
    const bool lowSocProtection = !lowSocBattery.setPowerSetpointKW(1.0);
    suite.add(makeResult(
        lowSocBattery.deviceId(),
        "Low-SOC discharge protection",
        "Discharge command rejected at <= 5% SOC",
        lowSocProtection ? "Rejected at 4.00% SOC" : "Accepted unexpectedly",
        lowSocProtection));
}

void runEvChargerTests(TestSuite& suite) {
    EVCharger charger("SIM-EVSE-001", 230.0, 32.0);
    charger.start();

    const bool currentAccepted = charger.setCurrentLimitA(16.0);
    charger.simulateStep(0.5);
    const bool rampPass = currentAccepted &&
                          charger.measuredCurrentA() > 0.0 &&
                          charger.measuredCurrentA() < 16.0;
    suite.add(makeResult(
        charger.deviceId(),
        "Current ramp transient",
        "Current approaches 16 A gradually rather than instantly",
        formatDouble(charger.measuredCurrentA()) + " A after 0.5 s",
        rampPass));

    for (int i = 0; i < 20; ++i) {
        charger.simulateStep(0.5);
    }
    const bool currentPass = withinTolerance(charger.measuredCurrentA(), 16.0, 0.20);
    suite.add(makeResult(
        charger.deviceId(),
        "Settled current-limit tracking",
        "16.00 A +/- 0.20 A",
        formatDouble(charger.measuredCurrentA()) + " A, " +
            formatDouble(charger.powerKW()) + " kW",
        currentPass));

    const bool badLimitRejected = !charger.setCurrentLimitA(40.0);
    suite.add(makeResult(
        charger.deviceId(),
        "Reject EV charger current above rating",
        "40.00 A command rejected (> 32.00 A rating)",
        badLimitRejected ? "Rejected" : "Accepted",
        badLimitRejected));
}

bool writeInverterStepResponse(const std::string& path) {
    const std::filesystem::path filePath(path);
    if (filePath.has_parent_path()) {
        std::filesystem::create_directories(filePath.parent_path());
    }

    std::ofstream out(path);
    if (!out) {
        return false;
    }

    Inverter inverter("SIM-INV-PLOT", 10.0);
    inverter.start();
    inverter.setExportLimitKW(5.0);

    out << "time_s,command_kw,measured_power_kw,voltage_v,status\n";
    out << "0,5.0,0.0," << inverter.voltageV() << ","
        << DerDevice::statusToString(inverter.status()) << "\n";

    for (int t = 1; t <= 20; ++t) {
        inverter.simulateStep(1.0);
        out << t << ','
            << inverter.exportLimitKW() << ','
            << inverter.powerKW() << ','
            << inverter.voltageV() << ','
            << DerDevice::statusToString(inverter.status()) << '\n';
    }
    return true;
}

} // namespace

int main(int argc, char* argv[]) {
    std::string reportPath = "reports/test_results.csv";
    if (argc == 3 && std::string(argv[1]) == "--report") {
        reportPath = argv[2];
    }

    std::cout << "===================================================\n";
    std::cout << " DER Device Test Platform v2 (C++17)\n";
    std::cout << "===================================================\n";
    std::cout << "Educational simulator with dynamic device models.\n";
    std::cout << "It does NOT claim certification or conformance with a real standard.\n";

    TestSuite suite;
    runInverterTests(suite);
    runBatteryTests(suite);
    runEvChargerTests(suite);
    suite.printSummary();

    const std::filesystem::path reportFile(reportPath);
    if (reportFile.has_parent_path()) {
        std::filesystem::create_directories(reportFile.parent_path());
    }

    if (CsvReporter::write(reportPath, suite)) {
        std::cout << "CSV test report written to: " << reportPath << "\n";
    } else {
        std::cerr << "Warning: could not write CSV report to " << reportPath << "\n";
    }

    const std::string responsePath = "reports/inverter_step_response.csv";
    if (writeInverterStepResponse(responsePath)) {
        std::cout << "Inverter step-response data written to: " << responsePath << "\n";
    }

    return suite.allPassed() ? 0 : 1;
}

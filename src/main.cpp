// ==========================================================
// HEADER FILES FROM OUR OWN PROJECT
// ==========================================================
//
// These files tell main.cpp about the classes/functions
// that were created elsewhere in the project.
//

#include "Battery.h"        // Gives access to the Battery class
#include "CsvReporter.h"    // Lets us save test results into a CSV file
#include "EVCharger.h"      // Gives access to the EVCharger class
#include "Inverter.h"       // Gives access to the Inverter class
#include "TestFramework.h"  // Gives access to TestSuite, TestResult, etc.


// ==========================================================
// STANDARD C++ LIBRARIES
// ==========================================================

#include <filesystem>  // Used for folders and file paths
#include <fstream>     // Used for writing files
#include <iostream>    // Used for cout and cerr
#include <string>      // Used for text variables


// ==========================================================
// ANONYMOUS NAMESPACE
// ==========================================================
//
// The functions inside this namespace are only meant to be
// used inside this main.cpp file.
//
// They are helper functions for this program.
//

namespace {


// ==========================================================
// HELPER FUNCTION: makeResult()
// ==========================================================
//
// This function creates one TestResult.
//
// Instead of manually building a TestResult every time,
// we give this function:
//
// 1. device name
// 2. test name
// 3. expected result
// 4. actual result
// 5. whether the test passed
//
// It then returns a TestResult object.
//

TestResult makeResult(
    const std::string& device,
    const std::string& testName,
    const std::string& expected,
    const std::string& actual,
    bool passed)
{
    return {
        device,
        testName,
        expected,
        actual,
        passed
    };
}


// ==========================================================
// INVERTER TESTS
// ==========================================================
//
// This function runs all tests relating to the
// simulated inverter.
//
// "&suite" means we are using the original TestSuite object
// created in main(), rather than making a copy.
//

void runInverterTests(TestSuite& suite)
{
    // ------------------------------------------------------
    // CREATE THE INVERTER
    // ------------------------------------------------------
    //
    // "Inverter" = class/type
    // "inverter" = object
    //
    // SIM-INV-001 is the simulated device ID.
    //
    // 10.0 means this inverter has a 10 kW rated power.
    //
    Inverter inverter("SIM-INV-001", 10.0);


    // Start the inverter.
    //
    // The inverter should now enter its normal running state.
    inverter.start();


    // ======================================================
    // TEST 1: FIRST-ORDER TRANSIENT RESPONSE
    // ======================================================

    // Ask the inverter to use a 5 kW export limit.
    //
    // setExportLimitKW() returns true if the command
    // was accepted.
    //
    const bool accepted =
        inverter.setExportLimitKW(5.0);


    // Advance the inverter simulation by 1 second.
    //
    // Because the inverter uses a first-order response,
    // the output should move toward 5 kW but should NOT
    // instantly jump to exactly 5 kW.
    //
    inverter.simulateStep(1.0);


    // The test passes if:
    //
    // 1. The 5 kW command was accepted
    // 2. Power has increased above 0 kW
    // 3. Power is still below 5 kW
    //
    // && means AND.
    //
    const bool transientPass =
        accepted &&
        inverter.powerKW() > 0.0 &&
        inverter.powerKW() < 5.0;


    // Add the result of this test into the TestSuite.
    suite.add(
        makeResult(
            inverter.deviceId(),

            "First-order transient response",

            "Power rises toward 5.00 kW without an instantaneous jump",

            formatDouble(inverter.powerKW()) +
                " kW after 1.0 s",

            transientPass
        )
    );


    // ======================================================
    // TEST 2: SETTLED EXPORT LIMIT
    // ======================================================

    // Run another 20 simulation steps.
    //
    // i starts at 0.
    // i < 20 means repeat while i is below 20.
    // ++i increases i by 1 each time.
    //
    // simulateStep(1.0) means each loop represents 1 second.
    //
    for (int i = 0; i < 20; ++i)
    {
        inverter.simulateStep(1.0);
    }


    // Now the inverter should have settled very close to 5 kW.
    //
    // withinTolerance(actual, target, tolerance)
    //
    // Here:
    //
    // actual     = inverter power
    // target     = 5.0 kW
    // tolerance  = 0.10 kW
    //
    // So acceptable range is approximately:
    //
    // 4.90 kW to 5.10 kW
    //
    const bool trackingPass =
        withinTolerance(
            inverter.powerKW(),
            5.0,
            0.10
        );


    suite.add(
        makeResult(
            inverter.deviceId(),

            "Settled export-limit tracking",

            "5.00 kW +/- 0.10 kW after settling",

            formatDouble(inverter.powerKW()) + " kW",

            trackingPass
        )
    );


    // ======================================================
    // TEST 3: REJECT INVALID EXPORT LIMIT
    // ======================================================

    // The inverter is rated at 10 kW.
    //
    // Here we deliberately try to set an export limit
    // of 12 kW.
    //
    // That should be rejected.
    //
    // setExportLimitKW() should therefore return false.
    //
    // The ! symbol means NOT.
    //
    // So:
    //
    // !false = true
    //
    // Therefore "rejected" becomes true when the inverter
    // correctly refuses the invalid command.
    //
    const bool rejected =
        !inverter.setExportLimitKW(12.0);


    suite.add(
        makeResult(
            inverter.deviceId(),

            "Reject export limit above device rating",

            "Command rejected (> 10.00 kW rating)",

            rejected ? "Rejected" : "Accepted",

            rejected
        )
    );


    // ======================================================
    // TEST 4: OVER-VOLTAGE FAULT
    // ======================================================

    // Simulate the grid voltage increasing to 255 V.
    inverter.setGridVoltageV(255.0);


    // Advance the simulation by 0.5 seconds.
    //
    // During this update, the inverter should detect
    // the over-voltage condition.
    //
    inverter.simulateStep(0.5);


    // The fault test passes if ALL of these are true:
    //
    // 1. overVoltageFault() is true
    // 2. device state is Faulted
    // 3. output power has fallen to approximately 0 kW
    //
    const bool faultPass =
        inverter.overVoltageFault() &&

        inverter.status() ==
            DerDevice::Status::Faulted &&

        withinTolerance(
            inverter.powerKW(),
            0.0,
            0.001
        );


    suite.add(
        makeResult(
            inverter.deviceId(),

            "Demo over-voltage fault response",

            "FAULTED and 0.00 kW above configured demo threshold",

            DerDevice::statusToString(
                inverter.status()
            )
            + ", "
            + formatDouble(
                inverter.powerKW()
            )
            + " kW",

            faultPass
        )
    );


    // ======================================================
    // TEST 5: FAULT RESET AND RECOVERY
    // ======================================================

    // Try to clear the fault while voltage is still too high.
    //
    // This SHOULD fail.
    //
    // clearFault() should return false.
    //
    const bool cannotClearWhileHigh =
        !inverter.clearFault();


    // Return the grid to normal voltage.
    inverter.setGridVoltageV(230.0);


    // Now try to clear the fault again.
    //
    // This time it should succeed.
    //
    const bool cleared =
        inverter.clearFault();


    // Restart the inverter.
    inverter.start();


    // Let the inverter settle back toward the 5 kW
    // export limit.
    //
    for (int i = 0; i < 20; ++i)
    {
        inverter.simulateStep(1.0);
    }


    // Recovery passes if:
    //
    // 1. Reset was blocked while voltage was high
    // 2. Reset succeeded after voltage returned to normal
    // 3. Device is now Running
    // 4. Power has returned close to 5 kW
    //
    const bool recoveryPass =
        cannotClearWhileHigh &&
        cleared &&

        inverter.status() ==
            DerDevice::Status::Running &&

        withinTolerance(
            inverter.powerKW(),
            5.0,
            0.10
        );


    suite.add(
        makeResult(
            inverter.deviceId(),

            "Fault reset and recovery",

            "Fault cannot clear at high voltage; "
            "recovers after voltage returns normal",

            std::string(
                cannotClearWhileHigh
                    ? "Blocked high-V reset; "
                    : "Unexpected reset; "
            )
            +
            DerDevice::statusToString(
                inverter.status()
            )
            + ", "
            +
            formatDouble(
                inverter.powerKW()
            )
            + " kW",

            recoveryPass
        )
    );


    // ======================================================
    // TEST 6: REGISTER MAP TELEMETRY
    // ======================================================

    // Read register number 102 from the inverter's
    // simulated register map.
    //
    // This register represents inverter power in watts.
    //
    const auto powerRegister =
        inverter.registers().read(102);


    // has_value() checks whether the register actually existed.
    //
    // *powerRegister gets the value stored inside it.
    //
    // The expected result is approximately 5000 W.
    //
    const bool registerPass =
        powerRegister.has_value() &&

        std::abs(
            *powerRegister - 5000
        ) <= 100;


    suite.add(
        makeResult(
            inverter.deviceId(),

            "Telemetry published to Modbus-style register map",

            "Register 102 approximately 5000 W after settling",

            powerRegister
                ? std::to_string(*powerRegister) + " W"
                : "Register missing",

            registerPass
        )
    );


    // ======================================================
    // TEST 7: COMMUNICATION LOSS
    // ======================================================

    // Simulate loss of communication with the inverter.
    inverter.setCommunicationAvailable(false);


    // Try to send a new 3 kW export command.
    //
    // The inverter should reject commands while
    // communication is unavailable.
    //
    const bool commandRejectedDuringCommsLoss =
        !inverter.setExportLimitKW(3.0);


    // Advance the simulation by 1 second.
    inverter.simulateStep(1.0);


    // Pass if:
    //
    // 1. the new command was rejected
    // 2. inverter state became CommunicationLost
    //
    const bool commsPass =
        commandRejectedDuringCommsLoss &&

        inverter.status() ==
            DerDevice::Status::CommunicationLost;


    suite.add(
        makeResult(
            inverter.deviceId(),

            "Communication loss handling",

            "Command rejected and status COMM_LOST",

            std::string(
                commandRejectedDuringCommsLoss
                    ? "Rejected, "
                    : "Accepted, "
            )
            +
            DerDevice::statusToString(
                inverter.status()
            ),

            commsPass
        )
    );
}


// ==========================================================
// BATTERY TESTS
// ==========================================================

void runBatteryTests(TestSuite& suite)
{
    // ======================================================
    // TEST 1: BATTERY DISCHARGE
    // ======================================================

    // Create a simulated battery.
    //
    // The exact meaning of these constructor values is
    // defined inside Battery.h / Battery.cpp.
    //
    // From how the object is used here:
    //
    // 13.5 = battery capacity in kWh
    // 5.0  = maximum discharge power
    // 5.0  = maximum charge power
    // 50.0 = starting state of charge (%)
    //
    Battery dischargeBattery(
        "SIM-BAT-001",
        13.5,
        5.0,
        5.0,
        50.0
    );


    // Start the battery.
    dischargeBattery.start();


    // Ask the battery to discharge at 3 kW.
    //
    // Positive power means discharge.
    //
    const bool dischargeAccepted =
        dischargeBattery.setPowerSetpointKW(3.0);


    // Simulate 600 seconds.
    //
    // 600 seconds = 10 minutes.
    //
    dischargeBattery.simulateStep(600.0);


    // ------------------------------------------------------
    // EXPECTED STATE OF CHARGE
    // ------------------------------------------------------
    //
    // This calculation uses:
    //
    // Energy = Power x Time
    //
    // Time must be converted:
    //
    // 600 seconds / 3600
    // = 0.1667 hours
    //
    // The discharge efficiency is also included.
    //
    const double expectedDischargeSoc =
        50.0 -
        (
            (
                (3.0 /
                 dischargeBattery.dischargeEfficiency())
                *
                (600.0 / 3600.0)
            )
            /
            dischargeBattery.capacityKWh()
        )
        * 100.0;


    // Compare the simulated SOC with the calculated SOC.
    const bool dischargePass =
        dischargeAccepted &&

        withinTolerance(
            dischargeBattery.socPercent(),
            expectedDischargeSoc,
            0.02
        );


    suite.add(
        makeResult(
            dischargeBattery.deviceId(),

            "Discharge energy and efficiency model",

            "SOC follows E = P*t with discharge efficiency",

            "SOC "
            + formatDouble(
                dischargeBattery.socPercent()
            )
            + "% (expected "
            + formatDouble(
                expectedDischargeSoc
            )
            + "%)",

            dischargePass
        )
    );


    // ======================================================
    // TEST 2: BATTERY CHARGING
    // ======================================================

    // Create another battery starting at 50% SOC.
    Battery chargeBattery(
        "SIM-BAT-002",
        13.5,
        5.0,
        5.0,
        50.0
    );


    chargeBattery.start();


    // Negative power represents charging.
    //
    // -3.0 kW means charge at 3 kW.
    //
    const bool chargeAccepted =
        chargeBattery.setPowerSetpointKW(-3.0);


    // Simulate charging for 10 minutes.
    chargeBattery.simulateStep(600.0);


    // Calculate what the battery SOC should become.
    //
    // Again:
    //
    // E = P * t
    //
    // but this time charging efficiency is used.
    //
    const double expectedChargeSoc =
        50.0 +
        (
            (
                (
                    3.0 *
                    chargeBattery.chargeEfficiency()
                )
                *
                (600.0 / 3600.0)
            )
            /
            chargeBattery.capacityKWh()
        )
        * 100.0;


    const bool chargePass =
        chargeAccepted &&

        withinTolerance(
            chargeBattery.socPercent(),
            expectedChargeSoc,
            0.02
        );


    suite.add(
        makeResult(
            chargeBattery.deviceId(),

            "Charging energy and efficiency model",

            "SOC increases using charging efficiency",

            "SOC "
            + formatDouble(
                chargeBattery.socPercent()
            )
            + "% (expected "
            + formatDouble(
                expectedChargeSoc
            )
            + "%)",

            chargePass
        )
    );


    // ======================================================
    // TEST 3: INVALID BATTERY POWER
    // ======================================================

    // The battery's maximum discharge power is 5 kW.
    //
    // Try asking for 8 kW.
    //
    // This should be rejected.
    //
    const bool badSetpointRejected =
        !chargeBattery.setPowerSetpointKW(8.0);


    suite.add(
        makeResult(
            chargeBattery.deviceId(),

            "Reject battery power above rating",

            "8.00 kW command rejected (> 5.00 kW max discharge)",

            badSetpointRejected
                ? "Rejected"
                : "Accepted",

            badSetpointRejected
        )
    );


    // ======================================================
    // TEST 4: LOW-SOC PROTECTION
    // ======================================================

    // Create a battery starting at only 4% SOC.
    Battery lowSocBattery(
        "SIM-BAT-LOW",
        13.5,
        5.0,
        5.0,
        4.0
    );


    lowSocBattery.start();


    // Try to discharge at 1 kW.
    //
    // Because SOC is already <= 5%,
    // the battery should reject this.
    //
    const bool lowSocProtection =
        !lowSocBattery.setPowerSetpointKW(1.0);


    suite.add(
        makeResult(
            lowSocBattery.deviceId(),

            "Low-SOC discharge protection",

            "Discharge command rejected at <= 5% SOC",

            lowSocProtection
                ? "Rejected at 4.00% SOC"
                : "Accepted unexpectedly",

            lowSocProtection
        )
    );
}


// ==========================================================
// EV CHARGER TESTS
// ==========================================================

void runEvChargerTests(TestSuite& suite)
{
    // Create an EV charger.
    //
    // SIM-EVSE-001 = device ID
    // 230.0        = supply voltage in volts
    // 32.0         = maximum charger current in amps
    //
    EVCharger charger(
        "SIM-EVSE-001",
        230.0,
        32.0
    );


    charger.start();


    // ======================================================
    // TEST 1: CURRENT RAMP
    // ======================================================

    // Request a 16 A charging current.
    const bool currentAccepted =
        charger.setCurrentLimitA(16.0);


    // Simulate 0.5 seconds.
    //
    // Current should begin moving toward 16 A
    // rather than instantly reaching it.
    //
    charger.simulateStep(0.5);


    const bool rampPass =
        currentAccepted &&

        charger.measuredCurrentA() > 0.0 &&

        charger.measuredCurrentA() < 16.0;


    suite.add(
        makeResult(
            charger.deviceId(),

            "Current ramp transient",

            "Current approaches 16 A gradually rather than instantly",

            formatDouble(
                charger.measuredCurrentA()
            )
            + " A after 0.5 s",

            rampPass
        )
    );


    // ======================================================
    // TEST 2: SETTLED CURRENT
    // ======================================================

    // Run another 20 simulation updates.
    //
    // Each update represents 0.5 seconds.
    //
    for (int i = 0; i < 20; ++i)
    {
        charger.simulateStep(0.5);
    }


    // Current should now be close to 16 A.
    const bool currentPass =
        withinTolerance(
            charger.measuredCurrentA(),
            16.0,
            0.20
        );


    suite.add(
        makeResult(
            charger.deviceId(),

            "Settled current-limit tracking",

            "16.00 A +/- 0.20 A",

            formatDouble(
                charger.measuredCurrentA()
            )
            + " A, "
            +
            formatDouble(
                charger.powerKW()
            )
            + " kW",

            currentPass
        )
    );


    // ======================================================
    // TEST 3: INVALID CURRENT LIMIT
    // ======================================================

    // Charger is rated for 32 A.
    //
    // Try requesting 40 A.
    //
    // It should be rejected.
    //
    const bool badLimitRejected =
        !charger.setCurrentLimitA(40.0);


    suite.add(
        makeResult(
            charger.deviceId(),

            "Reject EV charger current above rating",

            "40.00 A command rejected (> 32.00 A rating)",

            badLimitRejected
                ? "Rejected"
                : "Accepted",

            badLimitRejected
        )
    );
}


// ==========================================================
// INVERTER STEP-RESPONSE CSV GENERATOR
// ==========================================================
//
// This function runs a separate inverter simulation
// and saves its response to a CSV file.
//
// That CSV can later be opened in Excel or Python to plot:
//
// Power
//   ↑
// 5 |                    ______
//   |                ___/
//   |             __/
//   |          __/
//   |_______ _/________________→ Time
//

bool writeInverterStepResponse(
    const std::string& path)
{
    // Convert the text path into a filesystem path.
    const std::filesystem::path filePath(path);


    // If the file should be placed inside a folder,
    // make sure that folder exists.
    //
    if (filePath.has_parent_path())
    {
        std::filesystem::create_directories(
            filePath.parent_path()
        );
    }


    // Open the output CSV file.
    //
    // std::ofstream means:
    //
    // output file stream
    //
    // In simple terms:
    //
    // "Open a file so C++ can write into it."
    //
    std::ofstream out(path);


    // If the file could not be opened,
    // return false.
    //
    if (!out)
    {
        return false;
    }


    // Create another simulated inverter specifically
    // for generating plot data.
    Inverter inverter(
        "SIM-INV-PLOT",
        10.0
    );


    inverter.start();


    // Set the power target/export limit to 5 kW.
    inverter.setExportLimitKW(5.0);


    // ------------------------------------------------------
    // CSV COLUMN HEADINGS
    // ------------------------------------------------------
    //
    // This creates:
    //
    // time_s,
    // command_kw,
    // measured_power_kw,
    // voltage_v,
    // status
    //
    out
        << "time_s,"
        << "command_kw,"
        << "measured_power_kw,"
        << "voltage_v,"
        << "status\n";


    // Write the initial condition at t = 0.
    //
    // At the very beginning:
    //
    // command = 5 kW
    // measured power = 0 kW
    //
    out
        << "0,5.0,0.0,"
        << inverter.voltageV()
        << ","
        << DerDevice::statusToString(
               inverter.status()
           )
        << "\n";


    // Simulate from 1 second to 20 seconds.
    //
    for (int t = 1; t <= 20; ++t)
    {
        // Advance simulation by 1 second.
        inverter.simulateStep(1.0);


        // Save one row of data.
        out
            << t
            << ','
            << inverter.exportLimitKW()
            << ','
            << inverter.powerKW()
            << ','
            << inverter.voltageV()
            << ','
            << DerDevice::statusToString(
                   inverter.status()
               )
            << '\n';
    }


    // If everything worked, return true.
    return true;
}


// End of the anonymous namespace.
} // namespace


// ==========================================================
// MAIN PROGRAM
// ==========================================================
//
// This is where the program begins.
//
// When we run the executable,
// C++ starts executing from main().
//

int main(int argc, char* argv[])
{
    // ------------------------------------------------------
    // DEFAULT TEST REPORT LOCATION
    // ------------------------------------------------------

    std::string reportPath =
        "reports/test_results.csv";


    // ------------------------------------------------------
    // OPTIONAL COMMAND-LINE REPORT PATH
    // ------------------------------------------------------
    //
    // This allows someone to run:
    //
    // DER_Test.exe --report custom_results.csv
    //
    // argc = number of command-line items
    //
    // argv = the actual command-line items
    //
    if (
        argc == 3 &&
        std::string(argv[1]) == "--report"
    )
    {
        reportPath = argv[2];
    }


    // ------------------------------------------------------
    // PRINT PROGRAM TITLE
    // ------------------------------------------------------

    std::cout
        << "===================================================\n";

    std::cout
        << " DER Device Test Platform v2 (C++17)\n";

    std::cout
        << "===================================================\n";

    std::cout
        << "Educational simulator with dynamic device models.\n";

    std::cout
        << "It does NOT claim certification or conformance "
        << "with a real standard.\n";


    // ------------------------------------------------------
    // CREATE TEST SUITE
    // ------------------------------------------------------
    //
    // This object collects all PASS/FAIL test results.
    //
    TestSuite suite;


    // ------------------------------------------------------
    // RUN ALL DEVICE TESTS
    // ------------------------------------------------------

    // Test simulated inverter behaviour.
    runInverterTests(suite);

    // Test simulated battery behaviour.
    runBatteryTests(suite);

    // Test simulated EV charger behaviour.
    runEvChargerTests(suite);


    // Print all test results to the terminal.
    suite.printSummary();


    // ------------------------------------------------------
    // PREPARE CSV REPORT FOLDER
    // ------------------------------------------------------

    const std::filesystem::path reportFile(
        reportPath
    );


    // If a folder such as "reports" is required,
    // create it if it does not already exist.
    //
    if (reportFile.has_parent_path())
    {
        std::filesystem::create_directories(
            reportFile.parent_path()
        );
    }


    // ------------------------------------------------------
    // WRITE THE TEST RESULTS CSV
    // ------------------------------------------------------

    if (
        CsvReporter::write(
            reportPath,
            suite
        )
    )
    {
        std::cout
            << "CSV test report written to: "
            << reportPath
            << "\n";
    }
    else
    {
        std::cerr
            << "Warning: could not write CSV report to "
            << reportPath
            << "\n";
    }


    // ------------------------------------------------------
    // WRITE INVERTER STEP-RESPONSE CSV
    // ------------------------------------------------------

    const std::string responsePath =
        "reports/inverter_step_response.csv";


    if (
        writeInverterStepResponse(
            responsePath
        )
    )
    {
        std::cout
            << "Inverter step-response data written to: "
            << responsePath
            << "\n";
    }


    // ------------------------------------------------------
    // PROGRAM EXIT STATUS
    // ------------------------------------------------------
    //
    // The original code used:
    //
    // return suite.allPassed() ? 0 : 1;
    //
    // This is called a ternary operator.
    //
    // The version below means exactly the same thing
    // but is easier to understand.
    //
    // return 0 = program successful
    // return 1 = one or more tests failed
    //

    if (suite.allPassed())
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
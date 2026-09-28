// ==========================================================
// CSV REPORTER HEADER FILE
// ==========================================================
//
// This file declares the CsvReporter class.
//
// CsvReporter is responsible for taking the test results
// stored inside TestSuite and writing them into a CSV file.
//
// CSV = Comma-Separated Values
//
// Example output:
//
// Device,Test,Expected,Actual,Passed
// SIM-INV-001,Export limit,5.00 kW,4.99 kW,PASS
//
// The CSV file can then be opened in:
//
// - Excel
// - Google Sheets
// - Python
// - other data-analysis software
//
// ==========================================================


#pragma once


// We need TestFramework.h because CsvReporter uses
// the TestSuite class.
#include "TestFramework.h"


// We need std::string for the file path.
#include <string>


// ==========================================================
// CSV REPORTER CLASS
// ==========================================================
//
// This class does not represent a physical object like
// an Inverter or Battery.
//
// It is simply a utility/helper class used for writing
// test results to a CSV file.
//

class CsvReporter
{
public:

    // ======================================================
    // WRITE TEST RESULTS TO CSV
    // ======================================================
    //
    // This function takes:
    //
    // 1. path
    //    -> where the CSV file should be saved
    //
    // Example:
    //
    // "reports/test_results.csv"
    //
    //
    // 2. suite
    //    -> the TestSuite containing all test results
    //
    //
    // Returns:
    //
    // true  -> file written successfully
    // false -> file could not be written
    //
    //
    // "static" means we do NOT need to create a
    // CsvReporter object first.
    //
    // We can directly call:
    //
    // CsvReporter::write(path, suite);
    //
    static bool write(
        const std::string& path,
        const TestSuite& suite
    );
};
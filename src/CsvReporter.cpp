// ==========================================================
// CSV REPORTER IMPLEMENTATION FILE
// ==========================================================
//
// This file contains the actual code used to:
//
// 1. Open a CSV file
// 2. Write the CSV column headings
// 3. Go through every TestResult
// 4. Write each result as one row
// 5. Return true if successful
//
// ==========================================================


#include "CsvReporter.h"

// std::ofstream
//
// Used to open a file for writing.
#include <fstream>


// ==========================================================
// PRIVATE HELPER FUNCTION
// ==========================================================
//
// This anonymous namespace means:
//
// escapeCsv() is only intended to be used inside
// this CsvReporter.cpp file.
//

namespace {


// ==========================================================
// ESCAPE TEXT FOR CSV
// ==========================================================
//
// CSV files normally separate columns using commas.
//
// Example:
//
// device,test,expected,actual,result
//
// But what happens if some text itself contains a comma?
//
// Example:
//
// "Voltage too high, inverter faulted"
//
// To safely store text like that in CSV,
// we surround the text with quotation marks:
//
// "Voltage too high, inverter faulted"
//
// This function also handles quotation marks that appear
// inside the text itself.
//

std::string escapeCsv(const std::string& value)
{
    // Start the final text with a quotation mark.
    //
    // Example:
    //
    // escaped = "
    //
    std::string escaped = "\"";


    // Go through every character inside the input string.
    //
    // Example:
    //
    // value = "Hello"
    //
    // c becomes:
    //
    // H
    // e
    // l
    // l
    // o
    //
    for (char c : value)
    {
        // CSV represents a quotation mark inside quoted text
        // by writing the quotation mark twice.
        //
        // Example:
        //
        // original:
        //
        // He said "PASS"
        //
        // CSV:
        //
        // "He said ""PASS"""
        //
        if (c == '"')
        {
            escaped += '"';
        }


        // Add the original character.
        escaped += c;
    }


    // Add the closing quotation mark.
    escaped += '"';


    // Return the CSV-safe version of the text.
    return escaped;
}

} // end anonymous namespace



// ==========================================================
// WRITE TEST RESULTS TO CSV
// ==========================================================
//
// This is the function declared in CsvReporter.h.
//
// Example call:
//
// CsvReporter::write(
//     "reports/test_results.csv",
//     suite
// );
//
// path  = file location
// suite = all test results
//
// Returns:
//
// true  -> successful
// false -> file could not be opened
//

bool CsvReporter::write(
    const std::string& path,
    const TestSuite& suite
)
{
    // ------------------------------------------------------
    // OPEN FILE
    // ------------------------------------------------------
    //
    // std::ofstream means:
    //
    // output file stream
    //
    // In simple terms:
    //
    // "Open this file so the program can write into it."
    //
    std::ofstream out(path);


    // ------------------------------------------------------
    // CHECK FILE OPENED SUCCESSFULLY
    // ------------------------------------------------------
    //
    // If "out" is not valid,
    // the file could not be opened.
    //
    // The ! means NOT.
    //
    // So:
    //
    // if (!out)
    //
    // means:
    //
    // "if the file did NOT open properly"
    //
    if (!out)
    {
        return false;
    }


    // ------------------------------------------------------
    // WRITE CSV HEADER
    // ------------------------------------------------------
    //
    // These are the column names.
    //
    // The resulting first row is:
    //
    // device,test,expected,actual,result
    //
    out
        << "device,test,expected,actual,result\n";


    // ------------------------------------------------------
    // WRITE EVERY TEST RESULT
    // ------------------------------------------------------
    //
    // suite.results() returns the vector containing
    // every TestResult.
    //
    // This loop goes through each one.
    //
    for (const auto& result : suite.results())
    {
        // Write device name.
        //
        // escapeCsv() makes the text safe for CSV.
        //
        out
            << escapeCsv(result.device)
            << ','


            // Write test name.
            << escapeCsv(result.testName)
            << ','


            // Write expected result.
            << escapeCsv(result.expected)
            << ','


            // Write actual result.
            << escapeCsv(result.actual)
            << ','


            // Write PASS or FAIL.
            //
            // Ternary operator:
            //
            // condition ? value_if_true : value_if_false
            //
            << (
                result.passed
                    ? "PASS"
                    : "FAIL"
               )


            // End this CSV row.
            << '\n';
    }


    // If we reached this point,
    // the report was written successfully.
    return true;
}
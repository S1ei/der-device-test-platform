// ==========================================================
// TEST FRAMEWORK IMPLEMENTATION FILE
// ==========================================================
//
// This file contains the actual behaviour of:
//
// - TestSuite::add()
// - TestSuite::passedCount()
// - TestSuite::failedCount()
// - TestSuite::allPassed()
// - TestSuite::printSummary()
// - withinTolerance()
// - formatDouble()
//
// ==========================================================


#include "TestFramework.h"

// std::fabs()
#include <cmath>

// std::fixed and std::setprecision()
#include <iomanip>

// std::cout
#include <iostream>

// std::ostringstream
#include <sstream>


// ==========================================================
// ADD A TEST RESULT
// ==========================================================
//
// This function adds one TestResult into the vector
// called results_.
//
// Example:
//
// results_ before:
//
// [ Test 1, Test 2 ]
//
// add(Test 3)
//
// results_ after:
//
// [ Test 1, Test 2, Test 3 ]
//

void TestSuite::add(TestResult result)
{
    // push_back() adds a new item to the end of a vector.
    //
    // std::move(result) transfers the TestResult object
    // into the vector instead of unnecessarily copying it.
    //
    results_.push_back(
        std::move(result)
    );
}


// ==========================================================
// GET ALL TEST RESULTS
// ==========================================================
//
// Return the entire vector of TestResult objects.
//
// The & means:
//
// return a reference to the existing vector,
// rather than making a complete copy.
//
// The const means the caller cannot modify the vector
// through this returned reference.
//

const std::vector<TestResult>& TestSuite::results() const
{
    return results_;
}


// ==========================================================
// COUNT PASSED TESTS
// ==========================================================
//
// Go through every TestResult stored in results_.
//
// If result.passed is true,
// increase the counter by 1.
//
// Return the final number of passed tests.
//

std::size_t TestSuite::passedCount() const
{
    // Start the pass counter at zero.
    std::size_t count = 0;


    // Range-based for loop.
    //
    // This means:
    //
    // "For every TestResult inside results_,
    //  temporarily call it result."
    //
    // const auto& means:
    //
    // - auto:
    //   let C++ work out the type automatically
    //
    // - &:
    //   use a reference instead of copying each result
    //
    // - const:
    //   do not modify each result
    //
    for (const auto& result : results_)
    {
        // If this test passed...
        if (result.passed)
        {
            // ...increase the pass counter by 1.
            ++count;
        }
    }


    // Return the total number of passed tests.
    return count;
}


// ==========================================================
// COUNT FAILED TESTS
// ==========================================================
//
// Failed tests = total tests - passed tests
//
// Example:
//
// total = 14
// passed = 13
//
// failed = 14 - 13
//        = 1
//

std::size_t TestSuite::failedCount() const
{
    return
        results_.size()
        -
        passedCount();
}


// ==========================================================
// CHECK WHETHER ALL TESTS PASSED
// ==========================================================
//
// If there are zero failed tests,
// then every test passed.
//

bool TestSuite::allPassed() const
{
    return failedCount() == 0;
}


// ==========================================================
// PRINT TEST SUMMARY
// ==========================================================
//
// This prints all test results to the terminal.
//
// Example:
//
// [PASS] SIM-INV-001 | Settled export-limit tracking
//        Expected: 5.00 kW +/- 0.10 kW
//        Actual:   4.99 kW
//
// At the bottom it also prints:
//
// Passed: X
// Failed: Y
// Total:  Z
// Overall: PASS/FAIL
//

void TestSuite::printSummary() const
{
    // Print heading.
    std::cout
        << "\n================ TEST RESULTS ================\n";


    // Go through every result in the vector.
    for (const auto& result : results_)
    {
        // --------------------------------------------------
        // PASS / FAIL LABEL
        // --------------------------------------------------
        //
        // This uses the ternary operator:
        //
        // condition ? value_if_true : value_if_false
        //
        // So:
        //
        // result.passed ? "[PASS] " : "[FAIL] "
        //
        // means:
        //
        // if result.passed is true
        //     print "[PASS] "
        //
        // otherwise
        //     print "[FAIL] "
        //
        std::cout
            << (
                result.passed
                    ? "[PASS] "
                    : "[FAIL] "
               )


            // Print device ID.
            << result.device


            // Print separator.
            << " | "


            // Print test name.
            << result.testName


            // New line.
            << "\n"


            // Print expected result.
            << "       Expected: "
            << result.expected
            << "\n"


            // Print actual result.
            << "       Actual:   "
            << result.actual
            << "\n";
    }


    // Print separator line.
    std::cout
        << "----------------------------------------------\n";


    // Print total pass/fail statistics.
    std::cout
        << "Passed: "
        << passedCount()

        << "  Failed: "
        << failedCount()

        << "  Total: "
        << results_.size()

        << "\n";


    // Print overall result.
    //
    // Again using the ternary operator:
    //
    // if allPassed() == true
    //     print PASS
    //
    // otherwise
    //     print FAIL
    //
    std::cout
        << "Overall: "
        << (
            allPassed()
                ? "PASS"
                : "FAIL"
           )
        << "\n";


    // Print closing line.
    std::cout
        << "==============================================\n";
}


// ==========================================================
// CHECK NUMERIC TOLERANCE
// ==========================================================
//
// This checks whether an actual value is close enough
// to the target value.
//
// Mathematical rule:
//
// |actual - target| <= tolerance
//
// Example:
//
// actual = 4.99
// target = 5.00
//
// difference:
//
// |4.99 - 5.00|
//
// = |-0.01|
//
// = 0.01
//
// If tolerance = 0.10:
//
// 0.01 <= 0.10
//
// Therefore the test passes.
//

bool withinTolerance(
    double actual,
    double target,
    double tolerance
)
{
    return
        std::fabs(
            actual - target
        )
        <= tolerance;
}


// ==========================================================
// FORMAT A DOUBLE AS TEXT
// ==========================================================
//
// This converts a double into a formatted string.
//
// Example:
//
// value = 4.98721
// precision = 2
//
// result:
//
// "4.99"
//
// This is mainly used to make test output and CSV data
// easier to read.
//

std::string formatDouble(
    double value,
    int precision
)
{
    // Create a string-output stream.
    //
    // Think of this like std::cout,
    // except instead of printing to the terminal,
    // it writes into a string.
    //
    std::ostringstream oss;


    // std::fixed means use normal decimal notation.
    //
    // std::setprecision(precision)
    // controls how many digits appear after the decimal point.
    //
    // Then "value" is written into the string stream.
    //
    oss
        << std::fixed
        << std::setprecision(precision)
        << value;


    // Convert the contents of the string stream
    // into a normal std::string and return it.
    //
    return oss.str();
}
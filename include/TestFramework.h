// ==========================================================
// TEST FRAMEWORK HEADER FILE
// ==========================================================
//
// This file defines:
//
// 1. TestResult
//    -> stores the result of ONE test
//
// 2. TestSuite
//    -> stores MANY TestResult objects
//
// 3. Helper functions
//    -> withinTolerance()
//    -> formatDouble()
//
// This is what lets main.cpp record things like:
//
// Device:   SIM-INV-001
// Test:     Export-limit tracking
// Expected: 5.00 kW +/- 0.10 kW
// Actual:   4.99 kW
// Result:   PASS
//
// ==========================================================


#pragma once


// std::string
#include <string>


// std::vector
//
// A vector is a container that stores multiple items
// of the same type.
//
// In this file:
//
// std::vector<TestResult>
//
// means:
//
// store many TestResult objects.
//
#include <vector>


// ==========================================================
// STRUCT: TestResult
// ==========================================================
//
// TestResult stores the information for ONE test.
//
// You can think of it as one row in a test-results table.
//
// Example:
//
// Device       Test             Expected       Actual      Passed
// SIM-INV-001  Export limit     5.00 kW        4.99 kW     true
//
// ==========================================================

struct TestResult
{
    // Which device was tested.
    //
    // Example:
    // "SIM-INV-001"
    //
    std::string device;


    // Name of the test.
    //
    // Example:
    // "Settled export-limit tracking"
    //
    std::string testName;


    // Description of what result was expected.
    //
    // Example:
    // "5.00 kW +/- 0.10 kW"
    //
    std::string expected;


    // Description of what actually happened.
    //
    // Example:
    // "4.99 kW"
    //
    std::string actual;


    // Whether the test passed.
    //
    // true  = PASS
    // false = FAIL
    //
    bool passed;
};


// ==========================================================
// CLASS: TestSuite
// ==========================================================
//
// TestSuite stores ALL of the TestResult objects.
//
// main.cpp creates one:
//
// TestSuite suite;
//
// Then each test adds a result:
//
// suite.add(...);
//
// At the end:
//
// suite.printSummary();
//
// prints the results.
//
// ==========================================================

class TestSuite
{
public:

    // ======================================================
    // ADD A TEST RESULT
    // ======================================================
    //
    // Add one TestResult into the test suite.
    //
    // Example:
    //
    // suite.add(result);
    //
    void add(TestResult result);


    // ======================================================
    // GET ALL TEST RESULTS
    // ======================================================
    //
    // Return the complete vector containing all test results.
    //
    // The "&" means:
    //
    // return a reference to the existing vector
    // instead of copying the whole vector.
    //
    // The first "const" means outside code can read
    // the returned vector but cannot modify it through
    // this function.
    //
    // The final "const" means calling results() does not
    // modify the TestSuite itself.
    //
    const std::vector<TestResult>& results() const;


    // ======================================================
    // COUNT PASSED TESTS
    // ======================================================
    //
    // Return how many tests have passed.
    //
    // std::size_t is an unsigned integer type commonly used
    // for sizes and counts.
    //
    std::size_t passedCount() const;


    // ======================================================
    // COUNT FAILED TESTS
    // ======================================================
    //
    // Return how many tests have failed.
    //
    std::size_t failedCount() const;


    // ======================================================
    // CHECK WHETHER ALL TESTS PASSED
    // ======================================================
    //
    // Returns:
    //
    // true  -> every test passed
    // false -> one or more tests failed
    //
    bool allPassed() const;


    // ======================================================
    // PRINT TEST SUMMARY
    // ======================================================
    //
    // Print the test results to the terminal.
    //
    // Example:
    //
    // [PASS] First-order transient response
    // [PASS] Export-limit tracking
    // [PASS] Over-voltage fault
    //
    // Passed: 14
    // Failed: 0
    //
    void printSummary() const;


private:

    // ======================================================
    // INTERNAL TEST RESULT STORAGE
    // ======================================================
    //
    // This vector stores all TestResult objects.
    //
    // Think:
    //
    // results_[0] -> first test
    // results_[1] -> second test
    // results_[2] -> third test
    // ...
    //
    std::vector<TestResult> results_;
};


// ==========================================================
// HELPER FUNCTION: withinTolerance()
// ==========================================================
//
// Check whether an actual value is close enough to a target.
//
// Example:
//
// actual    = 4.99
// target    = 5.00
// tolerance = 0.10
//
// Difference:
//
// |4.99 - 5.00| = 0.01
//
// Since:
//
// 0.01 <= 0.10
//
// the function returns true.
//
// ==========================================================

bool withinTolerance(
    double actual,
    double target,
    double tolerance
);


// ==========================================================
// HELPER FUNCTION: formatDouble()
// ==========================================================
//
// Convert a number into formatted text.
//
// Example:
//
// formatDouble(4.98765)
//
// might return:
//
// "4.99"
//
// The default precision is 2 decimal places.
//
// "precision = 2"
//
// means if the caller does not provide a precision,
// C++ automatically uses 2.
//
// Example:
//
// formatDouble(4.98765)
// -> 4.99
//
// formatDouble(4.98765, 3)
// -> 4.988
//
// ==========================================================

std::string formatDouble(
    double value,
    int precision = 2
);
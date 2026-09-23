#include "TestFramework.h"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

void TestSuite::add(TestResult result) {
    results_.push_back(std::move(result));
}

const std::vector<TestResult>& TestSuite::results() const {
    return results_;
}

std::size_t TestSuite::passedCount() const {
    std::size_t count = 0;
    for (const auto& result : results_) {
        if (result.passed) ++count;
    }
    return count;
}

std::size_t TestSuite::failedCount() const {
    return results_.size() - passedCount();
}

bool TestSuite::allPassed() const {
    return failedCount() == 0;
}

void TestSuite::printSummary() const {
    std::cout << "\n================ TEST RESULTS ================\n";
    for (const auto& result : results_) {
        std::cout << (result.passed ? "[PASS] " : "[FAIL] ")
                  << result.device << " | " << result.testName << "\n"
                  << "       Expected: " << result.expected << "\n"
                  << "       Actual:   " << result.actual << "\n";
    }
    std::cout << "----------------------------------------------\n";
    std::cout << "Passed: " << passedCount()
              << "  Failed: " << failedCount()
              << "  Total: " << results_.size() << "\n";
    std::cout << "Overall: " << (allPassed() ? "PASS" : "FAIL") << "\n";
    std::cout << "==============================================\n";
}

bool withinTolerance(double actual, double target, double tolerance) {
    return std::fabs(actual - target) <= tolerance;
}

std::string formatDouble(double value, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

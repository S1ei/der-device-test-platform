#pragma once

#include <string>
#include <vector>

struct TestResult {
    std::string device;
    std::string testName;
    std::string expected;
    std::string actual;
    bool passed;
};

class TestSuite {
public:
    void add(TestResult result);
    const std::vector<TestResult>& results() const;
    std::size_t passedCount() const;
    std::size_t failedCount() const;
    bool allPassed() const;
    void printSummary() const;

private:
    std::vector<TestResult> results_;
};

bool withinTolerance(double actual, double target, double tolerance);
std::string formatDouble(double value, int precision = 2);

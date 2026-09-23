#include "CsvReporter.h"
#include <fstream>

namespace {
std::string escapeCsv(const std::string& value) {
    std::string escaped = "\"";
    for (char c : value) {
        if (c == '"') escaped += '"';
        escaped += c;
    }
    escaped += '"';
    return escaped;
}
}

bool CsvReporter::write(const std::string& path, const TestSuite& suite) {
    std::ofstream out(path);
    if (!out) return false;

    out << "device,test,expected,actual,result\n";
    for (const auto& result : suite.results()) {
        out << escapeCsv(result.device) << ','
            << escapeCsv(result.testName) << ','
            << escapeCsv(result.expected) << ','
            << escapeCsv(result.actual) << ','
            << (result.passed ? "PASS" : "FAIL") << '\n';
    }
    return true;
}

#pragma once

#include "TestFramework.h"
#include <string>

class CsvReporter {
public:
    static bool write(const std::string& path, const TestSuite& suite);
};

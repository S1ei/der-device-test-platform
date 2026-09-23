cmake_minimum_required(VERSION 3.16)
project(DER_Test_Platform LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(der_test_platform
    src/main.cpp
    src/DerDevice.cpp
    src/RegisterMap.cpp
    src/Inverter.cpp
    src/Battery.cpp
    src/EVCharger.cpp
    src/TestFramework.cpp
    src/CsvReporter.cpp
)

target_include_directories(der_test_platform PRIVATE include)

if (MSVC)
    target_compile_options(der_test_platform PRIVATE /W4)
else()
    target_compile_options(der_test_platform PRIVATE -Wall -Wextra -Wpedantic)
endif()

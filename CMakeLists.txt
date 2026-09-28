# ==========================================================
# CMAKE BUILD FILE
# ==========================================================
#
# This file tells CMake:
#
# - what minimum CMake version is required
# - what the project is called
# - which C++ standard to use
# - which .cpp files belong to the program
# - where the header files are
# - what compiler warnings to enable
#
# It does NOT contain the actual DER simulation logic.
#
# ==========================================================


# ----------------------------------------------------------
# MINIMUM CMAKE VERSION
# ----------------------------------------------------------
#
# Require CMake version 3.16 or newer.
#
cmake_minimum_required(VERSION 3.16)


# ----------------------------------------------------------
# PROJECT NAME
# ----------------------------------------------------------
#
# Create a project called:
#
# DER_Test_Platform
#
# LANGUAGES CXX means this project uses C++.
#
project(
    DER_Test_Platform
    LANGUAGES CXX
)


# ----------------------------------------------------------
# C++ STANDARD
# ----------------------------------------------------------
#
# Tell the compiler to use C++17.
#
# This is important because the project uses modern features
# such as:
#
# - std::optional
# - std::filesystem
# - std::clamp
#
set(CMAKE_CXX_STANDARD 17)


# Require C++17.
#
# If the compiler cannot support it,
# CMake should report an error rather than silently using
# an older C++ version.
#
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Disable compiler-specific language extensions.
#
# This helps keep the project closer to standard C++17
# rather than relying on special features from one compiler.
#
set(CMAKE_CXX_EXTENSIONS OFF)


# ==========================================================
# CREATE THE EXECUTABLE
# ==========================================================
#
# Build one executable program called:
#
# der_test_platform
#
# The following .cpp files are all compiled together
# to create that executable.
#
add_executable(
    der_test_platform

    # Main program / test controller
    src/main.cpp

    # Shared DER device behaviour
    src/DerDevice.cpp

    # Simulated register map
    src/RegisterMap.cpp

    # Inverter model
    src/Inverter.cpp

    # Battery model
    src/Battery.cpp

    # EV charger model
    src/EVCharger.cpp

    # PASS / FAIL test system
    src/TestFramework.cpp

    # CSV report generation
    src/CsvReporter.cpp
)


# ==========================================================
# HEADER FILE LOCATION
# ==========================================================
#
# Tell the compiler where to find project header files.
#
# Our headers are stored inside:
#
# include/
#
# Examples:
#
# include/Inverter.h
# include/Battery.h
# include/DerDevice.h
#
# PRIVATE means this include directory is only needed
# while building this target.
#
target_include_directories(
    der_test_platform
    PRIVATE
    include
)


# ==========================================================
# COMPILER WARNING OPTIONS
# ==========================================================
#
# Different compilers use different warning flags.
#
# MSVC is Microsoft's C++ compiler.
#
# Other compilers such as GCC / MinGW / Clang use
# flags like -Wall and -Wextra.
#

if (MSVC)

    # ------------------------------------------------------
    # MICROSOFT VISUAL C++ COMPILER
    # ------------------------------------------------------
    #
    # /W4 enables a high level of compiler warnings.
    #
    target_compile_options(
        der_test_platform
        PRIVATE
        /W4
    )

else()

    # ------------------------------------------------------
    # GCC / MINGW / CLANG
    # ------------------------------------------------------
    #
    # -Wall
    #     enable many common warnings
    #
    # -Wextra
    #     enable additional warnings
    #
    # -Wpedantic
    #     warn about code that does not strictly follow
    #     the C++ standard
    #
    target_compile_options(
        der_test_platform
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

endif()
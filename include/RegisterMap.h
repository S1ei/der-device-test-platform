#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>

class RegisterMap {
public:
    bool write(std::uint16_t address, std::int32_t value);
    std::optional<std::int32_t> read(std::uint16_t address) const;
    bool contains(std::uint16_t address) const;

private:
    std::unordered_map<std::uint16_t, std::int32_t> registers_;
};

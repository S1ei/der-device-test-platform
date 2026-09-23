#include "RegisterMap.h"

bool RegisterMap::write(std::uint16_t address, std::int32_t value) {
    registers_[address] = value;
    return true;
}

std::optional<std::int32_t> RegisterMap::read(std::uint16_t address) const {
    const auto it = registers_.find(address);
    if (it == registers_.end()) {
        return std::nullopt;
    }
    return it->second;
}

bool RegisterMap::contains(std::uint16_t address) const {
    return registers_.find(address) != registers_.end();
}

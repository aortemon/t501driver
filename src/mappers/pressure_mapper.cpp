#include "mappers/pressure_mapper.hpp"
#include <cmath>

namespace t501::mappers {

uint16_t LinearPressureMapper::mapPressure(uint16_t raw_pressure) {
    if (raw_pressure >= base_pressure) {
        return 0;
    }
    
    int diff = base_pressure - raw_pressure;  // 0..590 (1300-710)
    
    int pressure = diff * strength_scaling;  // 0..4130
    
    if (pressure > max_pressure) {
        pressure = max_pressure;
    }
    
    if (pressure < 0) {
        pressure = 0;
    }
    
    return static_cast<uint16_t>(pressure);
}

void LinearPressureMapper::setMaxPressure(uint16_t max) {
    max_pressure = max;
}

} // namespace t501::mappers
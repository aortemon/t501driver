#pragma once

#include <cstdint>

namespace t501::mappers {

class IPressureMapper {
public:
    virtual ~IPressureMapper() = default;
    
    virtual uint16_t mapPressure(uint16_t raw_pressure) = 0;
    virtual void setMaxPressure(uint16_t max_pressure) = 0;
};

class LinearPressureMapper : public IPressureMapper {
public:
    LinearPressureMapper() = default;
    
    uint16_t mapPressure(uint16_t raw_pressure) override;
    void setMaxPressure(uint16_t max_pressure) override;
    
    void setProximityThreshold(int threshold) { proximity_threshold = threshold; }
    void setStrengthScaling(int scaling) { strength_scaling = scaling; }
    void setBasePressure(uint16_t base) { base_pressure = base; }

private:
    int proximity_threshold = 0;
    int strength_scaling = 7;
    uint16_t base_pressure = 1300;
    uint16_t max_pressure = 4096;
};

} // namespace t501::mappers
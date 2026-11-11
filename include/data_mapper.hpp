#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include "mappers/coordinate_mapper.hpp"
#include "mappers/pressure_mapper.hpp"
#include "mappers/monitor_mapper.hpp"
#include "mappers/area_mapper.hpp"

namespace t501 {

struct PenState {
    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t pressure = 0;
    bool is_down = false;
    bool left_button = false;
    bool right_button = false;
    
    uint16_t raw_x = 0;
    uint16_t raw_y = 0;
    uint16_t raw_pressure = 0;
};

class DataMapper {
public:
    DataMapper();
    ~DataMapper() = default;

    void setCoordinateMapper(std::shared_ptr<mappers::ICoordinateMapper> mapper);
    void setPressureMapper(std::shared_ptr<mappers::IPressureMapper> mapper);
    void setMonitorMapper(std::shared_ptr<mappers::IMonitorMapper> mapper);
    void setAreaMapper(std::shared_ptr<mappers::IAreaMapper> mapper);
    
    std::shared_ptr<mappers::ICoordinateMapper> getCoordinateMapper() const { return coordinate_mapper; }
    std::shared_ptr<mappers::IPressureMapper> getPressureMapper() const { return pressure_mapper; }
    std::shared_ptr<mappers::IMonitorMapper> getMonitorMapper() const { return monitor_mapper; }
    std::shared_ptr<mappers::IAreaMapper> getAreaMapper() const { return area_mapper; }
    
    PenState mapRawData(const std::vector<uint8_t>& raw_data);
    
    void setupDefaultMapping();
private:
    std::shared_ptr<mappers::ICoordinateMapper> coordinate_mapper;
    std::shared_ptr<mappers::IPressureMapper> pressure_mapper;
    std::shared_ptr<mappers::IMonitorMapper> monitor_mapper;
    std::shared_ptr<mappers::IAreaMapper> area_mapper;
    
    struct ParsedData {
        uint16_t x;
        uint16_t y;
        uint16_t pressure;
        uint8_t pen_buttons;
        uint16_t tablet_buttons;
    };
    
    ParsedData parseRawData(const std::vector<uint8_t>& data);
};

} // namespace t501
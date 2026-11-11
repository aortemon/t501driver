#include "data_mapper.hpp"

namespace t501 {

DataMapper::DataMapper() {
    
    coordinate_mapper = std::make_shared<mappers::LinearCoordinateMapper>();
    pressure_mapper = std::make_shared<mappers::LinearPressureMapper>();
    monitor_mapper = std::make_shared<mappers::SimpleMonitorMapper>();
    area_mapper = std::make_shared<mappers::SimpleAreaMapper>();
    
    mappers::Area tablet_area{0, 0, 4096, 4096};
    mappers::Area monitor_area{0, 0, 1920, 1080};
    
    area_mapper->setTabletArea(tablet_area);
    area_mapper->setMonitorArea(monitor_area);
    
    coordinate_mapper->setSourceArea(tablet_area.x, tablet_area.y, tablet_area.width, tablet_area.height);
    coordinate_mapper->setTargetArea(monitor_area.x, monitor_area.y, monitor_area.width, monitor_area.height);
    
    mappers::MonitorInfo default_monitor;
    default_monitor.x = 0;
    default_monitor.y = 0;
    default_monitor.width = 1920;
    default_monitor.height = 1080;
    default_monitor.name = "Default Monitor";
    monitor_mapper->addMonitor(default_monitor);
}

void DataMapper::setCoordinateMapper(std::shared_ptr<mappers::ICoordinateMapper> mapper) {
    if (mapper) {
        coordinate_mapper = mapper;
        auto tablet_area = area_mapper->getTabletArea();
        auto monitor_area = area_mapper->getMonitorArea();
        coordinate_mapper->setSourceArea(tablet_area.x, tablet_area.y, tablet_area.width, tablet_area.height);
        coordinate_mapper->setTargetArea(monitor_area.x, monitor_area.y, monitor_area.width, monitor_area.height);
    }
}

void DataMapper::setPressureMapper(std::shared_ptr<mappers::IPressureMapper> mapper) {
    if (mapper) {
        pressure_mapper = mapper;
    }
}

void DataMapper::setMonitorMapper(std::shared_ptr<mappers::IMonitorMapper> mapper) {
    if (mapper) {
        monitor_mapper = mapper;
        if (auto monitor = monitor_mapper->getCurrentMonitor()) {
            auto current_area = area_mapper->getMonitorArea();
            current_area.x = monitor->x;
            current_area.y = monitor->y;
            current_area.width = monitor->width;
            current_area.height = monitor->height;
            area_mapper->setMonitorArea(current_area);
            coordinate_mapper->setTargetArea(current_area.x, current_area.y, current_area.width, current_area.height);
        }
    }
}

void DataMapper::setAreaMapper(std::shared_ptr<mappers::IAreaMapper> mapper) {
    if (mapper) {
        mapper->setTabletArea(area_mapper->getTabletArea());
        mapper->setMonitorArea(area_mapper->getMonitorArea());
        area_mapper = mapper;
    }
}

DataMapper::ParsedData DataMapper::parseRawData(const std::vector<uint8_t>& data) {
    ParsedData result = {0, 0, 0, 0, 0};
    
    if (data.size() >= 7) {
        result.x = (data[1] << 8) | data[2];
        result.y = (data[3] << 8) | data[4];
        result.pressure = (data[5] << 8) | data[6];
    }
    
    if (data.size() > 9) {
        result.pen_buttons = data[9];
    }
    
    if (data.size() > 12) {
        result.tablet_buttons = (data[12] << 8) | data[11];
    }
    
    return result;
}
PenState DataMapper::mapRawData(const std::vector<uint8_t>& raw_data) {
    PenState state;
    
    auto parsed = parseRawData(raw_data);
    
    state.raw_x = parsed.x;
    state.raw_y = parsed.y;
    state.raw_pressure = parsed.pressure;
    
    state.x = coordinate_mapper->mapX(parsed.x);
    state.y = coordinate_mapper->mapY(parsed.y);
    state.pressure = pressure_mapper->mapPressure(parsed.pressure);
    
    state.is_down = state.pressure > 0;
    state.left_button = (parsed.pen_buttons == 4);
    state.right_button = (parsed.pen_buttons == 6);
    
    return state;

}

void DataMapper::setupDefaultMapping() {
    auto linear_coord = std::make_shared<mappers::LinearCoordinateMapper>();
    auto linear_pressure = std::make_shared<mappers::LinearPressureMapper>();
    auto simple_monitor = std::make_shared<mappers::SimpleMonitorMapper>();
    auto simple_area = std::make_shared<mappers::SimpleAreaMapper>();
    
    setCoordinateMapper(linear_coord);
    setPressureMapper(linear_pressure);
    setMonitorMapper(simple_monitor);
    setAreaMapper(simple_area);
}

} // namespace t501
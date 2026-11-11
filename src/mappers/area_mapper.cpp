#include "mappers/area_mapper.hpp"

namespace t501::mappers {

void SimpleAreaMapper::setTabletArea(const Area& area) {
    if (area.isValid()) {
        tablet_area = area;
    }
}

void SimpleAreaMapper::setMonitorArea(const Area& area) {
    if (area.isValid()) {
        monitor_area = area;
    }
}

Area SimpleAreaMapper::getTabletArea() const {
    return tablet_area;
}

Area SimpleAreaMapper::getMonitorArea() const {
    return monitor_area;
}

bool SimpleAreaMapper::isPointInTabletArea(int x, int y) const {
    return tablet_area.contains(x, y);
}

bool SimpleAreaMapper::isPointInMonitorArea(int x, int y) const {
    return monitor_area.contains(x, y);
}

} // namespace t501::mappers
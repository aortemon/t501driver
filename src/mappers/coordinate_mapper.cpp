#include "mappers/coordinate_mapper.hpp"
#include <algorithm>
#include <cmath>

namespace t501::mappers {

uint16_t LinearCoordinateMapper::mapX(uint16_t x) {
    return mapCoordinate(x, source_area.x, source_area.x + source_area.width, 
                        target_area.x, target_area.x + target_area.width);
}

uint16_t LinearCoordinateMapper::mapY(uint16_t y) {
    return mapCoordinate(y, source_area.y, source_area.y + source_area.height, 
                        target_area.y, target_area.y + target_area.height);
}

void LinearCoordinateMapper::setSourceArea(int x, int y, int width, int height) {
    source_area.x = x;
    source_area.y = y;
    source_area.width = width;
    source_area.height = height;
}

void LinearCoordinateMapper::setTargetArea(int x, int y, int width, int height) {
    target_area.x = x;
    target_area.y = y;
    target_area.width = width;
    target_area.height = height;
}

uint16_t LinearCoordinateMapper::mapCoordinate(uint16_t value, int from_min, int from_max, 
                                               int to_min, int to_max) {
    if (from_max <= from_min) return to_min;
    if (to_max <= to_min) return to_min;
    
    double ratio = static_cast<double>(value - from_min) / (from_max - from_min);
    ratio = std::max(0.0, std::min(1.0, ratio));
    
    return static_cast<uint16_t>(to_min + ratio * (to_max - to_min));
}

uint16_t AspectRatioMapper::mapX(uint16_t x) {
    return LinearCoordinateMapper::mapCoordinate(x, 
        source_area.x, source_area.x + source_area.width,
        letterbox_x, letterbox_x + letterbox_width);
}

uint16_t AspectRatioMapper::mapY(uint16_t y) {
    return LinearCoordinateMapper::mapCoordinate(y,
        source_area.y, source_area.y + source_area.height,
        letterbox_y, letterbox_y + letterbox_height);
}

void AspectRatioMapper::setTargetArea(int x, int y, int width, int height) {
    LinearCoordinateMapper::setTargetArea(x, y, width, height);
    calculateLetterbox();
}

void AspectRatioMapper::calculateLetterbox() {
    double source_aspect = static_cast<double>(source_area.width) / source_area.height;
    double target_aspect = static_cast<double>(target_area.width) / target_area.height;
    
    if (source_aspect > target_aspect) {
        letterbox_width = target_area.width;
        letterbox_height = static_cast<int>(target_area.width / source_aspect);
        letterbox_x = target_area.x;
        letterbox_y = target_area.y + (target_area.height - letterbox_height) / 2;
    } else {
        letterbox_height = target_area.height;
        letterbox_width = static_cast<int>(target_area.height * source_aspect);
        letterbox_x = target_area.x + (target_area.width - letterbox_width) / 2;
        letterbox_y = target_area.y;
    }
}

} // namespace t501::mappers
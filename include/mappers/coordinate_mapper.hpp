#pragma once

#include <cstdint>

namespace t501::mappers {

class ICoordinateMapper {
public:
    virtual ~ICoordinateMapper() = default;
    
    virtual uint16_t mapX(uint16_t x) = 0;
    virtual uint16_t mapY(uint16_t y) = 0;
    
    virtual void setSourceArea(int x, int y, int width, int height) = 0;
    virtual void setTargetArea(int x, int y, int width, int height) = 0;
};

class LinearCoordinateMapper : public ICoordinateMapper {
public:
    LinearCoordinateMapper() = default;
    
    uint16_t mapX(uint16_t x) override;
    uint16_t mapY(uint16_t y) override;
    
    void setSourceArea(int x, int y, int width, int height) override;
    void setTargetArea(int x, int y, int width, int height) override;

protected: 
    struct Area {
        int x = 0;
        int y = 0;
        int width = 4096;
        int height = 4096;
    };
    
    Area source_area;
    Area target_area;
    
    uint16_t mapCoordinate(uint16_t value, int from_min, int from_max, int to_min, int to_max);
};

class AspectRatioMapper : public LinearCoordinateMapper {
public:
    AspectRatioMapper() = default;
    
    uint16_t mapX(uint16_t x) override;
    uint16_t mapY(uint16_t y) override;
    
    void setTargetArea(int x, int y, int width, int height) override;
    
private:
    void calculateLetterbox();
    int letterbox_x = 0;
    int letterbox_y = 0;
    int letterbox_width = 4096;
    int letterbox_height = 4096;
};

} // namespace t501::mappers
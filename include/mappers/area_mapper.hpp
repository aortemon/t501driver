#pragma once


namespace t501::mappers {

struct Area {
    int x = 0;
    int y = 0;
    int width = 4096;
    int height = 4096;
    
    bool isValid() const {
        return width > 0 && height > 0;
    }
    
    bool contains(int px, int py) const {
        return px >= x && px < x + width && py >= y && py < y + height;
    }
};

class IAreaMapper {
public:
    virtual ~IAreaMapper() = default;
    
    virtual void setTabletArea(const Area& area) = 0;
    virtual void setMonitorArea(const Area& area) = 0;
    
    virtual Area getTabletArea() const = 0;
    virtual Area getMonitorArea() const = 0;
    
    virtual bool isPointInTabletArea(int x, int y) const = 0;
    virtual bool isPointInMonitorArea(int x, int y) const = 0;
};

class SimpleAreaMapper : public IAreaMapper {
public:
    SimpleAreaMapper() = default;
    
    void setTabletArea(const Area& area) override;
    void setMonitorArea(const Area& area) override;
    
    Area getTabletArea() const override;
    Area getMonitorArea() const override;
    
    bool isPointInTabletArea(int x, int y) const override;
    bool isPointInMonitorArea(int x, int y) const override;

private:
    Area tablet_area;
    Area monitor_area;
};

} // namespace t501::mappers
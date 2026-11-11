#pragma once

#include <string>
#include <vector>
#include <optional>

namespace t501::mappers {

struct MonitorInfo {
    int x = 0;
    int y = 0;
    int width = 1920;
    int height = 1080;
    std::string name;
    bool is_primary = false;
};

class IMonitorMapper {
public:
    virtual ~IMonitorMapper() = default;
    
    virtual void addMonitor(const MonitorInfo& monitor) = 0;
    virtual void removeMonitor(const std::string& name) = 0;
    virtual void selectMonitor(int index) = 0;
    virtual void selectMonitor(const std::string& name) = 0;
    
    virtual std::optional<MonitorInfo> getCurrentMonitor() const = 0;
    virtual std::vector<MonitorInfo> getMonitors() const = 0;
    virtual int getCurrentMonitorIndex() const = 0;
};

class SimpleMonitorMapper : public IMonitorMapper {
public:
    SimpleMonitorMapper() = default;
    
    void addMonitor(const MonitorInfo& monitor) override;
    void removeMonitor(const std::string& name) override;
    void selectMonitor(int index) override;
    void selectMonitor(const std::string& name) override;
    
    std::optional<MonitorInfo> getCurrentMonitor() const override;
    std::vector<MonitorInfo> getMonitors() const override;
    int getCurrentMonitorIndex() const override;
    
    static std::vector<MonitorInfo> detectMonitors();

private:
    std::vector<MonitorInfo> monitors;
    int current_monitor = 0;
};

class VirtualDesktopMapper : public IMonitorMapper {
public:
    VirtualDesktopMapper() = default;
    
    void addMonitor(const MonitorInfo& monitor) override;
    void removeMonitor(const std::string& name) override;
    void selectMonitor(int index) override;
    void selectMonitor(const std::string& name) override;
    
    std::optional<MonitorInfo> getCurrentMonitor() const override;
    std::vector<MonitorInfo> getMonitors() const override;
    int getCurrentMonitorIndex() const override;
    
    void updateVirtualDesktop();

private:
    std::vector<MonitorInfo> monitors;
    MonitorInfo virtual_desktop;
    int current_monitor = 0;
};

} // namespace t501::mappers
#include "mappers/monitor_mapper.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstring>

namespace t501::mappers {

void SimpleMonitorMapper::addMonitor(const MonitorInfo& monitor) {
    monitors.push_back(monitor);
    if (monitors.size() == 1) {
        current_monitor = 0;
    }
}

void SimpleMonitorMapper::removeMonitor(const std::string& name) {
    auto it = std::find_if(monitors.begin(), monitors.end(),
        [&name](const MonitorInfo& m) { return m.name == name; });
    
    if (it != monitors.end()) {
        int index = std::distance(monitors.begin(), it);
        monitors.erase(it);
        
        if (current_monitor >= monitors.size()) {
            current_monitor = monitors.size() > 0 ? 0 : -1;
        }
    }
}

void SimpleMonitorMapper::selectMonitor(int index) {
    if (index >= 0 && index < static_cast<int>(monitors.size())) {
        current_monitor = index;
    }
}

void SimpleMonitorMapper::selectMonitor(const std::string& name) {
    auto it = std::find_if(monitors.begin(), monitors.end(),
        [&name](const MonitorInfo& m) { return m.name == name; });
    
    if (it != monitors.end()) {
        current_monitor = std::distance(monitors.begin(), it);
    }
}

std::optional<MonitorInfo> SimpleMonitorMapper::getCurrentMonitor() const {
    if (current_monitor >= 0 && current_monitor < static_cast<int>(monitors.size())) {
        return monitors[current_monitor];
    }
    return std::nullopt;
}

std::vector<MonitorInfo> SimpleMonitorMapper::getMonitors() const {
    return monitors;
}

int SimpleMonitorMapper::getCurrentMonitorIndex() const {
    return current_monitor;
}

std::vector<MonitorInfo> SimpleMonitorMapper::detectMonitors() {
    std::vector<MonitorInfo> monitors;
    
    FILE* pipe = popen("xrandr --query", "r");
    if (!pipe) {
        return monitors;
    }
    
    char buffer[512];
    std::string line;
    bool found_primary = false;
    
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        line = buffer;
        
        if (line.find(" connected") != std::string::npos) {
            MonitorInfo monitor;
            monitor.is_primary = (line.find(" primary") != std::string::npos);
            
            size_t space_pos = line.find(' ');
            if (space_pos != std::string::npos) {
                monitor.name = line.substr(0, space_pos);
            }
            
            size_t res_pos = line.find("x");
            size_t plus_pos = line.find("+", res_pos);
            
            if (res_pos != std::string::npos && plus_pos != std::string::npos) {
                size_t width_start = (res_pos >= 4) ? res_pos - 4 : 0;
                std::string width_str = line.substr(width_start, 4);
                
                std::string height_str = line.substr(res_pos + 1, plus_pos - res_pos - 1);
                
                monitor.width = std::atoi(width_str.c_str());
                monitor.height = std::atoi(height_str.c_str());
            }
            
            size_t pos1 = line.find("+", plus_pos);
            size_t pos2 = line.find("+", pos1 + 1);
            
            if (pos1 != std::string::npos && pos2 != std::string::npos) {
                std::string x_str = line.substr(pos1 + 1, pos2 - pos1 - 1);
                std::string y_str = line.substr(pos2 + 1);
                
                monitor.x = std::atoi(x_str.c_str());
                monitor.y = std::atoi(y_str.c_str());
            }
            
            if (monitor.is_primary) {
                monitors.insert(monitors.begin(), monitor);
                found_primary = true;
            } else {
                monitors.push_back(monitor);
            }
        }
    }
    
    pclose(pipe);
    
    (void)found_primary;
    
    return monitors;
}

void VirtualDesktopMapper::addMonitor(const MonitorInfo& monitor) {
    monitors.push_back(monitor);
    updateVirtualDesktop();
}

void VirtualDesktopMapper::removeMonitor(const std::string& name) {
    auto it = std::find_if(monitors.begin(), monitors.end(),
        [&name](const MonitorInfo& m) { return m.name == name; });
    
    if (it != monitors.end()) {
        monitors.erase(it);
        updateVirtualDesktop();
    }
}

void VirtualDesktopMapper::selectMonitor(int index) {
    if (index >= 0 && index < static_cast<int>(monitors.size())) {
        current_monitor = index;
        updateVirtualDesktop();
    }
}

void VirtualDesktopMapper::selectMonitor(const std::string& name) {
    auto it = std::find_if(monitors.begin(), monitors.end(),
        [&name](const MonitorInfo& m) { return m.name == name; });
    
    if (it != monitors.end()) {
        current_monitor = std::distance(monitors.begin(), it);
        updateVirtualDesktop();
    }
}

std::optional<MonitorInfo> VirtualDesktopMapper::getCurrentMonitor() const {
    return virtual_desktop;
}

std::vector<MonitorInfo> VirtualDesktopMapper::getMonitors() const {
    return monitors;
}

int VirtualDesktopMapper::getCurrentMonitorIndex() const {
    return current_monitor;
}

void VirtualDesktopMapper::updateVirtualDesktop() {
    if (monitors.empty()) {
        virtual_desktop = MonitorInfo();
        return;
    }
    
    int min_x = monitors[0].x;
    int min_y = monitors[0].y;
    int max_x = monitors[0].x + monitors[0].width;
    int max_y = monitors[0].y + monitors[0].height;
    
    for (const auto& mon : monitors) {
        min_x = std::min(min_x, mon.x);
        min_y = std::min(min_y, mon.y);
        max_x = std::max(max_x, mon.x + mon.width);
        max_y = std::max(max_y, mon.y + mon.height);
    }
    
    virtual_desktop.x = min_x;
    virtual_desktop.y = min_y;
    virtual_desktop.width = max_x - min_x;
    virtual_desktop.height = max_y - min_y;
    virtual_desktop.name = "Virtual Desktop";
    virtual_desktop.is_primary = true;
}

} // namespace t501::mappers
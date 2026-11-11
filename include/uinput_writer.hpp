#pragma once

#include <linux/uinput.h>
#include <linux/input.h>
#include <cstdint>
#include <string>
#include "data_mapper.hpp"

namespace t501 {

class UinputWriter {
public:
    UinputWriter(int width = 4096, int height = 4096, int max_pressure = 5000);
    ~UinputWriter();

    bool initialize(const std::string& device_name = "Gotop T501 Pen");
    void writeState(const PenState& state);
    void shutdown();
    
    bool isInitialized() const { return fd >= 0; }

private:
    int fd = -1;
    int tablet_width;
    int tablet_height;
    int pressure_max;
    PenState last_state;
    
    void sendEvent(uint16_t type, uint16_t code, int32_t value);
    void setupAxes();
    void setupKeys();
};

} // namespace t501

#pragma once

#include <cstdint>
#include <iostream>
#include <iomanip>

namespace t501 {
    
    struct PenEvent {
        uint8_t report_id;
        uint8_t buttons;
        uint16_t x;
        uint16_t y;
        uint16_t pressure;
        bool is_down;
    };

    inline PenEvent parsePenEvent(const unsigned char* data, int offset = 0) {
        PenEvent event;
        event.report_id = data[offset + 0];
        event.buttons = data[offset + 1];
        event.x = data[offset + 2] | (data[offset + 3] << 8);
        event.y = data[offset + 4] | (data[offset + 5] << 8);
        event.pressure = data[offset + 6] | (data[offset + 7] << 8);
        event.is_down = (event.buttons == 0xC1) || (event.pressure > 0);
        return event;
    }

    inline void printHexDump(const unsigned char* data, int length, const char* prefix = "") {
        std::cout << prefix;
        for (int i = 0; i < length; i++) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') 
                      << (int)data[i] << " ";
            if ((i + 1) % 16 == 0 && i + 1 < length) {
                std::cout << "\n" << prefix;
            }
        }
        std::cout << std::dec << std::endl;
    }

}
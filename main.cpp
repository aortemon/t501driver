#include <signal.h>
#include <iostream>
#include <atomic>
#include <memory>
#include <unistd.h>

#include "usb_reader.hpp"
#include "data_mapper.hpp"
#include "uinput_writer.hpp"
#include "keyboard_mapper.hpp"
#include "mappers/area_mapper.hpp"

std::atomic<bool> running{true};

void signal_handler(int sig) {
    (void)sig;
    running = false;
}

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    auto reader = std::make_unique<t501::USBReader>();
    auto mapper = std::make_unique<t501::DataMapper>();
    auto writer = std::make_unique<t501::UinputWriter>();
    auto keyboard = std::make_unique<t501::KeyboardMapper>();

    if (!reader->initialize()) {
        std::cerr << "Failed to initialize USB reader" << std::endl;
        return 1;
    }

    if (!reader->setFullMode()) {
        std::cerr << "Failed to set full mode" << std::endl;
        return 1;
    }

    mapper->getAreaMapper()->setTabletArea(t501::mappers::Area{0, 0, 4096, 4096});
    mapper->getCoordinateMapper()->setSourceArea(0, 0, 4096, 4096);
    mapper->getAreaMapper()->setMonitorArea(t501::mappers::Area{0, 0, 4096, 4096});
    mapper->getCoordinateMapper()->setTargetArea(0, 0, 4096, 4096);

    if (!writer->initialize("T501 Pen")) {
        std::cerr << "Failed to initialize pen writer" << std::endl;
        return 1;
    }

    keyboard->loadDefaultMapping();
    keyboard->addRepeatableKey(t501::SCROLL_UP);
    keyboard->addRepeatableKey(t501::SCROLL_DOWN);
    keyboard->addRepeatableKey(KEY_E);

    keyboard->setHoldThreshold(300);
    keyboard->setRepeatInterval(100);
    keyboard->start();

    std::cout << "Tablet keyboard mapping started" << std::endl;

    std::cout << "Driver started. Press Ctrl+C to stop." << std::endl;

    
    while (running) {
        auto raw_data = reader->readData(100);
        
        if (raw_data.valid) {
            
            auto state = mapper->mapRawData(raw_data.data);
            writer->writeState(state);
            
            if (raw_data.data.size() > 12) {
                uint16_t tablet_buttons = (raw_data.data[12] << 8) | raw_data.data[11];
                keyboard->processTabletButtons(tablet_buttons);
            }
        }
    }

    keyboard->stop();
    writer->shutdown();
    reader->shutdown();

    std::cout << "Driver stopped." << std::endl;
    return 0;
}
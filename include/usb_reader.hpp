#pragma once

#include <libusb.h>
#include <vector>
#include <cstdint>
#include <string>

namespace t501 {

struct RawData {
    std::vector<uint8_t> data;
    bool valid = false;
};

class USBReader {
public:
    USBReader(uint16_t vendor_id = 0x08f2, uint16_t product_id = 0x6811);
    ~USBReader();

    bool initialize();
    bool setFullMode();
    RawData readData(int timeout_ms = 100);
    RawData readButtonData(int timeout_ms = 100);
    void shutdown();
    
    std::string getDeviceInfo() const;
    bool isConnected() const { return dev_handle != nullptr; }
    
    int getPenEndpoint() const { return pen_endpoint; }
    int getButtonEndpoint() const { return button_endpoint; }

private:
    uint16_t vendor_id;
    uint16_t product_id;
    
    libusb_context* ctx = nullptr;
    libusb_device_handle* dev_handle = nullptr;
    libusb_device* dev = nullptr;
    
    int pen_endpoint = 0x83;
    int button_endpoint = 0x85;
    int pen_interface = 1;
    int button_interface = 2;
    
    bool findEndpoints();
    bool claimInterfaces();
    void releaseInterfaces();
};

} // namespace t501
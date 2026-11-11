#include "usb_reader.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>

namespace t501 {

USBReader::USBReader(uint16_t vid, uint16_t pid) 
    : vendor_id(vid), product_id(pid) {
}

USBReader::~USBReader() {
    shutdown();
}

bool USBReader::initialize() {
    int err = libusb_init(&ctx);
    if (err < 0) {
        std::cerr << "Failed to initialize libusb: " << libusb_error_name(err) << std::endl;
        return false;
    }

    dev_handle = libusb_open_device_with_vid_pid(ctx, vendor_id, product_id);
    if (!dev_handle) {
        std::cerr << "Device not found" << std::endl;
        libusb_exit(ctx);
        ctx = nullptr;
        return false;
    }

    dev = libusb_get_device(dev_handle);
    
    if (!findEndpoints()) {
        std::cerr << "Failed to find endpoints" << std::endl;
        shutdown();
        return false;
    }

    libusb_set_auto_detach_kernel_driver(dev_handle, 1);
    
    for (int i = 0; i <= 2; i++) {
        err = libusb_detach_kernel_driver(dev_handle, i);
        if (err == LIBUSB_SUCCESS || err == LIBUSB_ERROR_NOT_FOUND) {
            err = libusb_claim_interface(dev_handle, i);
            if (err == LIBUSB_SUCCESS) {
                std::cout << "Claimed interface " << i << std::endl;
            }
        }
    }

    return true;
}

bool USBReader::findEndpoints() {
    struct libusb_config_descriptor* config;
    int err = libusb_get_active_config_descriptor(dev, &config);
    if (err < 0) {
        return false;
    }

    for (int i = 0; i < config->bNumInterfaces; i++) {
        const struct libusb_interface* iface = &config->interface[i];
        for (int j = 0; j < iface->num_altsetting; j++) {
            const struct libusb_interface_descriptor* iface_desc = &iface->altsetting[j];
            
            for (int k = 0; k < iface_desc->bNumEndpoints; k++) {
                const struct libusb_endpoint_descriptor* ep = &iface_desc->endpoint[k];
                
                if ((ep->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) == LIBUSB_TRANSFER_TYPE_INTERRUPT &&
                    (ep->bEndpointAddress & LIBUSB_ENDPOINT_IN)) {
                    
                    if (ep->wMaxPacketSize == 64) {
                        pen_endpoint = ep->bEndpointAddress;
                        pen_interface = i;
                        std::cout << "Pen endpoint: 0x" << std::hex << (int)pen_endpoint 
                                  << std::dec << " on interface " << i << std::endl;
                    } else if (ep->wMaxPacketSize == 8) {
                        button_endpoint = ep->bEndpointAddress;
                        button_interface = i;
                        std::cout << "Button endpoint: 0x" << std::hex << (int)button_endpoint 
                                  << std::dec << " on interface " << i << std::endl;
                    }
                }
            }
        }
    }

    libusb_free_config_descriptor(config);
    return pen_endpoint != 0;
}

bool USBReader::setFullMode() {
    if (!dev_handle) return false;
    
    unsigned char report[8] = {0x08, 0x03, 0x00, 0xff, 0xf0, 0x00, 0xff, 0xf0};
    
    int err = libusb_control_transfer(
        dev_handle,
        0x21,
        0x9,
        0x0308,
        pen_interface,
        report,
        sizeof(report),
        250
    );
    
    if (err < 0) {
        std::cerr << "Failed to set full mode: " << libusb_error_name(err) << std::endl;
        return false;
    }
    
    std::cout << "Full mode set successfully" << std::endl;
    return true;
}

RawData USBReader::readData(int timeout_ms) {
    RawData result;
    result.data.resize(64);
    int transferred = 0;
    
    int err = libusb_interrupt_transfer(
        dev_handle,
        pen_endpoint,
        result.data.data(),
        result.data.size(),
        &transferred,
        timeout_ms
    );
    
    if (err == LIBUSB_SUCCESS && transferred > 0) {
        result.data.resize(transferred);
        result.valid = true;
    } else if (err == LIBUSB_ERROR_TIMEOUT) {
        result.valid = false;
    } else if (err < 0) {
        result.valid = false;
        if (err != LIBUSB_ERROR_INTERRUPTED) {
            std::cerr << "Transfer error: " << libusb_error_name(err) << std::endl;
        }
    }
    
    return result;
}

RawData USBReader::readButtonData(int timeout_ms) {
    RawData result;
    result.data.resize(8);
    int transferred = 0;
    
    if (button_endpoint == 0) {
        result.valid = false;
        return result;
    }
    
    int err = libusb_interrupt_transfer(
        dev_handle,
        button_endpoint,
        result.data.data(),
        result.data.size(),
        &transferred,
        timeout_ms
    );
    
    if (err == LIBUSB_SUCCESS && transferred > 0) {
        result.data.resize(transferred);
        result.valid = true;
    } else if (err == LIBUSB_ERROR_TIMEOUT) {
        result.valid = false;
    } else if (err < 0) {
        result.valid = false;
    }
    
    return result;
}

void USBReader::shutdown() {
    if (dev_handle) {
        for (int i = 0; i <= 2; i++) {
            libusb_release_interface(dev_handle, i);
        }
        libusb_close(dev_handle);
        dev_handle = nullptr;
    }
    
    if (ctx) {
        libusb_exit(ctx);
        ctx = nullptr;
    }
}

std::string USBReader::getDeviceInfo() const {
    if (!dev_handle) return "Not connected";
    
    unsigned char desc[256];
    int err = libusb_get_string_descriptor_ascii(
        dev_handle, 1, desc, sizeof(desc)
    );
    
    if (err > 0) {
        return std::string(reinterpret_cast<char*>(desc), err);
    }
    
    return "Unknown device";
}

} // namespace t501
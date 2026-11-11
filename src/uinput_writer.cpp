#include "uinput_writer.hpp"
#include <iostream>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>

namespace t501 {

UinputWriter::UinputWriter(int width, int height, int max_pressure)
    : tablet_width(width), tablet_height(height), pressure_max(max_pressure) {
}

UinputWriter::~UinputWriter() {
    shutdown();
}

bool UinputWriter::initialize(const std::string& device_name) {
    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        fd = open("/dev/input/uinput", O_WRONLY | O_NONBLOCK);
        if (fd < 0) {
            std::cerr << "Cannot open uinput: " << strerror(errno) << std::endl;
            return false;
        }
    }

    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_EVBIT, EV_ABS);
    ioctl(fd, UI_SET_EVBIT, EV_SYN);
    ioctl(fd, UI_SET_EVBIT, EV_MSC);

    ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT);
    ioctl(fd, UI_SET_KEYBIT, BTN_MIDDLE);
    
    ioctl(fd, UI_SET_KEYBIT, BTN_TOUCH);
    ioctl(fd, UI_SET_KEYBIT, BTN_TOOL_PEN);
    ioctl(fd, UI_SET_KEYBIT, BTN_STYLUS);
    ioctl(fd, UI_SET_KEYBIT, BTN_STYLUS2);
    ioctl(fd, UI_SET_KEYBIT, BTN_TOOL_RUBBER);
    ioctl(fd, UI_SET_KEYBIT, BTN_TOOL_BRUSH);
    ioctl(fd, UI_SET_KEYBIT, BTN_TOOL_PENCIL);
    ioctl(fd, UI_SET_KEYBIT, BTN_TOOL_AIRBRUSH);
    ioctl(fd, UI_SET_KEYBIT, BTN_TOOL_FINGER);
    ioctl(fd, UI_SET_KEYBIT, BTN_TOOL_MOUSE);
    ioctl(fd, UI_SET_KEYBIT, BTN_TOOL_LENS);

    ioctl(fd, UI_SET_ABSBIT, ABS_X);
    ioctl(fd, UI_SET_ABSBIT, ABS_Y);
    ioctl(fd, UI_SET_ABSBIT, ABS_PRESSURE);
    ioctl(fd, UI_SET_ABSBIT, ABS_DISTANCE);
    ioctl(fd, UI_SET_ABSBIT, ABS_TILT_X);
    ioctl(fd, UI_SET_ABSBIT, ABS_TILT_Y);
    ioctl(fd, UI_SET_ABSBIT, ABS_MISC);

    ioctl(fd, UI_SET_PROPBIT, INPUT_PROP_POINTER);
    ioctl(fd, UI_SET_PROPBIT, INPUT_PROP_DIRECT);

    struct uinput_abs_setup abs_setup;
    memset(&abs_setup, 0, sizeof(abs_setup));

    abs_setup.code = ABS_X;
    abs_setup.absinfo.minimum = 0;
    abs_setup.absinfo.maximum = tablet_width - 1;
    abs_setup.absinfo.flat = 0;
    abs_setup.absinfo.fuzz = 0;
    abs_setup.absinfo.resolution = 100;
    ioctl(fd, UI_ABS_SETUP, &abs_setup);

    abs_setup.code = ABS_Y;
    abs_setup.absinfo.minimum = 0;
    abs_setup.absinfo.maximum = tablet_height - 1;
    abs_setup.absinfo.resolution = 100;
    ioctl(fd, UI_ABS_SETUP, &abs_setup);

    abs_setup.code = ABS_PRESSURE;
    abs_setup.absinfo.minimum = 0;
    abs_setup.absinfo.maximum = pressure_max - 1;
    abs_setup.absinfo.resolution = 1;
    ioctl(fd, UI_ABS_SETUP, &abs_setup);

    abs_setup.code = ABS_DISTANCE;
    abs_setup.absinfo.minimum = 0;
    abs_setup.absinfo.maximum = 1;
    abs_setup.absinfo.resolution = 1;
    ioctl(fd, UI_ABS_SETUP, &abs_setup);

    abs_setup.code = ABS_TILT_X;
    abs_setup.absinfo.minimum = -90;
    abs_setup.absinfo.maximum = 90;
    abs_setup.absinfo.flat = 0;
    abs_setup.absinfo.fuzz = 0;
    abs_setup.absinfo.resolution = 1;
    ioctl(fd, UI_ABS_SETUP, &abs_setup);

    abs_setup.code = ABS_TILT_Y;
    abs_setup.absinfo.minimum = -90;
    abs_setup.absinfo.maximum = 90;
    abs_setup.absinfo.flat = 0;
    abs_setup.absinfo.fuzz = 0;
    abs_setup.absinfo.resolution = 1;
    ioctl(fd, UI_ABS_SETUP, &abs_setup);

    struct uinput_setup usetup;
    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x08f2;
    usetup.id.product = 0x6811;
    usetup.id.version = 1;
    strncpy(usetup.name, device_name.c_str(), UINPUT_MAX_NAME_SIZE - 1);

    if (ioctl(fd, UI_DEV_SETUP, &usetup) < 0) {
        std::cerr << "Failed to setup uinput: " << strerror(errno) << std::endl;
        shutdown();
        return false;
    }

    if (ioctl(fd, UI_DEV_CREATE) < 0) {
        std::cerr << "Failed to create uinput device: " << strerror(errno) << std::endl;
        shutdown();
        return false;
    }

    last_state = PenState();
    
    sendEvent(EV_ABS, ABS_X, 0);
    sendEvent(EV_ABS, ABS_Y, 0);
    sendEvent(EV_ABS, ABS_PRESSURE, 0);
    sendEvent(EV_ABS, ABS_DISTANCE, 1);
    sendEvent(EV_ABS, ABS_TILT_X, 0);
    sendEvent(EV_ABS, ABS_TILT_Y, 0);
    sendEvent(EV_KEY, BTN_TOOL_PEN, 1);
    sendEvent(EV_KEY, BTN_TOUCH, 0);
    sendEvent(EV_SYN, SYN_REPORT, 0);

    std::cout << "Uinput device created: " << device_name << std::endl;
    return true;
}

void UinputWriter::sendEvent(uint16_t type, uint16_t code, int32_t value) {
    if (fd < 0) return;
    
    struct input_event ev;
    memset(&ev, 0, sizeof(ev));
    gettimeofday(&ev.time, nullptr);
    ev.type = type;
    ev.code = code;
    ev.value = value;
    
    if (write(fd, &ev, sizeof(ev)) < 0) {
        std::cerr << "Failed to send event: " << strerror(errno) << std::endl;
    }
}

void UinputWriter::writeState(const PenState& state) {
    if (fd < 0) return;

    sendEvent(EV_ABS, ABS_X, state.x);
    sendEvent(EV_ABS, ABS_Y, state.y);
    sendEvent(EV_ABS, ABS_PRESSURE, state.pressure);
    sendEvent(EV_ABS, ABS_DISTANCE, state.is_down ? 0 : 1);
    
    sendEvent(EV_KEY, BTN_TOOL_PEN, 1); 
    sendEvent(EV_KEY, BTN_TOUCH, state.is_down ? 1 : 0);
    sendEvent(EV_KEY, BTN_STYLUS, state.left_button ? 1 : 0);
    sendEvent(EV_KEY, BTN_STYLUS2, state.right_button ? 1 : 0);
    
    sendEvent(EV_SYN, SYN_REPORT, 0);
    
    last_state = state;
}

void UinputWriter::shutdown() {
    if (fd >= 0) {
        ioctl(fd, UI_DEV_DESTROY);
        close(fd);
        fd = -1;
    }
}

} // namespace t501
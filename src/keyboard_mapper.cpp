#include "keyboard_mapper.hpp"
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/uinput.h>

namespace t501 {

KeyboardMapper::KeyboardMapper() = default;

KeyboardMapper::~KeyboardMapper() {
    stop();
}

bool KeyboardMapper::createVirtualKeyboard() {
    uinput_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (uinput_fd < 0) {
        std::cerr << "Cannot open uinput for keyboard" << std::endl;
        return false;
    }
    
    ioctl(uinput_fd, UI_SET_EVBIT, EV_KEY);
    ioctl(uinput_fd, UI_SET_EVBIT, EV_REL);
    ioctl(uinput_fd, UI_SET_EVBIT, EV_SYN);
    
    for (int i = 0; i < KEY_MAX; i++) {
        ioctl(uinput_fd, UI_SET_KEYBIT, i);
    }
    
    ioctl(uinput_fd, UI_SET_RELBIT, REL_WHEEL);
    ioctl(uinput_fd, UI_SET_RELBIT, REL_WHEEL_HI_RES);
    
    struct uinput_setup usetup;
    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x08f2;
    usetup.id.product = 0x6811;
    usetup.id.version = 1;
    strcpy(usetup.name, "Gotop T501 Keyboard");
    
    if (ioctl(uinput_fd, UI_DEV_SETUP, &usetup) < 0 ||
        ioctl(uinput_fd, UI_DEV_CREATE) < 0) {
        std::cerr << "Failed to create virtual keyboard" << std::endl;
        close(uinput_fd);
        uinput_fd = -1;
        return false;
    }
    
    std::cout << "Virtual keyboard created" << std::endl;
    return true;
}

void KeyboardMapper::loadDefaultMapping() {
    key_mapping.clear();
    repeatable_keys.clear();
    
    key_mapping[0x31FF] = KEY_E;
    key_mapping[0x23FF] = KEY_B;
    key_mapping[0x337F] = KEY_LEFTCTRL;
    key_mapping[0x32FF] = KEY_LEFTCTRL;
    key_mapping[0x33BF] = KEY_LEFTBRACE;
    key_mapping[0x13FF] = KEY_RIGHTBRACE;
    key_mapping[0x33DF] = SCROLL_UP;
    key_mapping[0x33EF] = SCROLL_DOWN;
    key_mapping[0x33FE] = KEY_TAB;
    key_mapping[0x33FD] = KEY_SPACE;
    key_mapping[0x33F7] = KEY_LEFTCTRL;
    key_mapping[0x33FB] = KEY_LEFTALT;
}

void KeyboardMapper::addRepeatableKey(int key_code) {
    repeatable_keys.insert(key_code);
    std::cout << "Action " << key_code << " added to repeatable" << std::endl;
}

void KeyboardMapper::removeRepeatableKey(int key_code) {
    repeatable_keys.erase(key_code);
    std::cout << "Action " << key_code << " removed from repeatable" << std::endl;
}

void KeyboardMapper::clearRepeatableKeys() {
    repeatable_keys.clear();
    std::cout << "All repeatable actions cleared" << std::endl;
}

bool KeyboardMapper::isKeyRepeatable(int key_code) const {
    return repeatable_keys.find(key_code) != repeatable_keys.end();
}

void KeyboardMapper::setKeyMapping(int original_key, int new_key) {
    key_mapping[original_key] = new_key;
}

void KeyboardMapper::sendKey(int key_code, bool pressed) {
    if (uinput_fd < 0) return;
    
    struct input_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_KEY;
    ev.code = key_code;
    ev.value = pressed ? 1 : 0;
    write(uinput_fd, &ev, sizeof(ev));
    
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(uinput_fd, &ev, sizeof(ev));
}

void KeyboardMapper::sendScroll(int direction, int steps) {
    if (uinput_fd < 0) return;
    
    struct input_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_REL;
    ev.code = REL_WHEEL;
    ev.value = direction;
    write(uinput_fd, &ev, sizeof(ev));
    
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_REL;
    ev.code = REL_WHEEL_HI_RES;
    ev.value = direction * 120 / steps;
    write(uinput_fd, &ev, sizeof(ev));
    
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(uinput_fd, &ev, sizeof(ev));
}

void KeyboardMapper::executeAction(int action) {
    if (action == SCROLL_UP) {
        sendScroll(1);
    } else if (action == SCROLL_DOWN) {
        sendScroll(-1);
    } else {
        sendKey(action, true);
    }
}

void KeyboardMapper::releaseHeldAction() {
    if (action_held) {
        if (held_action != SCROLL_UP && held_action != SCROLL_DOWN) {
            sendKey(held_action, false);
        }
        action_held = false;
        held_action = 0;
    }
}

void KeyboardMapper::start() {
    if (!createVirtualKeyboard()) return;
    running = true;
    std::cout << "Keyboard mapper started" << std::endl;
}

void KeyboardMapper::stop() {
    running = false;
    if (uinput_fd >= 0) {
        ioctl(uinput_fd, UI_DEV_DESTROY);
        close(uinput_fd);
        uinput_fd = -1;
    }
}

void KeyboardMapper::processTabletButtons(uint16_t button_state) {
    const uint16_t RESET_STATE = 0x33FF;
    
    if (action_held) {
        auto now = std::chrono::steady_clock::now();
        auto held_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - press_time
        ).count();
        
        if (held_duration >= hold_threshold_ms) {
            auto repeat_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - last_repeat_time
            ).count();
            
            if (repeat_elapsed >= repeat_interval_ms) {
                if (held_action == SCROLL_UP || held_action == SCROLL_DOWN) {
                    executeAction(held_action);
                } else {
                    sendKey(held_action, false);
                    sendKey(held_action, true);
                }
                last_repeat_time = now;
                
                std::cout << "Repeat: " << held_action << std::endl;
            }
        }
    }
    
    if (button_state == last_button_state) {
        return;
    }


    if (button_state == RESET_STATE) {
        releaseHeldAction();
        last_button_state = RESET_STATE;
        return;
    }
    
    auto it = key_mapping.find(button_state);
    if (it != key_mapping.end()) {
        releaseHeldAction();
        
        int action = it->second;
        bool is_repeatable = isKeyRepeatable(action);
        
        if (is_repeatable) {
            executeAction(action);
            action_held = true;
            held_action = action;
            press_time = std::chrono::steady_clock::now();
            last_repeat_time = press_time;
            
            std::cout << "Action started (repeatable): " << action << std::endl;
        } else {
            if (action == SCROLL_UP) {
                sendScroll(1);
            } else if (action == SCROLL_DOWN) {
                sendScroll(-1);
            } else {
                sendKey(action, true);
                sendKey(action, false);
            }
            
            action_held = false;
            held_action = 0;
            
            std::cout << "Action (single): " << action << std::endl;
        }
    }
    
    last_button_state = button_state;
}

} // namespace t501
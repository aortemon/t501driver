#pragma once

#include <map>
#include <set>
#include <atomic>
#include <chrono>
#include <linux/input.h>

namespace t501 {

constexpr int SCROLL_UP = 100000;
constexpr int SCROLL_DOWN = 100001;

class KeyboardMapper {
public:
    KeyboardMapper();
    ~KeyboardMapper();
    
    void setKeyMapping(int original_key, int new_key);
    void loadDefaultMapping();
    
    void addRepeatableKey(int key_code);
    void removeRepeatableKey(int key_code);
    void clearRepeatableKeys();
    bool isKeyRepeatable(int key_code) const;
    
    void start();
    void stop();
    
    void sendKey(int key_code, bool pressed);
    void sendScroll(int direction, int steps = 1);
    void processTabletButtons(uint16_t button_state);
    
    void setHoldThreshold(int ms) { hold_threshold_ms = ms; }
    void setRepeatInterval(int ms) { repeat_interval_ms = ms; }
    
private:
    int uinput_fd = -1;
    std::atomic<bool> running{false};
    std::map<int, int> key_mapping;
    std::set<int> repeatable_keys;
    
    uint16_t last_button_state = 0x33FF;
    bool action_held = false;
    int held_action = 0;
    std::chrono::steady_clock::time_point press_time;
    std::chrono::steady_clock::time_point last_repeat_time;
    
    int hold_threshold_ms = 300;
    int repeat_interval_ms = 100;
    
    bool createVirtualKeyboard();
    void executeAction(int action);
    void releaseHeldAction();
};

} // namespace t501
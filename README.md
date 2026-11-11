# T501 Tablet Driver for Linux

A userspace C++17 driver for the **Gotop Information Inc T501** graphics tablet (`08f2:6811`) on Linux.

Linux binds this tablet in a Windows-compatibility mode ("Tablet driver Inside for Windows"), which exposes only half of the active area and leaves the tablet's hardware keys and stylus buttons non-functional. This driver talks to the device directly, giving you full control over the working area and key mapping.

## Why this exists

Under the default Linux HID driver, the T501 is recognized in a compatibility mode that:

- limits the usable drawing area to roughly **50%** of the physical tablet surface;
- does **not** report the tablet's hardware keys;
- does **not** report the stylus buttons.

This userspace driver bypasses that mode and exposes the full device.

## Features

- **Full active area** — uses the entire tablet surface instead of the half exposed in compatibility mode
- **Custom active area size** — define any sub-region of the tablet as the drawing area
- **Monitor mapping** — map the tablet to a portion of a single monitor, or across **multiple monitors** at once
- **Key remapping** — rebind the tablet's hardware keys and the stylus buttons to arbitrary actions
- **Userspace only** — no kernel module, no patching, builds with C++17 and CMake

## Supported hardware

| Model | VID:PID | Status |
|-------|---------|--------|
| Gotop Information Inc T501 | `08f2:6811` | supported |

> **Note:** A USB 2.0 port is recommended.

## Requirements

### Runtime dependencies

- **Linux** (uses `libusb-1.0`, `/dev/uinput`, and Linux input subsystem headers)
- **libusb-1.0** — USB communication with the tablet
- **`/dev/uinput`** — the kernel `uinput` module must be loaded (used for the virtual pen
  tablet and the virtual keyboard)
- **Root privileges** (or equivalent udev/permissions setup) — the driver detaches kernel
  drivers, claims the USB interfaces, and opens `/dev/uinput`

### Build dependencies

- **C++17** compiler (GCC 8+ or Clang 7+)
- **CMake 3.10+**
- **pkg-config**
- **libusb-1.0 development headers**
- Linux kernel headers (for `linux/uinput.h`, `linux/input.h`)
- **X11 / `xrandr`** — only needed at runtime for automatic monitor detection

## Build and run

```bash
git clone https://github.com/aortemon/t501driver
cd t501driver
mkdir build
cd build
cmake ..
cmake --build .
sudo ./t501driver
```

## Avoiding running as root
if you want users to run the driver without sudo, add a udev rule here —
e.g. /etc/udev/rules.d/99-t501.rules with
```txt 
SUBSYSTEM=="usb", ATTR{idVendor}=="08f2", ATTR{idProduct}=="6811", MODE="0666"
```
and/or a rule granting access to /dev/uinput to a uinput group.
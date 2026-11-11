cmake_minimum_required(VERSION 3.10)
project(t501driver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

include_directories(include)

add_executable(t501driver
    main.cpp
    src/usb_reader.cpp
    src/data_mapper.cpp
    src/uinput_writer.cpp
    src/keyboard_mapper.cpp
    src/mappers/coordinate_mapper.cpp
    src/mappers/pressure_mapper.cpp
    src/mappers/monitor_mapper.cpp
    src/mappers/area_mapper.cpp
)

target_include_directories(t501driver PRIVATE ${LIBUSB_INCLUDE_DIRS})
target_link_libraries(t501driver PRIVATE ${LIBUSB_LIBRARIES})
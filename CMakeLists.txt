cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)
project(clock)
pico_sdk_init()

# add a compilation subdirectory
add_subdirectory(lib/Config)
add_subdirectory(lib/e-Paper)
add_subdirectory(lib/Fonts)
add_subdirectory(lib/GUI)
add_subdirectory(examples)

# add a header directory
include_directories(examples)
include_directories(./lib/e-Paper)
include_directories(./lib/Config)
include_directories(./lib/GUI)

# generate an executable file
add_executable(clock 
main.c
)

# enable usb output, disable uart output
pico_enable_stdio_usb(clock 1)
pico_enable_stdio_uart(clock 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(clock)

target_link_libraries(clock examples ePaper GUI Fonts Config pico_stdlib hardware_spi)

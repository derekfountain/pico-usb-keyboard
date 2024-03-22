cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pico_usb_keyboard C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(pico_usb_keyboard
	main.c
        usb_descriptors.c
)

# Make sure TinyUSB can find tusb_config.h
target_include_directories(pico_usb_keyboard PUBLIC
        ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(pico_usb_keyboard
	pico_stdlib
	pico_unique_id
	tinyusb_device
	tinyusb_board)

pico_add_extra_outputs(pico_usb_keyboard)

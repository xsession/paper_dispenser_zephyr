cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Set the board name
# Check out the docs: https://docs.zephyrproject.org/latest/develop/application/index.html#application-cmakelists-txt
set(BOARD rpi_pico)

# Set default console output (ON: USB CDC, OFF: UART)
# Check out the docs: https://docs.zephyrproject.org/latest/develop/west/build-flash-debug.html#permanent-cmake-arguments
set(USE_USB_CONSOLE ON CACHE BOOL "Enable USB CDC console")

# Setting CONF_FILE and DTC_OVERLAY_FILE
# Check out the docs: https://docs.zephyrproject.org/latest/build/kconfig/setting.html#the-initial-configuration
# Check out the docs: https://docs.zephyrproject.org/latest/build/dts/howtos.html#set-devicetree-overlays
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/prj.conf")
  list(APPEND CONF_FILE "prj.conf")
endif()

if(USE_USB_CONSOLE)
  message("-- The console output set to USB")
  list(APPEND CONF_FILE "usb.conf")

  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/app.overlay")
    list(APPEND DTC_OVERLAY_FILE "app.overlay")
  endif()
  list(APPEND DTC_OVERLAY_FILE "usb.overlay")
else()
  message("-- The console output set to UART")
endif()

project(paper_dispenser_zephyr)

#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../deps/cmake_utils")
# include(cmake_utils)

include_directories(libs/interfaces/indicator/inc)
include_directories(libs/interfaces/trigger/inc)
include_directories(inc)

set(libs_dir libs/interfaces)
file(
    GLOB_RECURSE
    interface_files
    CONFIGURE_DEPENDS
    "${libs_dir}/*.[cC]")

if(CONFIG_BUTTON_TRIGGER)
  list(APPEND app_sources src/button_trigger.c)
elseif(CONFIG_TIMER_TRIGGER)
  list(APPEND app_sources src/timer_trigger.c)
endif()

message(STATUS "-------------------------> interface_files: ${interface_files}")

target_sources(app 
                PRIVATE 
                ${app_sources} 
                ${interface_files} 
                src/main.c 
                src/led_indicator.c
                )
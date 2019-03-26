cmake_minimum_required (VERSION 3.6)

if( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Darwin" )
  set(SOS_TOOLCHAIN_CMAKE_PATH /Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/cmake)
endif()
if( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows" )
  set(SOS_TOOLCHAIN_CMAKE_PATH C:/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/lib/ldscripts)
endif()

include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-variables.cmake)

set(CMAKE_TOOLCHAIN_FILE ${SOS_TOOLCHAIN_CMAKE_PATH}/sos-bsp-toolchain.cmake)
get_filename_component(SOS_NAME ${CMAKE_SOURCE_DIR} NAME)
project(${SOS_NAME} CXX C)

#Add sources to the project
set(SOURCES_PREFIX ${CMAKE_SOURCE_DIR}/src)
add_subdirectory(src)
list(APPEND SOS_KERNEL_SOURCELIST ${SOURCES})

#Change to boot sources
set(SOURCES "")
set(SOURCES_PREFIX ${CMAKE_SOURCE_DIR}/boot)
add_subdirectory(boot)
list(APPEND BOOT_SOURCELIST ${SOURCES})

set(SOS_ARCH v7m)
set(SOS_DEVICE lpc1768)
set(SOS_DEVICE_FAMILY lpc17xx)
set(SOS_HARDWARD_ID 0x00000003)

set(SOS_KERNEL_START_ADDRESS 0x40000)
set(SOS_BOOT_START_ADDRESS 0x0)
set(SOS_BOOT_SOURCELIST ${BOOT_SOURCELIST})
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-bsp-std.cmake)

set(SOS_DEFINITIONS __UART=1)
set(SOS_START_ADDRESS 0x40000)
set(SOS_SOURCELIST ${SOS_KERNEL_SOURCELIST})
set(SOS_CONFIG uart)
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-bsp.cmake)


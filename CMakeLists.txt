cmake_minimum_required (VERSION 3.6)

# This will set the default RAM used by the application
set(BUILD_RAM_SIZE 4096 CACHE INTERNAL "ram size")

#Add sources to the project
set(SOURCES_PREFIX ${CMAKE_SOURCE_DIR}/src CACHE INTERNAL "src prefix")
add_subdirectory(src)
list(APPEND SOURCELIST ${SOURCES})

if( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Darwin" )
  set(SOS_TOOLCHAIN_CMAKE_PATH /Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/lib/ldscripts CACHE INTERNAL "sos toolchain path")
  set(SOS_TOOLCHAIN_CMAKE_PATH /Users/tgil/git/StratifyOS/cmake CACHE INTERNAL "sos toolchain path")
endif()
if( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows" )
  set(SOS_TOOLCHAIN_CMAKE_PATH C:/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/lib/ldscripts CACHE INTERNAL "sos toolchain path")
endif()
set(CMAKE_TOOLCHAIN_FILE ${SOS_TOOLCHAIN_CMAKE_PATH}/sos-bsp-toolchain.cmake CACHE INTERNAL "sos toolchain file")
get_filename_component(NAME ${CMAKE_SOURCE_DIR} NAME)
project(${NAME} CXX C)

set(SOS_BSP_CONFIG release CACHE INTERNAL "sos build config release")
set(SOS_BSP_ARCH armv7-m CACHE INTERNAL "sos build ")
set(SOS_BSP_DEVICE lpc1768 CACHE INTERNAL "sos device ")
set(SOS_BSP_DEVICE_FAMILY lpc17xx CACHE INTERNAL "sos device ")
set(SOS_BSP_HARDWARD_ID 0x00000003 CACHE INTERNAL "sos device ")
set(SOS_BSP_START_ADDRESS 0x40000 CACHE INTERNAL "sos device ")
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-bsp.cmake)

set(SOS_BSP_CONFIG debug CACHE INTERNAL "sos build config debug")
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-bsp.cmake)

#Change to boot sources
set(SOURCELIST "")
set(SOURCES_PREFIX ${CMAKE_SOURCE_DIR}/boot CACHE INTERNAL "src prefix")
add_subdirectory(boot)
list(APPEND SOURCELIST ${SOURCES})

set(SOS_BSP_CONFIG release_boot CACHE INTERNAL "sos build config boot")
set(SOS_BSP_START_ADDRESS 0x0 CACHE INTERNAL "sos device ")
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-bsp.cmake)

set(SOS_BSP_CONFIG debug_boot CACHE INTERNAL "sos build config debug boot")
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-bsp.cmake)

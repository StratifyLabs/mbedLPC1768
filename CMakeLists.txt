cmake_minimum_required (VERSION 3.12)

set(SDK_DIRECTORY ${CMAKE_SOURCE_DIR}/SDK)
set(SOS_SDK_PATH ${SDK_DIRECTORY}/StratifyLabs-SDK)

set(CMAKE_MODULE_PATH ${SOS_SDK_PATH}/cmake)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_MODULE_PATH}/sos-toolchain.cmake)
include(sos-sdk)

project(mbedLPC1768 CXX C ASM)
include(CTest)

set(SOS_ARCH v7m)
set(SOS_ARCH_LIST "")

add_custom_target(pull
	COMMAND git pull)

set(SOS_SDK_PULL_TARGET pull)

add_subdirectory(SDK)
add_subdirectory(mbedLPC1768)

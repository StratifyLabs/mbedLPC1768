cmake_minimum_required (VERSION 3.12)

set(ARCH ${SOS_ARCH})
set(MCU lpc17xx)

set(BOOT_LIBRARIES StratifyOS_${MCU} StratifyOS_boot StratifyOS_usbd StratifyOS_${MCU})
set(KERNEL_LIBRARIES
	StratifyOS_${MCU}
	StratifyOS_sys
	StratifyOS_usbd
	newlib_libc
	newlib_libm
	StratifyOS_sys)

get_filename_component(SOS_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${SOS_NAME} CXX C ASM)

#Add sources to the project
sos_sdk_add_subdirectory(KERNEL_SOURCELIST ${CMAKE_CURRENT_SOURCE_DIR}/src)
list(
	APPEND
	SOS_KERNEL_SOURCELIST
	${SOURCES}
	sl_settings.json
	)

#Change to boot sources
sos_sdk_add_subdirectory(BOOT_SOURCELIST ${CMAKE_CURRENT_SOURCE_DIR}/boot)


set(SOS_ARCH v7em_f4sh)
set(SOS_DEVICE lpc1768)
set(SOS_DEVICE_FAMILY lpc17xx)
set(HARDWARE_ID 0x00000003)
set(KERNEL_START_ADDRESS 0x40000)
set(BOOT_START_ADDRESS 0x0)
set(KERNEL_LINKER_FILE lpc1768-rom.ld)
set(BOOT_LINKER_FILE lpc1768-rom.ld)

sos_sdk_bsp_target(RELEASE ${PROJECT_NAME} "" release ${ARCH})
sos_sdk_bsp_target(DEBUG ${PROJECT_NAME} "" debug ${ARCH})

add_executable(${RELEASE_TARGET})

target_sources(${RELEASE_TARGET}
	PUBLIC
	${KERNEL_SOURCELIST}
	)
target_include_directories(${RELEASE_TARGET}
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/../src
	${CMAKE_CURRENT_SOURCE_DIR}/src
	)
target_compile_definitions(${RELEASE_TARGET}
	PUBLIC
	MCU_ARCH_LPC
	__BOOT_START_ADDRESS=${BOOT_START_ADDRESS}
	__KERNEL_START_ADDRESS=${KERNEL_START_ADDRESS}
	${DEFINITIONS}
	)
target_compile_options(${RELEASE_TARGET} PUBLIC -Os)
set_target_properties(${RELEASE_TARGET}
	PROPERTIES
	LINK_FLAGS
	"-u symbols_table -T${KERNEL_LINKER_FILE}"
	)

target_link_directories(${RELEASE_TARGET}
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/ldscripts)

add_executable(${DEBUG_TARGET})
sos_sdk_copy_target(${RELEASE_TARGET} ${DEBUG_TARGET})


sos_sdk_bsp_target(BOOT_RELEASE ${PROJECT_NAME} boot release ${ARCH})
sos_sdk_bsp_target(BOOT_DEBUG ${PROJECT_NAME} boot debug ${ARCH})

add_executable(${BOOT_RELEASE_TARGET})

target_sources(${BOOT_RELEASE_TARGET}
	PUBLIC
	${BOOT_SOURCELIST}
	)

target_include_directories(${BOOT_RELEASE_TARGET}
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/src
	)

target_compile_definitions(${BOOT_RELEASE_TARGET}
	PUBLIC
	${DEFINITIONS}
	__BOOT_START_ADDRESS=${BOOT_START_ADDRESS}
	__KERNEL_START_ADDRESS=${KERNEL_START_ADDRESS}
	_IS_BOOT=1
	)
target_compile_options(${BOOT_RELEASE_TARGET} PUBLIC -Os)
set_target_properties(${BOOT_RELEASE_TARGET}
	PROPERTIES
	LINK_FLAGS
	"-u _main -T${BOOT_LINKER_FILE} "
	)

target_link_directories(${BOOT_RELEASE_TARGET}
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/ldscripts)


add_executable(${BOOT_DEBUG_TARGET})
sos_sdk_copy_target(${BOOT_RELEASE_TARGET} ${BOOT_DEBUG_TARGET})

sos_sdk_bsp("${BOOT_RELEASE_OPTIONS}" ${HARDWARE_ID} ${BOOT_START_ADDRESS} "${BOOT_LIBRARIES}")
sos_sdk_bsp("${BOOT_DEBUG_OPTIONS}" ${HARDWARE_ID} ${BOOT_START_ADDRESS} "${BOOT_LIBRARIES}")

sos_sdk_bsp("${RELEASE_OPTIONS}" ${HARDWARE_ID} ${KERNEL_START_ADDRESS} "${KERNEL_LIBRARIES}")
sos_sdk_bsp("${DEBUG_OPTIONS}" ${HARDWARE_ID} ${KERNEL_START_ADDRESS} "${KERNEL_LIBRARIES}")



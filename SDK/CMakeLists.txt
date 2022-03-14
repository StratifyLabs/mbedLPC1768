cmake_minimum_required (VERSION 3.19)

project(Nucleo144-SDK)

set(API_IS_SDK TRUE CACHE INTERNAL "Build SDK projects")
option(IS_PULL "Pull SDK projects" ON)

set(BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dependencies)
file(MAKE_DIRECTORY ${BASE_DIRECTORY})

message(STATUS "Base directory ${BASE_DIRECTORY}")

#if INSTALL=ON the library will be staged to be part of an application SDK deployment

set(GIT_STATUS_DEPENDS "")

macro(sdk_add_subdirectory NAME BRANCH INSTALL)

	if(IS_PULL OR NOT EXISTS ${BASE_DIRECTORY}/${NAME})
		sos_sdk_git_clone_or_pull_branch(${BASE_DIRECTORY} ${NAME} https://github.com/StratifyLabs/${NAME}.git ${BRANCH})
	endif()

	if(${INSTALL})
		message(STATUS "Adding ${NAME} to install target")
		add_subdirectory(${BASE_DIRECTORY}/${NAME} ${NAME})
	else()
		message(STATUS "Excluding ${NAME} from install target")
		add_subdirectory(${BASE_DIRECTORY}/${NAME} ${NAME} EXCLUDE_FROM_ALL)
	endif()

endmacro()


set(API_IS_SDK ON CACHE INTERNAL "API_IS_SDK disable include statemenets")

sdk_add_subdirectory(API main ON)
include(${BASE_DIRECTORY}/API/libraries/API.cmake)

set(SOS_CONFIG_PATH ${CMAKE_CURRENT_SOURCE_DIR}/config CACHE INTERNAL "StratifyOS config directory")
set(LPC_CONFIG_PATH ${CMAKE_CURRENT_SOURCE_DIR}/config CACHE INTERNAL "LPC config path")
set(LWIP_CONFIG_PATH ${CMAKE_CURRENT_SOURCE_DIR}/config CACHE INTERNAL "LWIP config path")

set(STRATIFYOS_LWIP_PATH
	${CMAKE_CURRENT_SOURCE_DIR}/dependencies/InetAPI/lwip/include
	${CMAKE_CURRENT_SOURCE_DIR}/dependencies/InetAPI/lwip/lwip-2.1.2/src/include
	CACHE INTERNAL "StratifyOS LWIP INCLUDE PATHS")

set(LWIP_STRATIFYOS_PATH
	${CMAKE_CURRENT_SOURCE_DIR}/dependencies/StratifyOS/include
	CACHE INTERNAL "LWIP StratifyOS INCLUDE PATHS")

set(SOS_BOOTSTRAP_SOCKETS ON CACHE INTERNAL "LPC config path")

set(BUILD_LPC_ALL OFF CACHE INTERNAL "DONT BUILD ALL LPC BUILDS")
set(BUILD_LPC17XX ON CACHE INTERNAL "ENABLE BUILDING OF LPC17XX")
set(IS_MBEDTLS OFF CACHE INTERNAL "DISABLE MBEDTLS")
set(IS_LWIP ON CACHE INTERNAL "Enable LWIP")

sdk_add_subdirectory(StratifyOS-compiler-rt v1.0 OFF)
sdk_add_subdirectory(StratifyOS-newlib v1.0 OFF)
sdk_add_subdirectory(StratifyOS main ON)
sdk_add_subdirectory(StratifyOS-mcu-lpc main OFF)
sdk_add_subdirectory(HalAPI main ON)
sdk_add_subdirectory(JsonAPI main ON)
sdk_add_subdirectory(InetAPI main ON)
sdk_add_subdirectory(CryptoAPI main ON)
sdk_add_subdirectory(SosAPI main ON)
sdk_add_subdirectory(ChartAPI main ON)
sdk_add_subdirectory(UxAPI main ON)
sdk_add_subdirectory(DspAPI main ON)
sdk_add_subdirectory(InstrumentationAPI main ON)

add_custom_target(status DEPENDS ${GIT_STATUS_DEPENDS})

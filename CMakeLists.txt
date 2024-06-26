############################################################################
# Copyright (c) 2016, Johan Mabille, Sylvain Corlay, Martin Renou          #
# Copyright (c) 2016, QuantStack                                           #
#                                                                          #
# Distributed under the terms of the BSD 3-Clause License.                 #
#                                                                          #
# The full license is in the file LICENSE, distributed with this software. #
############################################################################

cmake_minimum_required(VERSION 3.8)
project(xeus-zmq)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
set(XEUS_ZMQ_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(XEUS_ZMQ_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(XEUS_ZMQ_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)

# Versionning
# ===========

# Project version
file(STRINGS "${XEUS_ZMQ_INCLUDE_DIR}/xeus-zmq/xeus-zmq.hpp" xeus_zmq_version_defines
     REGEX "#define XEUS_ZMQ_VERSION_(MAJOR|MINOR|PATCH)")
foreach(ver ${xeus_zmq_version_defines})
    if(ver MATCHES "#define XEUS_ZMQ_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
        set(XEUS_ZMQ_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
endforeach()
set(XEUS_ZMQ_VERSION
    ${XEUS_ZMQ_VERSION_MAJOR}.${XEUS_ZMQ_VERSION_MINOR}.${XEUS_ZMQ_VERSION_PATCH})
message(STATUS "xeus-zmq version: v${XEUS_ZMQ_VERSION}")

# Binary version
# See the following URL for explanations about the binary versionning
# https://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html#Updating-version-info
file(STRINGS "${XEUS_ZMQ_INCLUDE_DIR}/xeus-zmq/xeus-zmq.hpp" xeus_zmq_version_defines
    REGEX "#define XEUS_ZMQ_BINARY_(CURRENT|REVISION|AGE)")
foreach(ver ${xeus_zmq_version_defines})
    if(ver MATCHES "#define XEUS_ZMQ_BINARY_(CURRENT|REVISION|AGE) +([^ ]+)$")
        set(XEUS_ZMQ_BINARY_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
endforeach()
set(XEUS_ZMQ_BINARY_VERSION
    ${XEUS_ZMQ_BINARY_CURRENT}.${XEUS_ZMQ_BINARY_REVISION}.${XEUS_ZMQ_BINARY_AGE})
message(STATUS "xeus-zmq binary version: v${XEUS_ZMQ_BINARY_VERSION}")

# Build options
# =============

# Compilation options
option(XEUS_ZMQ_BUILD_SHARED_LIBS "Build xeus shared library." ON)
option(XEUS_ZMQ_BUILD_STATIC_LIBS "Build xeus static library (default if BUILD_SHARED_LIBS is OFF)." ON)
option(XEUS_ZMQ_STATIC_DEPENDENCIES "link statically with xeus dependencies" OFF)

# Test options
option(XEUS_ZMQ_BUILD_TESTS "xeus test suite" OFF)

# Option to build without libsodium
option(XEUS_ZMQ_BUILD_WITHOUT_LIBSODIUM "Build xeus-zmq without libsodium" OFF)

# Static build configuration
# ==========================

if (XEUS_ZMQ_STATIC_DEPENDENCIES)
    set(CPPZMQ_TARGET_NAME cppzmq-static)
    set(XEUS_TARGET_NAME xeus-static)
    set(OPENSSL_USE_STATIC_LIBS ON CACHE BOOL "Linking statically with OpenSSL")
else()
    set(CPPZMQ_TARGET_NAME cppzmq)
    set(XEUS_TARGET_NAME xeus)
    set(OPENSSL_USE_STATIC_LIBS OFF CACHE BOOL "Not linking statically with OpenSSL")
endif()

# Print build configuration
# ==========================

message(STATUS "XEUS_ZMQ_BUILD_SHARED_LIBS:          ${XEUS_ZMQ_BUILD_SHARED_LIBS}")
message(STATUS "XEUS_ZMQ_BUILD_STATIC_LIBS:          ${XEUS_ZMQ_BUILD_STATIC_LIBS}")
message(STATUS "XEUS_ZMQ_STATIC_DEPENDENCIES:        ${XEUS_ZMQ_STATIC_DEPENDENCIES}")  
message(STATUS "XEUS_ZMQ_BUILD_TESTS:                ${XEUS_ZMQ_BUILD_TESTS}")  

# Dependencies
# ============

set(xeus_REQUIRED_VERSION 5.0.0)
set(nlohmann_json_REQUIRED_VERSION 3.11.3)
set(cppzmq_REQUIRED_VERSION 4.8.1)
set(zeromq_REQUIRED_VERSION 4.3.2)

if (NOT TARGET xeus AND NOT TARGET xeus-static)
    find_package(xeus ${xeus_REQUIRED_VERSION} REQUIRED)
    message(STATUS "Found xeus ${xeus_VERSION}")
endif ()

if (NOT TARGET nlohmann_json)
    find_package(nlohmann_json ${nlohmann_json_REQUIRED_VERSION} REQUIRED)
    message(STATUS "Found nlohmann_json ${nlohmann_json_VERSION}")
endif ()

if (NOT TARGET cppzmq AND NOT TARGET cppzmq-static)
    find_package(cppzmq ${cppzmq_REQUIRED_VERSION} REQUIRED)
    message(STATUS "Found cppzmq ${cppzmq_VERSION}")
endif ()

if (NOT TARGET libzmq AND NOT TARGET libzmq-static)
    if (WIN32)
        find_package(zeromq ${zeromq_REQUIRED_VERSION} REQUIRED)
    else ()
        find_package(zeromq ${zeromq_REQUIRED_VERSION} QUIET)

        if (NOT ZeroMQ_FOUND)
            message(STATUS "CMake libzmq package not found, trying again with pkg-config")
            find_package(PkgConfig)
            pkg_check_modules(ZeroMQ libzmq>=${zeromq_REQUIRED_VERSION} REQUIRED)
            set(ZeroMQ_VERSION ${PC_LIBZMQ_VERSION})
            find_library(ZeroMQ_LIBRARY NAMES libzmq.so libzmq.dylib libzmq.dll
                 PATHS ${PC_LIBZMQ_LIBDIR} ${PC_LIBZMQ_LIBRARY_DIRS})
            find_library(ZeroMQ_STATIC_LIBRARY NAMES libzmq-static.a libzmq.a libzmq.dll.a
                 PATHS ${PC_LIBZMQ_LIBDIR} ${PC_LIBZMQ_LIBRARY_DIRS})
            message(STATUS "STATIC_LIBRARY" {ZeroMQ_LIBRARY})
            message(STATUS "STATIC_STATIC_LIBRARY" {ZeroMQ_STATIC_LIBRARY})
         endif ()
    endif ()
endif ()

find_package(OpenSSL REQUIRED)

# Source files
# ============

set(XEUS_ZMQ_HEADERS
    ${XEUS_ZMQ_INCLUDE_DIR}/xeus-zmq/xclient_zmq.hpp
    ${XEUS_ZMQ_INCLUDE_DIR}/xeus-zmq/xcontrol_default_runner.hpp
    ${XEUS_ZMQ_INCLUDE_DIR}/xeus-zmq/xcontrol_runner.hpp
    ${XEUS_ZMQ_INCLUDE_DIR}/xeus-zmq/xdap_tcp_client.hpp
    ${XEUS_ZMQ_INCLUDE_DIR}/xeus-zmq/xdebugger_base.hpp
    ${XEUS_ZMQ_INCLUDE_DIR}/xeus-zmq/xeus-zmq.hpp
    ${XEUS_ZMQ_INCLUDE_DIR}/xeus-zmq/xmiddleware.hpp
    ${XEUS_ZMQ_INCLUDE_DIR}/xeus-zmq/xserver_zmq.hpp
    ${XEUS_ZMQ_INCLUDE_DIR}/xeus-zmq/xserver_zmq_split.hpp
    ${XEUS_ZMQ_INCLUDE_DIR}/xeus-zmq/xshell_default_runner.hpp
    ${XEUS_ZMQ_INCLUDE_DIR}/xeus-zmq/xshell_runner.hpp
    ${XEUS_ZMQ_INCLUDE_DIR}/xeus-zmq/xthread.hpp
    ${XEUS_ZMQ_INCLUDE_DIR}/xeus-zmq/xzmq_context.hpp
)

set(XEUS_ZMQ_SOURCES
    ${XEUS_ZMQ_SOURCE_DIR}/client/xclient_zmq.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/client/xclient_messenger.hpp
    ${XEUS_ZMQ_SOURCE_DIR}/client/xclient_messenger.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/client/xclient_zmq_impl.hpp
    ${XEUS_ZMQ_SOURCE_DIR}/client/xclient_zmq_impl.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/client/xdealer_channel.hpp
    ${XEUS_ZMQ_SOURCE_DIR}/client/xdealer_channel.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/client/xheartbeat_client.hpp
    ${XEUS_ZMQ_SOURCE_DIR}/client/xheartbeat_client.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/client/xiopub_client.hpp
    ${XEUS_ZMQ_SOURCE_DIR}/client/xiopub_client.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/common/xauthentication.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/common/xauthentication.hpp
    ${XEUS_ZMQ_SOURCE_DIR}/common/xmiddleware.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/common/xmiddleware_impl.hpp
    ${XEUS_ZMQ_SOURCE_DIR}/common/xzmq_context.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/common/xzmq_serializer.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/common/xzmq_serializer.hpp
    ${XEUS_ZMQ_SOURCE_DIR}/debugger/xdap_tcp_client.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/debugger/xdap_tcp_client_impl.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/debugger/xdap_tcp_client_impl.hpp
    ${XEUS_ZMQ_SOURCE_DIR}/debugger/xdebugger_base.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/debugger/xdebugger_middleware.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/debugger/xdebugger_middleware.hpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xcontrol_default_runner.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xcontrol_runner.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xcontrol.hpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xcontrol.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xheartbeat.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xheartbeat.hpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xpublisher.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xpublisher.hpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xserver_control_main.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xserver_control_main.hpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xserver_shell_main.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xserver_shell_main.hpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xserver_zmq_default.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xserver_zmq_default.hpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xserver_zmq_impl.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xserver_zmq_impl.hpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xserver_zmq_split.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xserver_zmq_split_impl.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xserver_zmq_split_impl.hpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xserver_zmq.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xshell.hpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xshell.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xshell_default_runner.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xshell_runner.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xtrivial_messenger.hpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xtrivial_messenger.cpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xzmq_messenger.hpp
    ${XEUS_ZMQ_SOURCE_DIR}/server/xzmq_messenger.cpp
)

# Targets and link
# ================

# Binaries that link with xeus-zmq must be able to find and load
# libzmq. However, since xeus-zmq privately links with libzmq,
# we need to tell cmake to append to rpath directories containing
# linked libraries.
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

include(CheckCXXCompilerFlag)

string(TOUPPER "${CMAKE_BUILD_TYPE}" U_CMAKE_BUILD_TYPE)

if (XEUS_ZMQ_STATIC_DEPENDENCIES AND NOT MSVC AND NOT APPLE AND NOT XEUS_ZMQ_BUILD_WITHOUT_LIBSODIUM)
	# Explicitly finds and links with libsodium.a
	# because it is not exported as a dependency by
	# the static build of libzmq. Remove this when
	# it is fixed upstream.
	set(sodium_USE_STATIC_LIBS ON)
	find_package(sodium REQUIRED)
endif ()

macro(xeus_zmq_create_target target_name linkage output_name)
    string(TOUPPER "${linkage}" linkage_upper)

    if (NOT ${linkage_upper} MATCHES "^(SHARED|STATIC)$")
        message(FATAL_ERROR "Invalid library linkage: ${linkage}")
    endif ()

    # Output
    # ======

    add_library(${target_name} ${linkage_upper} ${XEUS_ZMQ_SOURCES} ${XEUS_ZMQ_HEADERS})

    target_include_directories(
        ${target_name}
        PUBLIC $<BUILD_INTERFACE:${XEUS_ZMQ_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
    )

    target_link_libraries(
        ${target_name}
        PUBLIC nlohmann_json::nlohmann_json
        PUBLIC ${XEUS_TARGET_NAME}
        PRIVATE ${CPPZMQ_TARGET_NAME}
        PRIVATE OpenSSL::Crypto
    )

    if (NOT MSVC)
        if (APPLE)
            target_link_libraries(${target_name} PUBLIC "-framework CoreFoundation")
        else ()
            if (XEUS_ZMQ_STATIC_DEPENDENCIES)
                target_link_libraries(${target_name} PUBLIC ${sodium_LIBRARY_RELEASE})
            endif ()
        endif ()
    endif ()

    if (UNIX)
        # CMake does not compute the version number of so files as libtool
        # does on Linux. Strictly speaking, we should exclude FreeBSD and
        # Apple from this, but that would require having different version
        # numbers depending on the platform. We prefer to follow the
        # libtool pattern everywhere.
        math(EXPR XEUS_ZMQ_BINARY_COMPATIBLE "${XEUS_ZMQ_BINARY_CURRENT} - ${XEUS_ZMQ_BINARY_AGE}")
        set_target_properties(
            ${target_name}
            PROPERTIES
            PUBLIC_HEADER "${XEUS_ZMQ_HEADERS}"
            COMPILE_OPTIONS "-fvisibility=hidden"
            COMPILE_DEFINITIONS "XEUS_ZMQ_EXPORTS"
            PREFIX ""
            VERSION
            "${XEUS_ZMQ_BINARY_COMPATIBLE}.${XEUS_ZMQ_BINARY_REVISION}.${XEUS_ZMQ_BINARY_AGE}"
            SOVERSION ${XEUS_ZMQ_BINARY_COMPATIBLE}
            OUTPUT_NAME "lib${output_name}"
        )
    else()
        set_target_properties(
            ${target_name}
            PROPERTIES
            PUBLIC_HEADER "${XEUS_ZMQ_HEADERS}"
            COMPILE_DEFINITIONS "XEUS_ZMQ_EXPORTS"
            PREFIX ""
            VERSION ${XEUS_ZMQ_BINARY_VERSION}
            SOVERSION ${XEUS_ZMQ_BINARY_CURRENT}
            OUTPUT_NAME "lib${output_name}"
        )
    endif()

    # Compilation flags
    # =================

    target_compile_features(${target_name} PRIVATE cxx_std_17)

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
        CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR
        CMAKE_CXX_COMPILER_ID MATCHES "Intel")

        target_compile_options(${target_name} PUBLIC -Wunused-parameter -Wextra -Wreorder)

        message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
    endif()

    if (${linkage_upper} STREQUAL "STATIC")
        target_compile_definitions(${target_name} PUBLIC XEUS_ZMQ_STATIC_LIB)
    endif ()

    if (MSVC)
        target_compile_definitions(${target_name} PUBLIC -DNOMINMAX)
        target_compile_options(${target_name} PUBLIC /DGUID_WINDOWS /MP /bigobj)
        target_compile_options(${target_name} PUBLIC /wd4251 /wd4996)
    elseif (APPLE)
        target_compile_definitions(${target_name} PUBLIC -DGUID_CFUUID)
    else ()
        target_compile_definitions(${target_name} PUBLIC -DGUID_LIBUUID)
    endif ()

    if (XEUS_ZMQ_STATIC_DEPENDENCIES)
        if (CMAKE_DL_LIBS)
            target_link_libraries(${target_name} PRIVATE ${CMAKE_DL_LIBS})
        endif ()
        if (UNIX AND NOT APPLE)
            target_link_libraries(${target_name} PRIVATE util rt)
        endif ()
    endif ()
endmacro()

set(xeus_zmq_targets "")

if (XEUS_ZMQ_BUILD_SHARED_LIBS)
    xeus_zmq_create_target(xeus-zmq SHARED xeus-zmq)
    if(CMAKE_TARGET_SYSTEM MATCHES "Linux" AND OPENSSL_USE_STATIC_LIBS)
        # Do not reexport OpenSSL symbols from xeus, for libraries
        #   Prevents conflicts with other versions of OpenSSL
        #   loaded in the same process namespace, which can cause
        #   crashes if the versions are not compatible.
        set_target_properties(xeus-zmq PROPERTIES LINK_FLAGS "-Wl,--exclude-libs,libcrypto.a")
    endif()
    list(APPEND xeus_zmq_targets xeus-zmq)
endif ()

if (XEUS_ZMQ_BUILD_STATIC_LIBS)
    # On Windows, a static library should use a different output name
    # to avoid the conflict with the import library of a shared one.
    if (CMAKE_HOST_WIN32)
        xeus_zmq_create_target(xeus-zmq-static STATIC xeus-zmq-static)
    else ()
        xeus_zmq_create_target(xeus-zmq-static STATIC xeus-zmq)
    endif ()

    list(APPEND xeus_zmq_targets xeus-zmq-static)
endif ()

# Tests
# =====

# We need to control from outside whether we enable testing or not. We cannot
# rely on BUILD_TESTING since it doe snot exist until CTest is included.

include(CTest)

if(XEUS_ZMQ_BUILD_TESTS)
    set(BUILD_TESTING ON)
    message(STATUS "tests enabled")
else ()
    set(BUILD_TESTING OFF)
    message(STATUS "tests disabled")
endif()

if(BUILD_TESTING)
    add_subdirectory(test)
endif()

# Installation
# ============

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(XEUS_ZMQ_CMAKECONFIG_INSTALL_DIR
    "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" CACHE STRING "install path for xeus-zmqConfig.cmake")

install(TARGETS ${xeus_zmq_targets}
        EXPORT ${PROJECT_NAME}-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xeus-zmq)

# Makes the project importable from the build directory
export(EXPORT ${PROJECT_NAME}-targets
       FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

# Configure 'xeusConfig.cmake' for a build tree
set(XEUS_ZMQ_CONFIG_CODE "####### Expanded from \@XEUS_CONFIG_CODE\@ #######\n")
set(XEUS_ZMQ_CONFIG_CODE "${XEUS_ZMQ_CONFIG_CODE}set(CMAKE_MODULE_PATH \"${CMAKE_CURRENT_SOURCE_DIR}/cmake;\${CMAKE_MODULE_PATH}\")\n")
set(XEUS_ZMQ_CONFIG_CODE "${XEUS_ZMQ_CONFIG_CODE}##################################################")
configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${PROJECT_BINARY_DIR})

# Configure 'xeusConfig.cmake' for an install tree
set(XEUS_ZMQ_CONFIG_CODE "")
configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${XEUS_ZMQ_CMAKECONFIG_INSTALL_DIR})


write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
                                 VERSION ${XEUS_ZMQ_VERSION}
                                 COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
              DESTINATION ${XEUS_ZMQ_CMAKECONFIG_INSTALL_DIR})
install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}Targets.cmake
        DESTINATION ${XEUS_ZMQ_CMAKECONFIG_INSTALL_DIR})

cmake_minimum_required(VERSION 3.20)
project(frameometer VERSION 1.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if(NOT MSVC)
    foreach(_flags CMAKE_CXX_FLAGS CMAKE_SHARED_LINKER_FLAGS)
        set(${_flags} "${${_flags}} -m32")
    endforeach()
endif()

set(FRAMEOMETER_AMXX_ROOT "" CACHE PATH "Root of an amxmodx checkout")

if(NOT FRAMEOMETER_AMXX_ROOT)
    get_filename_component(_default_amxx "${CMAKE_CURRENT_SOURCE_DIR}/../amxmodx" ABSOLUTE)
    set(FRAMEOMETER_AMXX_ROOT "${_default_amxx}" CACHE PATH "" FORCE)
endif()

set(_hlsdk "${FRAMEOMETER_AMXX_ROOT}/build_deps/hlsdk")
set(_metamod "${FRAMEOMETER_AMXX_ROOT}/build_deps/metamod-hl1/metamod")

foreach(_required
    "${_hlsdk}/common"
    "${_hlsdk}/dlls"
    "${_hlsdk}/engine"
    "${_hlsdk}/public"
    "${_metamod}"
    "${FRAMEOMETER_AMXX_ROOT}/public"
    "${FRAMEOMETER_AMXX_ROOT}/public/sdk")
    if(NOT IS_DIRECTORY "${_required}")
        message(FATAL_ERROR "Missing SDK directory: ${_required}. Set FRAMEOMETER_AMXX_ROOT to an amxmodx checkout with its submodules present.")
    endif()
endforeach()

add_library(frameometer SHARED
    src/frameometer.cpp
    "${FRAMEOMETER_AMXX_ROOT}/public/sdk/amxxmodule.cpp")

if(WIN32)
    target_sources(frameometer PRIVATE src/frameometer.def)
endif()

target_include_directories(frameometer PRIVATE src)
target_include_directories(frameometer SYSTEM PRIVATE
    "${_hlsdk}/common"
    "${_hlsdk}/dlls"
    "${_hlsdk}/engine"
    "${_hlsdk}/public"
    "${_metamod}"
    "${FRAMEOMETER_AMXX_ROOT}/public"
    "${FRAMEOMETER_AMXX_ROOT}/public/sdk")

target_compile_definitions(frameometer PRIVATE
    HAVE_STDINT_H
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _CRT_SECURE_NO_WARNINGS
    FRAMEOMETER_VERSION="${PROJECT_VERSION}")

set(_sdk_source "${FRAMEOMETER_AMXX_ROOT}/public/sdk/amxxmodule.cpp")

if(MSVC)
    target_compile_options(frameometer PRIVATE /W4 /EHsc)
    target_link_options(frameometer PRIVATE /MACHINE:X86)
    set_source_files_properties("${_sdk_source}" PROPERTIES COMPILE_OPTIONS "/w")
else()
    target_compile_options(frameometer PRIVATE -Wall -Wextra -Wno-write-strings)
    set_source_files_properties("${_sdk_source}" PROPERTIES COMPILE_OPTIONS "-w")
    target_compile_options(frameometer PRIVATE -include cmath)
    target_link_options(frameometer PRIVATE
        -static-libstdc++ -static-libgcc -Wl,--exclude-libs,ALL)
    target_link_libraries(frameometer PRIVATE dl)
    set_target_properties(frameometer PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON)
endif()

if(WIN32 AND NOT MSVC)
    message(WARNING "Non-MSVC Windows toolchain: <cbase.h> will not parse. Use a Visual Studio generator.")
endif()

set_target_properties(frameometer PROPERTIES
    PREFIX ""
    OUTPUT_NAME $<IF:$<BOOL:${WIN32}>,frameometer_amxx,frameometer_amxx_i386>)

include(CheckTypeSize)
check_type_size("void*" FRAMEOMETER_POINTER_SIZE LANGUAGE CXX)
if(NOT FRAMEOMETER_POINTER_SIZE EQUAL 4)
    message(FATAL_ERROR
        "HLDS/Metamod is 32-bit but this configuration builds ${FRAMEOMETER_POINTER_SIZE}-byte "
        "pointers. Use a Win32/x86 generator, or install 32-bit development libraries "
        "(g++-multilib, libc6-dev-i386).")
endif()

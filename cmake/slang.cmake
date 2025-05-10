# From slang-rhi repository https://github.com/shader-slang/slang-rhi/tree/main
# Fetch slang binaries from the release repository

cmake_policy(SET CMP0135 NEW)

include(FetchContent)

# Use FetchContent to download and populate a package without calling add_subdirectory().
# This is used for downloading prebuilt external binaries.
macro(FetchPackage name)
    cmake_parse_arguments(FETCH "" "URL" "" ${ARGN})
    FetchContent_Declare(
            ${name}
            URL ${FETCH_URL}
            HTTP_HEADER "Authorization: token ${SLANG_GITHUB_TOKEN}"
    )
    FetchContent_GetProperties(${name})
    if (NOT ${name}_POPULATED)
        FetchContent_MakeAvailable(${name})
    endif ()
endmacro()

function(determine_target_architecture VAR)
    if (MSVC)
        if ("${MSVC_C_ARCHITECTURE_ID}" STREQUAL "X86")
            set(ARCH "i686")
        elseif ("${MSVC_C_ARCHITECTURE_ID}" STREQUAL "x64")
            set(ARCH "x86_64")
        elseif ("${MSVC_C_ARCHITECTURE_ID}" STREQUAL "ARM")
            set(ARCH "arm")
        elseif ("${MSVC_C_ARCHITECTURE_ID}" STREQUAL "ARM64")
            set(ARCH "arm64")
        else ()
            message(FATAL_ERROR "Failed to determine the MSVC target architecture: ${MSVC_C_ARCHITECTURE_ID}")
        endif ()
    else ()
        execute_process(
                COMMAND ${CMAKE_C_COMPILER} -dumpmachine
                RESULT_VARIABLE RESULT
                OUTPUT_VARIABLE ARCH
                ERROR_QUIET
        )
        if (RESULT)
            message(FATAL_ERROR "Failed to determine target architecture triplet: ${RESULT}")
        endif ()
        string(REGEX MATCH "([^-]+).*" ARCH_MATCH ${ARCH})
        if (NOT CMAKE_MATCH_1 OR NOT ARCH_MATCH)
            message(FATAL_ERROR "Failed to match the target architecture triplet: ${ARCH}")
        endif ()
        set(ARCH ${CMAKE_MATCH_1})
    endif ()
    message(STATUS "Target architecture - ${ARCH}")
    set(${VAR} ${ARCH} PARENT_SCOPE)
endfunction()

function(fetch_slang)
    set(VZT_FETCH_SLANG_VERSION "2025.6.3")
    set(SLANG_VERSION ${VZT_FETCH_SLANG_VERSION})
    set(SLANG_URL "https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}")

    determine_target_architecture(VZT_ARCHITECTURE)

    if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if (VZT_ARCHITECTURE MATCHES "x86_64")
            set(SLANG_URL "${SLANG_URL}-windows-x86_64.zip")
        elseif (VZT_ARCHITECTURE MATCHES "aarch64|arm64")
            set(SLANG_URL "${SLANG_URL}-windows-aarch64.zip")
        endif ()
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        if (VZT_ARCHITECTURE MATCHES "x86_64")
            set(SLANG_URL "${SLANG_URL}-linux-x86_64-glibc-2.17.tar.gz")
        elseif (VZT_ARCHITECTURE MATCHES "aarch64|arm64")
            set(SLANG_URL "${SLANG_URL}-linux-aarch64.tar.gz")
        endif ()
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        if (CMAKE_APPLE_SILICON_PROCESSOR MATCHES "x86_64")
            set(SLANG_URL "${SLANG_URL}-macos-x86_64.zip")
        else ()
            set(SLANG_URL "${SLANG_URL}-macos-aarch64.zip")
        endif ()
    endif ()

    message(STATUS "Fetching Slang ${SLANG_VERSION} ...")
    FetchPackage(slang URL ${SLANG_URL})
    set(VZT_SLANG_INCLUDE_DIR ${slang_SOURCE_DIR}/include)
    set(VZT_SLANG_BINARY_DIR ${slang_SOURCE_DIR})

    add_library(slang SHARED IMPORTED GLOBAL)

    if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set_target_properties(slang PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES ${VZT_SLANG_INCLUDE_DIR}
                IMPORTED_IMPLIB ${VZT_SLANG_BINARY_DIR}/lib/slang.lib
                IMPORTED_LOCATION ${VZT_SLANG_BINARY_DIR}/bin/slang.dll
        )
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set_target_properties(slang PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES ${VZT_SLANG_INCLUDE_DIR}
                IMPORTED_LOCATION ${VZT_SLANG_BINARY_DIR}/lib/libslang.so
        )
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set_target_properties(slang PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES ${VZT_SLANG_INCLUDE_DIR}
                IMPORTED_LOCATION ${VZT_SLANG_BINARY_DIR}/lib/libslang.dylib
        )
    endif ()

    # if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    #     copy_file(${VZT_SLANG_BINARY_DIR}/bin/slang.dll .)
    #     copy_file(${VZT_SLANG_BINARY_DIR}/bin/slang-glslang.dll .)
    #     copy_file(${VZT_SLANG_BINARY_DIR}/bin/slang-llvm.dll .)
    #     copy_file(${VZT_SLANG_BINARY_DIR}/bin/slang-rt.dll .)
    # endif ()
endfunction()

fetch_slang()

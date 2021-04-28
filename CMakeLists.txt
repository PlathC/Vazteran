cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(Vazteran)

set(VAZTERAN_VERSION 0.1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/Modules/")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(extern/glfw)
add_subdirectory(extern/glm)

# Based on https://github.com/SaschaWillems/Vulkan/blob/master/CMakeLists.txt
# Use FindVulkan module added with CMAKE 3.7
if (NOT CMAKE_VERSION VERSION_LESS 3.7.0)
    message(STATUS "Using module to find Vulkan")
    find_package(Vulkan)
endif()

IF(WIN32)
    IF (NOT Vulkan_FOUND)
        find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan PATHS ${CMAKE_SOURCE_DIR}/libs/vulkan)
        IF (Vulkan_LIBRARY)
            set(Vulkan_FOUND ON)
            MESSAGE("Using bundled Vulkan library version")
        ENDIF()
    ENDIF()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
ELSEIF(LINUX)
    IF (NOT Vulkan_FOUND)
        find_library(Vulkan_LIBRARY NAMES vulkan HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
        IF (Vulkan_LIBRARY)
            set(Vulkan_FOUND ON)
            MESSAGE("Using bundled Vulkan library version")
        ENDIF()
    ENDIF()
ELSEIF(APPLE)
    set(VAZTERAN_COMPILATION_FLAGS "${VAZTERAN_COMPILATION_FLAGS} -DVK_USE_PLATFORM_MACOS_MVK -DVK_EXAMPLE_XCODE_GENERATED")
    # Todo : android?
ENDIF(WIN32)

IF (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan library!")
ELSE()
    message(STATUS ${Vulkan_LIBRARY})
ENDIF()

# Based on https://github.com/lefticus/cppbestpractices/blob/master/02-Use_the_Tools_Available.md
if (CMAKE_COMPILER_IS_GNUCC)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(VAZTERAN_COMPILATION_FLAGS
                -Wall
                -Wextra
                -Wshadow
                -pedantic
                -Wnon-virtual-dtor
                -Wold-style-cast
                -Wcast-align
                -Wunused
                -Woverloaded-virtual
                -Wpedantic
                -Wconversion
                -Wsign-conversion
                -Wmisleading-indentation
                -Wduplicated-cond
                -Wduplicated-branches
                -Wlogical-op
                -Wnull-dereference
                -Wuseless-cast
                -Wdouble-promotion
                -Wformat=2)
    else()
        set(VAZTERAN_COMPILATION_FLAGS
                -DNDEBUG
                -Wall
                -Wextra
                -Wshadow
                -pedantic
                -Wnon-virtual-dtor
                -Wold-style-cast
                -Wcast-align
                -Wunused
                -Woverloaded-virtual
                -Wpedantic
                -Wconversion
                -Wsign-conversion
                -Wmisleading-indentation
                -Wduplicated-cond
                -Wduplicated-branches
                -Wlogical-op
                -Wnull-dereference
                -Wuseless-cast
                -Wdouble-promotion
                -Wformat=2)
    endif()
elseif (MSVC)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(VAZTERAN_COMPILATION_FLAGS
                /permissive
                /w14242
                /w14254
                /w14263
                /w14265
                /w14287
                /we4289
                /w14296
                /w14311
                /w14545
                /w14546
                /w14547
                /w14549
                /w14555
                /w14619
                /w14640
                /w14826
                /w14928)
        set(VAZTERAN_LINKER_FLAGS
                -DNOMINMAX -D_USE_MATH_DEFINES)
    else()
        set(VAZTERAN_COMPILATION_FLAGS
                /permissive
                /w14242
                /w14254
                /w14263
                /w14265
                /w14287
                /we4289
                /w14296
                /w14311
                /w14545
                /w14546
                /w14547
                /w14549
                /w14555
                /w14619
                /w14640
                /w14826
                /w14928)
    endif()
endif ()

set(HEADERS
        include/Vazteran/Vulkan/DeviceManager.hpp
        include/Vazteran/Vulkan/Instance.hpp
        include/Vazteran/Application.hpp
        include/Vazteran/Window.hpp
        )

set(SOURCES
        src/Vazteran/Vulkan/DeviceManager.cpp
        src/Vazteran/Vulkan/Instance.cpp
        src/Vazteran/Application.cpp
        src/Vazteran/Window.cpp
        src/Vazteran/main.cpp)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})

if (CMAKE_COMPILER_IS_GNUCC)
    target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
endif ()


target_compile_options(${PROJECT_NAME} PUBLIC ${VAZTERAN_COMPILATION_FLAGS})
target_compile_definitions(${PROJECT_NAME} PUBLIC ${VAZTERAN_LINKER_FLAGS})

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE
        glfw
        glm::glm
        Vulkan::Vulkan)

file(COPY shaders DESTINATION ${CMAKE_BINARY_DIR}/bin)

if(WIN32)
    add_custom_target(compileShader COMMAND cmd /c ${CMAKE_CURRENT_SOURCE_DIR}/shaders/compile.bat )
else()
    add_custom_target(compileShader COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/shaders/compile.sh )
endif()
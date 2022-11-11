cmake_minimum_required(VERSION 3.19 FATAL_ERROR)
project(Vazteran)

set(VAZTERAN_VERSION 0.1)

add_subdirectory(extern)

find_package(Vulkan REQUIRED FATAL_ERROR)

if( NOT TARGET Vulkan::Vulkan)
    message("Can't find fulkan")
endif()

set(VZT_HEADERS
    include/vzt/Core/Logger.hpp
    include/vzt/Core/Math.hpp
    include/vzt/Core/Meta.hpp
    include/vzt/Core/Type.hpp
    include/vzt/Core/Vulkan.hpp

    include/vzt/Command.hpp
    include/vzt/Device.hpp
    include/vzt/Instance.hpp
    include/vzt/Pipeline.hpp
    include/vzt/Program.hpp
    include/vzt/Surface.hpp
    include/vzt/Swapchain.hpp
    include/vzt/Window.hpp
)

set(VZT_SOURCES
    src/Core/Logger.cpp
    src/Core/Vulkan.cpp

    src/Command.cpp
    src/Device.cpp
    src/Instance.cpp
    src/Pipeline.cpp
    src/Program.cpp
    src/Surface.cpp
    src/Swapchain.cpp
    src/Window.cpp
)

add_library(Vazteran STATIC ${VZT_HEADERS} ${VZT_SOURCES} ${VZT_EXTERN_SOURCES})

get_cxx_flags(VZT_COMPILATION_FLAGS VZT_COMPILE_DEFINITIONS)
target_compile_options(    Vazteran PRIVATE ${VZT_COMPILATION_FLAGS})
target_compile_definitions(Vazteran PRIVATE ${VZT_COMPILE_DEFINITIONS})
target_include_directories(Vazteran PRIVATE ${VZT_EXTERN_INCLUDES} include/)
target_link_libraries(     Vazteran PRIVATE ${VZT_EXTERN_LIBRARIES})
target_compile_features(   Vazteran PRIVATE cxx_std_17)

target_link_libraries(     Vazteran PUBLIC  Vulkan::Vulkan)
target_include_directories(Vazteran PUBLIC ${VZT_EXTERN_PUBLIC_INCLUDES})
target_link_libraries(     Vazteran PUBLIC ${VZT_EXTERN_PUBLIC_LIBRARIES})

target_include_directories(Vazteran INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

cmake_minimum_required(VERSION 3.19 FATAL_ERROR)
project(Vazteran)

set(VAZTERAN_VERSION 0.1)

find_package(Vulkan REQUIRED)

include(../cmake/slang.cmake)

add_subdirectory(extern)

set(VZT_HEADERS

        include/vzt/camera.hpp
        include/vzt/input.hpp
        include/vzt/compiler.hpp
        include/vzt/render_graph.hpp
        include/vzt/Window.hpp

        include/vzt/core/assert.hpp
        include/vzt/core/logger.hpp
        include/vzt/core/file.hpp
        include/vzt/core/math.hpp
        include/vzt/core/meta.hpp
        include/vzt/core/type.hpp

        include/vzt/vulkan/acceleration_structure.hpp
        include/vzt/vulkan/buffer.hpp
        include/vzt/vulkan/command.hpp
        include/vzt/vulkan/descriptor.hpp
        include/vzt/vulkan/device.hpp
        include/vzt/vulkan/frame_buffer.hpp
        include/vzt/vulkan/image.hpp
        include/vzt/vulkan/instance.hpp
        include/vzt/vulkan/pipeline.hpp
        include/vzt/vulkan/program.hpp
        include/vzt/vulkan/query_pool.hpp
        include/vzt/vulkan/render_pass.hpp
        include/vzt/vulkan/setup.hpp
        include/vzt/vulkan/surface.hpp
        include/vzt/vulkan/swapchain.hpp

        include/vzt/Vulkan/pipeline/compute.hpp
        include/vzt/Vulkan/pipeline/graphics.hpp
        include/vzt/Vulkan/pipeline/raytracing.hpp
)

set(VZT_SOURCES

        src/camera.cpp
        src/compiler.cpp
        src/input.cpp
        src/render_graph.cpp
        src/window.cpp

        src/core/file.cpp
        src/core/logger.cpp

        src/vulkan/acceleration_structure.cpp
        src/vulkan/buffer.cpp
        src/vulkan/command.cpp
        src/vulkan/descriptor.cpp
        src/vulkan/device.cpp
        src/vulkan/frame_buffer.cpp
        src/vulkan/image.cpp
        src/vulkan/instance.cpp
        src/vulkan/program.cpp
        src/vulkan/query_pool.cpp
        src/vulkan/render_pass.cpp
        src/vulkan/setup.cpp
        src/vulkan/surface.cpp
        src/vulkan/swapchain.cpp
        src/vulkan/uniform.cpp

        src/vulkan/pipeline/compute.cpp
        src/vulkan/pipeline/graphics.cpp
        src/vulkan/pipeline/raytracing.cpp
)

add_library(Vazteran STATIC ${VZT_HEADERS} ${VZT_SOURCES} ${VZT_EXTERN_SOURCES})

get_cxx_flags(VZT_COMPILATION_FLAGS VZT_COMPILE_DEFINITIONS)
target_compile_options(Vazteran PRIVATE ${VZT_COMPILATION_FLAGS})
target_compile_definitions(Vazteran PRIVATE ${VZT_COMPILE_DEFINITIONS})
target_include_directories(Vazteran SYSTEM PRIVATE ${VZT_EXTERN_INCLUDES})
target_include_directories(Vazteran PRIVATE include/)
target_include_directories(Vazteran SYSTEM INTERFACE include/)
target_link_libraries(Vazteran PRIVATE ${VZT_EXTERN_LIBRARIES})
target_compile_features(Vazteran PRIVATE cxx_std_20)

target_link_libraries(Vazteran PUBLIC Vulkan::Vulkan slang)

target_include_directories(Vazteran SYSTEM PUBLIC ${VZT_EXTERN_PUBLIC_INCLUDES})
target_link_libraries(Vazteran PUBLIC ${VZT_EXTERN_PUBLIC_LIBRARIES})

target_include_directories(Vazteran INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_dependency_folder(VztShaders "${CMAKE_CURRENT_SOURCE_DIR}/shaders" "${CMAKE_BINARY_DIR}/bin/shaders")
add_dependencies(Vazteran VztShaders)

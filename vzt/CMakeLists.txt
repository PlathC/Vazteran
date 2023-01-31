cmake_minimum_required(VERSION 3.19 FATAL_ERROR)
project(Vazteran)

set(VAZTERAN_VERSION 0.1)

find_package(Vulkan REQUIRED FATAL_ERROR)

add_subdirectory(extern)

set(VZT_HEADERS
    include/vzt/Core/Logger.hpp
    include/vzt/Core/File.hpp
    include/vzt/Core/Math.hpp
    include/vzt/Core/Meta.hpp
    include/vzt/Core/Type.hpp

    include/vzt/Data/Camera.hpp

    include/vzt/Ui/Input.hpp
    
    include/vzt/Utils/Compiler.hpp
    include/vzt/Utils/MeshLoader.hpp
    include/vzt/Utils/RenderGraph.hpp

    include/vzt/Vulkan/Buffer.hpp
    include/vzt/Vulkan/Command.hpp
    include/vzt/Vulkan/Core.hpp
    include/vzt/Vulkan/Descriptor.hpp
    include/vzt/Vulkan/Device.hpp
    include/vzt/Vulkan/DeviceConfiguration.hpp
    include/vzt/Vulkan/FrameBuffer.hpp
    include/vzt/Vulkan/Image.hpp
    include/vzt/Vulkan/Instance.hpp
    include/vzt/Vulkan/Pipeline.hpp
    include/vzt/Vulkan/Program.hpp
    include/vzt/Vulkan/RenderPass.hpp
    include/vzt/Vulkan/Surface.hpp
    include/vzt/Vulkan/Swapchain.hpp
    include/vzt/Vulkan/Texture.hpp

    include/vzt/Vulkan/Raytracing/AccelerationStructure.hpp
    
    include/vzt/Vulkan/Pipeline/ComputePipeline.hpp
    include/vzt/Vulkan/Pipeline/GraphicPipeline.hpp

    include/vzt/Window.hpp
)

set(VZT_SOURCES
    src/Core/File.cpp
    src/Core/Logger.cpp
    src/Core/Vulkan.cpp

    src/Data/Camera.cpp

    src/Ui/Input.cpp

    src/Utils/Compiler.cpp
    src/Utils/MeshLoader.cpp
    src/Utils/RenderGraph.cpp

    src/Vulkan/Buffer.cpp
    src/Vulkan/Command.cpp
    src/Vulkan/Descriptor.cpp
    src/Vulkan/Device.cpp
    src/Vulkan/DeviceConfiguration.cpp
    src/Vulkan/FrameBuffer.cpp
    src/Vulkan/Image.cpp
    src/Vulkan/Instance.cpp
    src/Vulkan/Program.cpp
    src/Vulkan/RenderPass.cpp
    src/Vulkan/Surface.cpp
    src/Vulkan/Swapchain.cpp
    src/Vulkan/Texture.cpp

    src/Vulkan/Raytracing/AccelerationStructure.cpp

    src/Vulkan/Pipeline/ComputePipeline.cpp
    src/Vulkan/Pipeline/GraphicPipeline.cpp

    src/Window.cpp
)

add_library(Vazteran STATIC ${VZT_HEADERS} ${VZT_SOURCES} ${VZT_EXTERN_SOURCES})

get_cxx_flags(VZT_COMPILATION_FLAGS VZT_COMPILE_DEFINITIONS)
target_compile_options(    Vazteran PRIVATE ${VZT_COMPILATION_FLAGS})
target_compile_definitions(Vazteran PRIVATE ${VZT_COMPILE_DEFINITIONS})
target_include_directories(Vazteran PRIVATE ${VZT_EXTERN_INCLUDES} include/)
target_link_libraries(     Vazteran PRIVATE ${VZT_EXTERN_LIBRARIES})
target_compile_features(   Vazteran PRIVATE cxx_std_17)

target_include_directories(Vazteran PUBLIC ${VZT_EXTERN_PUBLIC_INCLUDES})
target_link_libraries(     Vazteran PUBLIC ${VZT_EXTERN_PUBLIC_LIBRARIES})

target_include_directories(Vazteran INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_dependency_folder(     Vazteran VztShaders  "${CMAKE_CURRENT_SOURCE_DIR}/shaders" "${CMAKE_BINARY_DIR}/bin/shaders")
add_dependency_folder(     Vazteran VztSamples  "${CMAKE_CURRENT_SOURCE_DIR}/samples" "${CMAKE_BINARY_DIR}/bin/samples")
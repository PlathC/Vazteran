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

# Based on https://github.com/SaschaWillems/Vulkan/blob/master/CMakeLists.txt
if (NOT CMAKE_VERSION VERSION_LESS 3.19.0)
    message(STATUS "Using module to find Vulkan")
    find_package(Vulkan)
endif()

IF(WIN32)
    set(VZT_COMPILE_DEFINITIONS "${VZT_COMPILE_DEFINITIONS} -DVK_USE_PLATFORM_WIN32_KHR")
ELSEIF(APPLE)
    set(VZT_COMPILE_DEFINITIONS "${VZT_COMPILE_DEFINITIONS} -DVK_USE_PLATFORM_MACOS_MVK -DVK_EXAMPLE_XCODE_GENERATED")
ENDIF(WIN32)

IF (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan library!")
ENDIF()

set(VZT_EXTERN_LIBRARIES
    ${VZT_EXTERN_LIBRARIES}
    Vulkan::Vulkan
)

add_subdirectory(extern)

# Based on https://github.com/lefticus/cppbestpractices/blob/master/02-Use_the_Tools_Available.md
if (CMAKE_COMPILER_IS_GNUCC)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(VZT_COMPILATION_FLAGS
                ${VZT_COMPILATION_FLAGS}
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
        set(VZT_COMPILATION_FLAGS
                ${VZT_COMPILATION_FLAGS}
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
        set(VZT_COMPILATION_FLAGS
                ${VZT_COMPILATION_FLAGS}
                /WX
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
                /w14928
                
                /wd4296)
        set(VZT_COMPILE_DEFINITIONS
                ${VZT_COMPILE_DEFINITIONS}
                -DNOMINMAX 
                -D_USE_MATH_DEFINES)
    else()
        set(VZT_COMPILATION_FLAGS
                ${VZT_COMPILATION_FLAGS}
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

set(VZT_HEADERS
        "include/Vazteran/Window.hpp"
        "include/Vazteran/WindowTypes.hpp"

        "include/Vazteran/Core/Logger.hpp"
        "include/Vazteran/Core/Macro.hpp"
        "include/Vazteran/Core/Type.hpp"

        "include/Vazteran/Data/Camera.hpp"
        "include/Vazteran/Data/Image.hpp"
        "include/Vazteran/Data/Material.hpp"
        "include/Vazteran/Data/Mesh.hpp"
        
        "include/Vazteran/FileSystem/File.hpp"
        "include/Vazteran/FileSystem/MeshReader.hpp"

        "include/Vazteran/Math/Aabb.hpp"
        "include/Vazteran/Math/Math.hpp"

        "include/Vazteran/Renderer/DescriptorBufferList.hpp"
        "include/Vazteran/Renderer/ShaderCompiler.hpp"
        "include/Vazteran/Renderer/ShaderLibrary.hpp"
        
        "include/Vazteran/System/Listener.hpp"
        "include/Vazteran/System/Scene.hpp"
        "include/Vazteran/System/Transform.hpp"

        "include/Vazteran/Ui/Component.hpp"
        "include/Vazteran/Ui/MainMenuBar.hpp"
        "include/Vazteran/Ui/UiManager.hpp"

        "include/Vazteran/Views/MeshView.hpp"

        "include/Vazteran/Backend/Vulkan/Attachment.hpp"
        "include/Vazteran/Backend/Vulkan/Buffer.hpp"
        "include/Vazteran/Backend/Vulkan/Buffer.inl"
        "include/Vazteran/Backend/Vulkan/CommandPool.hpp"
        "include/Vazteran/Backend/Vulkan/Descriptor.hpp"
        "include/Vazteran/Backend/Vulkan/Device.hpp"
        "include/Vazteran/Backend/Vulkan/FrameBuffer.hpp"
        
        "include/Vazteran/Backend/Vulkan/GraphicPipeline.hpp"
        "include/Vazteran/Backend/Vulkan/ImageTypes.hpp"
        "include/Vazteran/Backend/Vulkan/ImageUtils.hpp"
        "include/Vazteran/Backend/Vulkan/Instance.hpp"
        "include/Vazteran/Backend/Vulkan/Renderer.hpp"
        "include/Vazteran/Backend/Vulkan/RenderGraph.hpp"
        "include/Vazteran/Backend/Vulkan/RenderPass.hpp"
        "include/Vazteran/Backend/Vulkan/Shader.hpp"
        "include/Vazteran/Backend/Vulkan/SwapChain.hpp"
        "include/Vazteran/Backend/Vulkan/UiRenderer.hpp"
)

set(VZT_SOURCES
        "src/main.cpp"
        "src/Window.cpp"
        
        "src/Core/Logger.cpp"

        "src/Data/Camera.cpp"
        "src/Data/Image.cpp"
        "src/Data/Material.cpp"
        "src/Data/Mesh.cpp"
        
        "src/FileSystem/File.cpp"
        "src/FileSystem/MeshReader.cpp"

        "src/Math/Aabb.cpp"

        "src/Renderer/DescriptorBufferList.cpp"
        "src/Renderer/ShaderCompiler.cpp"
        "src/Renderer/ShaderLibrary.cpp"

        "src/System/Scene.cpp"
        "src/System/Transform.cpp"

        "src/Ui/Component.cpp"
        "src/Ui/MainMenuBar.cpp"
        "src/Ui/UiManager.cpp"

        "src/Views/MeshView.cpp"

        "src/Backend/Vulkan/Attachment.cpp"
        "src/Backend/Vulkan/Buffer.cpp"
        "src/Backend/Vulkan/CommandPool.cpp"
        "src/Backend/Vulkan/Descriptor.cpp"
        "src/Backend/Vulkan/Device.cpp"
        "src/Backend/Vulkan/Instance.cpp"
        "src/Backend/Vulkan/GraphicPipeline.cpp"
        
        "src/Backend/Vulkan/ImageUtils.cpp"
        "src/Backend/Vulkan/FrameBuffer.cpp"
        "src/Backend/Vulkan/RenderGraph.cpp"
        "src/Backend/Vulkan/RenderPass.cpp"
        "src/Backend/Vulkan/Shader.cpp"
        "src/Backend/Vulkan/SwapChain.cpp"
        "src/Backend/Vulkan/UiRenderer.cpp"
        "src/Backend/Vulkan/Renderer.cpp"
)

add_executable(${PROJECT_NAME} ${VZT_HEADERS} ${VZT_SOURCES} ${VZT_EXTERN_SOURCES})

if (CMAKE_COMPILER_IS_GNUCC)
    set(VZT_EXTERN_LIBRARIES ${VZT_EXTERN_LIBRARIES} stdc++fs)
endif ()

if(WIN32)
    set(${VZT_COMPILE_DEFINITIONS} -DNOMINMAX)
endif()

target_compile_options(${PROJECT_NAME} PUBLIC ${VZT_COMPILATION_FLAGS})
target_compile_definitions(${PROJECT_NAME} PUBLIC ${VZT_COMPILE_DEFINITIONS})
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include" ${VZT_EXTERN_INCLUDES})
target_link_libraries(${PROJECT_NAME} PRIVATE ${VZT_EXTERN_LIBRARIES})

file(GLOB_RECURSE VZT_SHADERS shaders/*.*)
add_custom_target(VZT_SHADERS COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/shaders/ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/)
set_property(TARGET VZT_SHADERS APPEND PROPERTY SOURCES ${VZT_SHADERS}) 
add_dependencies(${PROJECT_NAME} VZT_SHADERS)

file(COPY shaders DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(COPY samples DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

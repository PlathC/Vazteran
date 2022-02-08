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

add_subdirectory(extern/glfw)
add_subdirectory(extern/glm)
add_subdirectory(extern/imgui)

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
        "include/Vazteran/Application.hpp"
        "include/Vazteran/Window.hpp"
        "include/Vazteran/WindowTypes.hpp"

        "include/Vazteran/Core/Math.hpp"
        "include/Vazteran/Core/Math/Aabb.hpp"
        "include/Vazteran/Core/Utils.hpp"
        "include/Vazteran/Data/Camera.hpp"
        "include/Vazteran/Data/Image.hpp"
        "include/Vazteran/Data/Material.hpp"
        "include/Vazteran/Data/Mesh.hpp"
        "include/Vazteran/Data/Model.hpp"
        "include/Vazteran/Data/Scene.hpp"
        
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
        "include/Vazteran/Backend/Vulkan/GpuObjects.hpp"
        "include/Vazteran/Backend/Vulkan/GraphicPipeline.hpp"
        "include/Vazteran/Backend/Vulkan/ImageTypes.hpp"
        "include/Vazteran/Backend/Vulkan/ImageUtils.hpp"
        "include/Vazteran/Backend/Vulkan/Instance.hpp"
        "include/Vazteran/Backend/Vulkan/RenderPass.hpp"
        "include/Vazteran/Backend/Vulkan/Shader.hpp"
        "include/Vazteran/Backend/Vulkan/SwapChain.hpp"
        "include/Vazteran/Backend/Vulkan/VkUiRenderer.hpp"
        "include/Vazteran/Backend/Vulkan/VkRenderer.hpp"
)

set(SOURCES
        "src/main.cpp"
        "src/Application.cpp"
        "src/Window.cpp"
        "src/Core/Math/Aabb.cpp"
        "src/Core/Utils.cpp"
        "src/Data/Camera.cpp"
        "src/Data/Image.cpp"
        "src/Data/Material.cpp"
        "src/Data/Mesh.cpp"
        "src/Data/Model.cpp"
        "src/Data/Scene.cpp"

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
        "src/Backend/Vulkan/RenderPass.cpp"
        "src/Backend/Vulkan/Shader.cpp"
        "src/Backend/Vulkan/SwapChain.cpp"
        "src/Backend/Vulkan/VkUiRenderer.cpp"
        "src/Backend/Vulkan/VkRenderer.cpp"
)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})

if (CMAKE_COMPILER_IS_GNUCC)
    target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
endif ()

target_compile_options(${PROJECT_NAME} PUBLIC ${VAZTERAN_COMPILATION_FLAGS})
target_compile_definitions(${PROJECT_NAME} PUBLIC ${VAZTERAN_LINKER_FLAGS})

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE extern/stb)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE extern/tinyobj)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE extern/vma)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE extern/imgui)

target_link_libraries(${PROJECT_NAME} PRIVATE
        glfw
        glm::glm
        Vulkan::Vulkan
        imgui
)

# Based on https://www.reddit.com/r/vulkan/comments/kbaxlz/what_is_your_workflow_when_compiling_shader_files/gfg0s3s?utm_source=share&utm_medium=web2x&context=3
function(add_shader TARGET SHADER)
    find_program(GLSLC glslc)

    set(current-shader-path ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER})
    set(current-output-path ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SHADER}.spv)

    # Add a custom command to compile GLSL to SPIR-V.
    get_filename_component(current-output-dir ${current-output-path} DIRECTORY)
    file(MAKE_DIRECTORY ${current-output-dir})

    add_custom_command(
            OUTPUT ${current-output-path}
            COMMAND ${GLSLC} -o ${current-output-path} ${current-shader-path}
            DEPENDS ${current-shader-path}
            IMPLICIT_DEPENDS CXX ${current-shader-path}
            VERBATIM)

    # Make sure our build depends on this output.
    set_source_files_properties(${current-output-path} PROPERTIES GENERATED TRUE)
    target_sources(${TARGET} PRIVATE ${current-output-path})
endfunction(add_shader)


add_shader(${PROJECT_NAME} shaders/fs_triangle.vert)
add_shader(${PROJECT_NAME} shaders/blinn_phong.frag)
add_shader(${PROJECT_NAME} shaders/triangle.vert)
add_shader(${PROJECT_NAME} shaders/triangle.frag)
file(COPY samples DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

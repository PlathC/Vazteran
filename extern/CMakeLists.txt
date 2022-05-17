add_subdirectory(entt)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

set(SPIRV_HEADERS_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
set(SPIRV_HEADERS_SKIP_INSTALL ON CACHE BOOL "" FORCE)
add_subdirectory(SPIRV-Headers)

set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
set(SPIRV_SKIP_TESTS ON CACHE BOOL "" FORCE)
add_subdirectory(SPIRV-Tools)


set(BUILD_EXTERNAL OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
add_subdirectory(glslang)

add_subdirectory(glm)

add_subdirectory(fmt)

set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "" FORCE)
add_subdirectory(spdlog)

set(IMGUI_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imconfig.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_internal.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_vulkan.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_vulkan.h"
)

# Prepare data for parent scope
set(VZT_EXTERN_INCLUDES
    ${VZT_EXTERN_INCLUDES}
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui"
    "${CMAKE_CURRENT_SOURCE_DIR}/stb"
    "${CMAKE_CURRENT_SOURCE_DIR}/tinyobjloader"
    "${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/glslang"
)

set(VZT_EXTERN_LIBRARIES
    ${VZT_EXTERN_LIBRARIES}
    EnTT
    fmt
    glfw
    glm::glm
    glslang
    spdlog::spdlog
    SPIRV 
)

set(VZT_EXTERN_SOURCES
    ${VZT_EXTERN_SOURCES}
    ${IMGUI_SOURCES}
)

set(VZT_COMPILATION_FLAGS ${VZT_COMPILATION_FLAGS} PARENT_SCOPE)
set(VZT_EXTERN_INCLUDES ${VZT_EXTERN_INCLUDES} PARENT_SCOPE)
set(VZT_EXTERN_SOURCES ${VZT_EXTERN_SOURCES} PARENT_SCOPE)
set(VZT_EXTERN_LIBRARIES ${VZT_EXTERN_LIBRARIES} PARENT_SCOPE)

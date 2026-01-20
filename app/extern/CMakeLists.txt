# IMGUI
set(IMGUI_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imconfig.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_internal.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdl3.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdl3.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_vulkan.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_vulkan.h"
)

add_library(VztImGui STATIC ${IMGUI_SOURCES})
target_link_libraries(VztImGui PRIVATE SDL3::SDL3)
target_compile_options(VztImGui PRIVATE "")
target_compile_definitions(VztImGui PRIVATE IMGUI_IMPL_VULKAN_NO_PROTOTYPES)
target_include_directories(VztImGui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/imgui")

# TinyObjLoader
add_library(VztTinyObjLoader INTERFACE)
target_include_directories(VztTinyObjLoader INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/tinyobjloader")


set(VZT_APP_DEPENDENCIES

        VztImGui
        VztTinyObjLoader
)

set(VZT_APP_DEPENDENCIES ${VZT_APP_DEPENDENCIES} PARENT_SCOPE)

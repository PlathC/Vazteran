
find_package(Vulkan REQUIRED)

add_library(imgui STATIC 
    imgui.h
    imgui.cpp

    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_impl_vulkan.cpp
    imgui_impl_glfw.cpp
    imgui_widgets.cpp
    misc/cpp/imgui_stdlib.cpp
    # misc/fonts/binary_to_compressed_c.cpp

    # misc/freetype/imgui_freetype.cpp
)

target_compile_definitions(imgui INTERFACE IMGUI_DISABLE_INCLUDE_IMCONFIG_H)
target_link_libraries(imgui PRIVATE Vulkan::Vulkan glfw)

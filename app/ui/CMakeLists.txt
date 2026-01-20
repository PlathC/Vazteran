get_cxx_flags(VZT_COMPILATION_FLAGS VZT_COMPILE_DEFINITIONS)

add_executable(VztUi main.cpp ${IMGUI_SOURCES})
target_link_libraries(VztUi PRIVATE VztAppCommon)
target_compile_options(VztUi PRIVATE ${VZT_COMPILATION_FLAGS})
target_compile_definitions(VztUi PRIVATE ${VZT_COMPILE_DEFINITIONS} IMGUI_IMPL_VULKAN_USE_VOLK)
target_compile_features(VztUi PRIVATE cxx_std_20)

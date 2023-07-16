get_cxx_flags(VZT_COMPILATION_FLAGS VZT_COMPILE_DEFINITIONS)

add_executable(VztBase main.cpp)
target_link_libraries(VztBase PRIVATE Vazteran)
target_compile_options(VztBase PRIVATE ${VZT_COMPILATION_FLAGS})
target_compile_definitions(VztBase PRIVATE ${VZT_COMPILE_DEFINITIONS})
target_compile_features(VztBase PRIVATE cxx_std_17)
add_dependency_folder(VztBaseShaders "${CMAKE_CURRENT_SOURCE_DIR}/shaders" "${CMAKE_BINARY_DIR}/bin/shaders")
add_dependencies(VztBase VztBaseShaders VztSamples)

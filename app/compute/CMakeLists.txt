get_cxx_flags(VZT_COMPILATION_FLAGS VZT_COMPILE_DEFINITIONS)

add_executable(VztCompute main.cpp)
target_link_libraries(VztCompute PRIVATE VztAppCommon)
target_compile_features(VztCompute PRIVATE cxx_std_20)
target_compile_options(VztCompute PRIVATE ${VZT_COMPILATION_FLAGS})
target_compile_definitions(VztCompute PRIVATE ${VZT_COMPILE_DEFINITIONS})
add_dependency_folder(VztComputeShaders "${CMAKE_CURRENT_SOURCE_DIR}/shaders" "${CMAKE_BINARY_DIR}/bin/shaders")
add_dependencies(VztCompute VztComputeShaders)

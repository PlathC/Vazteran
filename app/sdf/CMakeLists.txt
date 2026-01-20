get_cxx_flags(VZT_COMPILATION_FLAGS VZT_COMPILE_DEFINITIONS)

add_executable(VztSdf main.cpp)
target_link_libraries(VztSdf PRIVATE VztAppCommon)
target_compile_features(VztSdf PRIVATE cxx_std_20)
target_compile_options(VztSdf PRIVATE ${VZT_COMPILATION_FLAGS})
target_compile_definitions(VztSdf PRIVATE ${VZT_COMPILE_DEFINITIONS})
add_dependency_folder(VztSdfShaders "${CMAKE_CURRENT_SOURCE_DIR}/shaders" "${CMAKE_BINARY_DIR}/bin/shaders")
add_dependencies(VztSdf VztSdfShaders VztSamples)

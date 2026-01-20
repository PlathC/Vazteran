get_cxx_flags(VZT_COMPILATION_FLAGS VZT_COMPILE_DEFINITIONS)

add_executable(VztDeferred main.cpp)
target_link_libraries(VztDeferred PRIVATE VztAppCommon)
target_compile_features(VztDeferred PRIVATE cxx_std_20)
target_compile_options(VztDeferred PRIVATE ${VZT_COMPILATION_FLAGS})
target_compile_definitions(VztDeferred PRIVATE ${VZT_COMPILE_DEFINITIONS})
add_dependency_folder(VztDeferredShaders "${CMAKE_CURRENT_SOURCE_DIR}/shaders" "${CMAKE_BINARY_DIR}/bin/shaders")
add_dependencies(VztDeferred VztDeferredShaders VztSamples)

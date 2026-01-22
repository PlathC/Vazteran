get_cxx_flags(VZT_COMPILATION_FLAGS VZT_COMPILE_DEFINITIONS)

add_executable(VztOffline main.cpp)
target_link_libraries(VztOffline PRIVATE VztAppCommon)
target_compile_features(VztOffline PRIVATE cxx_std_20)
target_compile_options(VztOffline PRIVATE ${VZT_COMPILATION_FLAGS})
target_compile_definitions(VztOffline PRIVATE ${VZT_COMPILE_DEFINITIONS})
add_dependency_folder(VztOfflineShaders "${CMAKE_CURRENT_SOURCE_DIR}/shaders" "${CMAKE_BINARY_DIR}/bin/shaders")
add_dependencies(VztOffline VztOfflineShaders VztSamples)

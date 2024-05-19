get_cxx_flags(VZT_COMPILATION_FLAGS VZT_COMPILE_DEFINITIONS)

add_executable(VztRaytracing main.cpp)
target_link_libraries(VztRaytracing PRIVATE Vazteran)
target_compile_features(VztRaytracing PRIVATE cxx_std_20)
target_compile_options(VztRaytracing PRIVATE ${VZT_COMPILATION_FLAGS})
target_compile_definitions(VztRaytracing PRIVATE ${VZT_COMPILE_DEFINITIONS})
add_dependency_folder(VztRaytracingShaders "${CMAKE_CURRENT_SOURCE_DIR}/shaders" "${CMAKE_BINARY_DIR}/bin/shaders")
add_dependencies(VztRaytracing VztRaytracingShaders VztSamples)


add_executable(         VztRaytracing main.cpp)
target_link_libraries(  VztRaytracing PRIVATE Vazteran)
target_compile_features(VztRaytracing PRIVATE cxx_std_17)
add_dependency_folder(  VztRaytracing VztRaytracingShaders "${CMAKE_CURRENT_SOURCE_DIR}/shaders" "${CMAKE_BINARY_DIR}/bin/shaders")

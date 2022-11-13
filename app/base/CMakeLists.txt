
add_executable(VztBase main.cpp)
target_link_libraries(  VztBase PRIVATE Vazteran)
target_compile_features(VztBase PRIVATE cxx_std_17)
add_dependency_folder(  VztBase VztBaseShaders "${CMAKE_CURRENT_SOURCE_DIR}/shaders" "${CMAKE_BINARY_DIR}/bin/shaders")
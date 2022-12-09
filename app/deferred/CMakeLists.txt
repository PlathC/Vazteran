
add_executable(         VztDeferred main.cpp)
target_link_libraries(  VztDeferred PRIVATE Vazteran)
target_compile_features(VztDeferred PRIVATE cxx_std_17)
add_dependency_folder(  VztDeferred VztDeferredShaders "${CMAKE_CURRENT_SOURCE_DIR}/shaders" "${CMAKE_BINARY_DIR}/bin/shaders")

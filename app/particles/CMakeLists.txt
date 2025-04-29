get_cxx_flags(VZT_COMPILATION_FLAGS VZT_COMPILE_DEFINITIONS)

add_executable(VztParticles main.cpp)
target_link_libraries(VztParticles PRIVATE Vazteran)
target_compile_features(VztParticles PRIVATE cxx_std_20)
target_compile_options(VztParticles PRIVATE ${VZT_COMPILATION_FLAGS})
target_compile_definitions(VztParticles PRIVATE ${VZT_COMPILE_DEFINITIONS})
add_dependency_folder(VztParticlesShaders "${CMAKE_CURRENT_SOURCE_DIR}/shaders" "${CMAKE_BINARY_DIR}/bin/shaders")
add_dependencies(VztParticles VztParticlesShaders VztSamples)

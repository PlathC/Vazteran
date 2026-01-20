get_cxx_flags(VZT_COMPILATION_FLAGS VZT_COMPILE_DEFINITIONS)

add_executable(VztBlank main.cpp)
target_link_libraries(VztBlank PRIVATE VztAppCommon)
target_compile_options(VztBlank PRIVATE ${VZT_COMPILATION_FLAGS})
target_compile_definitions(VztBlank PRIVATE ${VZT_COMPILE_DEFINITIONS})
target_compile_features(VztBlank PRIVATE cxx_std_20)

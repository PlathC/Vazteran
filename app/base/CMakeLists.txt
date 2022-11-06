
add_executable(VztBase main.cpp)
target_link_libraries(  VztBase PRIVATE Vazteran)
target_compile_features(VztBase PRIVATE cxx_std_17)

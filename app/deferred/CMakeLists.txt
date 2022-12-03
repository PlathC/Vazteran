
add_executable(         VztDeferred main.cpp)
target_link_libraries(  VztDeferred PRIVATE Vazteran)
target_compile_features(VztDeferred PRIVATE cxx_std_17)

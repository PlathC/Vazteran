get_cxx_flags(VZT_COMPILATION_FLAGS VZT_COMPILE_DEFINITIONS)

add_library(VztAppCommon STATIC loader.hpp loader.cpp)
target_link_libraries(VztAppCommon PUBLIC Vazteran ${VZT_APP_DEPENDENCIES})
target_include_directories(VztAppCommon PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_compile_options(VztAppCommon PRIVATE "")
target_compile_definitions(VztAppCommon PRIVATE "")
target_compile_features(VztAppCommon PRIVATE cxx_std_20)


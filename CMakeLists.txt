cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(Vazteran)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(cmake/get_cxx_flags.cmake)

add_subdirectory(vzt)
add_subdirectory(app)

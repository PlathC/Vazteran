add_dependency_folder(VztSamples "${CMAKE_CURRENT_SOURCE_DIR}/samples" "${CMAKE_BINARY_DIR}/bin/samples")

add_subdirectory(extern)
add_subdirectory(common)

add_subdirectory(blank)
add_subdirectory(base)
add_subdirectory(deferred)
add_subdirectory(offline)
add_subdirectory(particles)
# add_subdirectory(raytracing)
add_subdirectory(sdf)
add_subdirectory(ui)

if (NOT TARGET fmt)
    add_subdirectory(fmt)
endif ()

if (NOT TARGET glm::glm)
    add_subdirectory(glm)
    target_compile_definitions(glm INTERFACE GLM_ENABLE_EXPERIMENTAL)
endif ()

if (NOT TARGET SDL3::SDL3)
    set(SDL3_DISABLE_UNINSTALL ON CACHE BOOL "" FORCE)
    set(SDL_TEST OFF CACHE BOOL "" FORCE)
    set(SDL_TESTS OFF CACHE BOOL "" FORCE)
    add_subdirectory(SDL)
endif ()

if (NOT TARGET volk)
    if (WIN32)
        set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
    endif ()
    add_subdirectory(volk)
endif ()

if (NOT TARGET tinyexr)
    # set(TINYEXR_BUILD_SAMPLE OFF)
    # add_subdirectory(tinyexr)
endif ()

# Prepare data for parent scope
set(VZT_EXTERN_PUBLIC_INCLUDES
        ${VZT_EXTERN_INCLUDES}

        "${CMAKE_CURRENT_SOURCE_DIR}/glslang"
        "${CMAKE_CURRENT_SOURCE_DIR}/tinyobjloader"
        "${CMAKE_CURRENT_SOURCE_DIR}/volk"
        "${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/tinyexr"
        "${CMAKE_CURRENT_SOURCE_DIR}/tinyexr/deps/miniz"
        ${Vulkan_INCLUDE_DIRS}
)

set(VZT_EXTERN_PUBLIC_LIBRARIES
        ${VZT_EXTERN_PUBLIC_LIBRARIES}

        fmt
        glm::glm
        SDL3::SDL3
        volk
        Vulkan::Vulkan
)

set(VZT_EXTERN_LIBRARIES
        ${VZT_EXTERN_LIBRARIES}

        Vulkan::glslang
        Vulkan::SPIRV-Tools
)

set(VZT_EXTERN_SOURCES
        ${VZT_EXTERN_SOURCES}
        "${CMAKE_CURRENT_SOURCE_DIR}/tinyexr/deps/miniz/miniz.c"
)

set(VZT_COMPILATION_FLAGS ${VZT_COMPILATION_FLAGS} PARENT_SCOPE)
set(VZT_EXTERN_PUBLIC_INCLUDES ${VZT_EXTERN_PUBLIC_INCLUDES} PARENT_SCOPE)
set(VZT_EXTERN_PUBLIC_LIBRARIES ${VZT_EXTERN_PUBLIC_LIBRARIES} PARENT_SCOPE)
set(VZT_EXTERN_SOURCES ${VZT_EXTERN_SOURCES} PARENT_SCOPE)
set(VZT_EXTERN_LIBRARIES ${VZT_EXTERN_LIBRARIES} PARENT_SCOPE)

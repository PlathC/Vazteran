function(message)
    if (NOT MESSAGE_QUIET)
        _message(${ARGN})
    endif ()
endfunction()

function(vzt_add_subdirectory)
    set(MESSAGE_QUIET ON)
    add_subdirectory(${ARGN})
    unset(MESSAGE_QUIET)
endfunction()

if (NOT TARGET fmt)
    message(STATUS "Fetching fmt ...")
    vzt_add_subdirectory(fmt)
endif ()

if (NOT TARGET glm::glm)
    message(STATUS "Fetching glm ...")
    vzt_add_subdirectory(glm)
    target_compile_definitions(glm INTERFACE GLM_ENABLE_EXPERIMENTAL)
endif ()

if (NOT TARGET SDL3::SDL3)
    message(STATUS "Fetching SDL3 ...")
    set(SDL3_DISABLE_UNINSTALL ON CACHE BOOL "" FORCE)
    set(SDL_TEST OFF CACHE BOOL "" FORCE)
    set(SDL_TESTS OFF CACHE BOOL "" FORCE)

    vzt_add_subdirectory(SDL)
endif ()

if (NOT TARGET volk)
    message(STATUS "Fetching volk ...")
    if (WIN32)
        set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
    endif ()
    vzt_add_subdirectory(volk)
endif ()

# Prepare data for parent scope
set(VZT_EXTERN_PUBLIC_INCLUDES
        ${VZT_EXTERN_INCLUDES}

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

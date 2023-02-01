if(NOT TARGET fmt)
    add_subdirectory(fmt)
endif() 

if(NOT TARGET glm::glm)
    add_subdirectory(glm)
endif()

if(NOT TARGET SDL2::SDL2)
    set(SDL2_DISABLE_UNINSTALL      ON  CACHE BOOL "" FORCE)
    set(SDL_TEST                    OFF CACHE BOOL "" FORCE)
    set(SDL_TESTS                   OFF CACHE BOOL "" FORCE)
    add_subdirectory(SDL)
endif() 

if(NOT TARGET SPIRV)
    set(SPIRV_HEADERS_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
    set(SPIRV_HEADERS_SKIP_INSTALL ON CACHE BOOL "" FORCE)
    add_subdirectory(SPIRV-Headers)

    set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
    set(SPIRV_SKIP_TESTS ON CACHE BOOL "" FORCE)
    add_subdirectory(SPIRV-Tools)
endif() 

if(NOT TARGET glslang)
    set(BUILD_EXTERNAL OFF CACHE BOOL "" FORCE)
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
    add_subdirectory(glslang)
endif() 

if(NOT TARGET volk)
    if (WIN32)
       set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
    endif()
    add_subdirectory(volk)
endif()

# Prepare data for parent scope
set(VZT_EXTERN_PUBLIC_INCLUDES
    ${VZT_EXTERN_INCLUDES}

    "${CMAKE_CURRENT_SOURCE_DIR}/glslang"
    "${CMAKE_CURRENT_SOURCE_DIR}/tinyobjloader"
    "${CMAKE_CURRENT_SOURCE_DIR}/volk"
    "${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator/include"
    ${Vulkan_INCLUDE_DIRS}
)

set(VZT_EXTERN_PUBLIC_LIBRARIES
    ${VZT_EXTERN_PUBLIC_LIBRARIES}

    fmt
    glm::glm
    volk
    Vulkan::Vulkan
)

set(VZT_EXTERN_LIBRARIES
    ${VZT_EXTERN_LIBRARIES}

    glslang
    SDL2::SDL2 SDL2::SDL2main
    SPIRV 
)

set(VZT_EXTERN_SOURCES
    ${VZT_EXTERN_SOURCES}
)

set(VZT_COMPILATION_FLAGS ${VZT_COMPILATION_FLAGS} PARENT_SCOPE)
set(VZT_EXTERN_PUBLIC_INCLUDES ${VZT_EXTERN_PUBLIC_INCLUDES} PARENT_SCOPE)
set(VZT_EXTERN_PUBLIC_LIBRARIES ${VZT_EXTERN_PUBLIC_LIBRARIES} PARENT_SCOPE)
set(VZT_EXTERN_SOURCES ${VZT_EXTERN_SOURCES} PARENT_SCOPE)
set(VZT_EXTERN_LIBRARIES ${VZT_EXTERN_LIBRARIES} PARENT_SCOPE)

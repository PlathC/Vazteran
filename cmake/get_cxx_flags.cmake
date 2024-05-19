function(get_cxx_flags compile_flag_var compile_definitions_var)
    # Reference: https://github.com/lefticus/cppbestpractices/blob/master/02-Use_the_Tools_Available.md
    if (MSVC)
        set(${compile_flag_var}
                ${${compile_flag_var}}
                /permissive
                /w14242
                /w14254
                /w14263
                /w14265
                /w14287
                /we4289
                /w14296
                /w14311
                /w14545
                /w14546
                /w14547
                /w14549
                /w14555
                /w14619
                /w14640
                /w14826
                /w14928
                /fp:fast
                )

        set(${compile_definitions_var} ${${compile_definitions_var}} NOMINMAX USE_MATH_DEFINES PARENT_SCOPE)
    else ()
        set(${compile_flag_var}
                ${${compile_flag_var}}
                -Wall
                -Wfatal-errors
                -Wextra
                -Wshadow
                -pedantic
                -Wnon-virtual-dtor
                -Wold-style-cast
                -Wcast-align
                -Wunused
                -Woverloaded-virtual
                -Wpedantic
                -Wconversion
                -Wsign-conversion
                -Wmisleading-indentation
                -Wnull-dereference
                -Wdouble-promotion
                -Wformat=2
                -Wno-missing-field-initializers
                -Wno-missing-braces
                )
    endif ()

    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        set(${compile_flag_var}
                ${${compile_flag_var}}
                -Wno-nullability-completeness
                )
    endif ()

    set(${compile_flag_var} ${${compile_flag_var}} PARENT_SCOPE)
endfunction()

include(osxie_lib)

macro(add_darling_library name)
    add_osxie_library(${name} ${ARGN})
endmacro()

macro(add_darling_object_library name)
    add_osxie_object_library(${name} ${ARGN})
endmacro()

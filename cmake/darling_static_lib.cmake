include(osxie_static_lib)

macro(add_darling_static_library name)
    add_osxie_static_library(${name} ${ARGN})
endmacro()

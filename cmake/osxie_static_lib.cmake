macro(add_osxie_static_library name)
    add_osxie_library(${name} STATIC ${ARGN})
endmacro()

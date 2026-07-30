macro(add_osxie_library name)
    add_library(${name} ${ARGN})
endmacro()

macro(add_osxie_object_library name)
    add_library(${name} OBJECT ${ARGN})
endmacro()

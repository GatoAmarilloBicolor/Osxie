include(osxie_framework)

macro(add_darling_framework name)
    add_framework(${name} ${ARGN})
endmacro()

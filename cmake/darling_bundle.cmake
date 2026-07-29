include(osxie_bundle)

macro(add_darling_bundle name loader)
    add_osxie_bundle(${name} ${loader} ${ARGN})
endmacro()

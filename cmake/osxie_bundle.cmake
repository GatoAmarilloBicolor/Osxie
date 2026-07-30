macro(add_osxie_bundle name loader)
    add_osxie_executable(${name} ${loader} ${ARGN})
endmacro()

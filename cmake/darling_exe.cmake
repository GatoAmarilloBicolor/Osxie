include(osxie_exe)

macro(add_darling_executable exe)
    add_osxie_executable(${exe} ${ARGN})
endmacro()

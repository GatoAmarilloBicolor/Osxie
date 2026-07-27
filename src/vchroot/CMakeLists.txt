project(vchroot)
include(osxie_exe)

add_definitions(-nostdinc)

add_osxie_executable(vchroot vchroot.c)
target_link_libraries(vchroot system)
install(TARGETS vchroot DESTINATION libexec/osxie/usr/libexec/osxie)


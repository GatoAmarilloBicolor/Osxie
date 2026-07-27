project(pboard)

add_definitions(-nostdinc)

add_osxie_executable(pbcopy pbcopy.m)
target_link_libraries(pbcopy AppKit Foundation objc)

add_osxie_executable(pbpaste pbpaste.m)
target_link_libraries(pbpaste AppKit Foundation objc)

install(TARGETS pbcopy pbpaste DESTINATION libexec/osxie/usr/bin)

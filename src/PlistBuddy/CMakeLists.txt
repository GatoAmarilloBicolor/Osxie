project(PlistBuddy)

add_osxie_executable(PlistBuddy
	PlistBuddy.c
)

target_link_libraries(PlistBuddy CoreFoundation)

install(TARGETS PlistBuddy DESTINATION libexec/osxie/usr/libexec)

install(FILES PlistBuddy.8 DESTINATION libexec/osxie/usr/share/man/man8)

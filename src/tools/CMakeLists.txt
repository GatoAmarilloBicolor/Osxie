project(tools)

if (COMPONENT_cli)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "cli")

	add_osxie_executable(sw_vers sw_vers.c)
	add_osxie_executable(sudo sudo.c)
	add_osxie_executable(codesign codesign.c)
	add_osxie_executable(dsmemberutil dsmemberutil.c)
	add_osxie_executable(softwareupdate softwareupdate.c)
	add_osxie_executable(spctl spctl.c)

	install(TARGETS sw_vers sudo codesign dsmemberutil DESTINATION libexec/osxie/usr/bin)
	install(TARGETS softwareupdate spctl DESTINATION libexec/osxie/usr/sbin)

	target_link_libraries(sw_vers CoreFoundation)
endif()

if (COMPONENT_gui)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "gui")

	add_osxie_executable(open open.m)
	target_link_libraries(open CoreServices Foundation AppKit)
	install(TARGETS open DESTINATION libexec/osxie/usr/bin)
endif()

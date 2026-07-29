project(libsimple)

set(libsimple_sources
	src/lock.c
)

if (NOT DEFINED libsimple_linux_added)
	set(libsimple_linux_added TRUE PARENT_SCOPE)

	add_library(libsimple_osxieserver STATIC "${libsimple_sources}")

	target_include_directories(libsimple_osxieserver PUBLIC
		include
	)

	target_compile_definitions(libsimple_osxieserver PRIVATE
		LIBSIMPLE_LINUX=1
	)
else()
	add_osxie_static_library(libsimple_osxie
		FAT
		SOURCES "${libsimple_sources}"
	)

	target_include_directories(libsimple_osxie PUBLIC
		include
	)

	target_compile_definitions(libsimple_osxie PRIVATE
		LIBSIMPLE_OSXIE=1
	)
endif()

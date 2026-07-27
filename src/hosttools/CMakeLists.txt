project(hosttools)

option(OSXIE_COREDUMP_SANITIZE "Enable/disable ASAN and UBSAN in osxie-coredump" OFF)

add_executable(osxie-coredump
	src/coredump/main.cpp
)

target_compile_options(osxie-coredump PRIVATE
	-std=c++17
)

target_include_directories(osxie-coredump PRIVATE
	include

	# TEMPORARY
	# this set of includes should probably be moved out because we probably want to be able to use Mach-O headers
	# in non-Darling compilation environments more often. by non-Darling, i mean when compiling directly for Linux
	# as we do in mldr and now here.
	../startup/mldr/include
)

install(TARGETS osxie-coredump DESTINATION bin)

if (OSXIE_COREDUMP_SANITIZE)
	target_compile_options(osxie-coredump PRIVATE
		-fsanitize=address,undefined
		-fsanitize-recover=all
		-fno-omit-frame-pointerer
		-g
		-Og
	)
	target_link_options(osxie-coredump PRIVATE
		-fsanitize=address,undefined
		-fsanitize-recover=all
	)
endif()

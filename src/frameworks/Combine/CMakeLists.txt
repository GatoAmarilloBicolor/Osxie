project(Combine)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
set(FRAMEWORK_VERSION "A")

remove_sdk_framework(Combine)

generate_sdk_framework(Combine
	VERSION ${FRAMEWORK_VERSION}
	HEADER "include/Combine"
)

add_framework(Combine
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/stubs.c
)

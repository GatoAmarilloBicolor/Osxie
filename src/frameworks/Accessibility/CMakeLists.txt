project(Accessibility_FW)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
set(FRAMEWORK_VERSION "A")

remove_sdk_framework(Accessibility)

generate_sdk_framework(Accessibility
	VERSION ${FRAMEWORK_VERSION}
	HEADER "include/Accessibility"
)

add_framework(Accessibility
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}
	TARGET_NAME Accessibility_FW

	SOURCES
		src/stubs.c
)

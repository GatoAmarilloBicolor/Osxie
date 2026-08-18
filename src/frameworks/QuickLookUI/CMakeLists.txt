project(QuickLookUI)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
set(FRAMEWORK_VERSION "A")

remove_sdk_framework(QuickLookUI)

generate_sdk_framework(QuickLookUI
	VERSION ${FRAMEWORK_VERSION}
	HEADER "include/QuickLookUI"
)

add_framework(QuickLookUI
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/stubs.m

	DEPENDENCIES
		system
		objc
		Foundation
)

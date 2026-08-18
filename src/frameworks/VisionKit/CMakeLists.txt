project(VisionKit)
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
set(FRAMEWORK_VERSION "A")
remove_sdk_framework(VisionKit)
generate_sdk_framework(VisionKit VERSION ${FRAMEWORK_VERSION} HEADER "include/VisionKit")
add_framework(VisionKit FAT CURRENT_VERSION VERSION ${FRAMEWORK_VERSION} TARGET_NAME VisionKit_FW
    SOURCES src/stubs.m
    DEPENDENCIES system objc Foundation)

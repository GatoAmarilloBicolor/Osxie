project(CoreDisplay)
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
set(FRAMEWORK_VERSION "A")
remove_sdk_framework(CoreDisplay)
generate_sdk_framework(CoreDisplay VERSION ${FRAMEWORK_VERSION} HEADER "include/CoreDisplay")
add_framework(CoreDisplay FAT CURRENT_VERSION VERSION ${FRAMEWORK_VERSION} TARGET_NAME CoreDisplay_FW
    SOURCES src/stubs.m
    DEPENDENCIES system objc Foundation)

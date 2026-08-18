project(CoreHaptics)
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
set(FRAMEWORK_VERSION "A")
remove_sdk_framework(CoreHaptics)
generate_sdk_framework(CoreHaptics VERSION ${FRAMEWORK_VERSION} HEADER "include/CoreHaptics")
add_framework(CoreHaptics FAT CURRENT_VERSION VERSION ${FRAMEWORK_VERSION} TARGET_NAME CoreHaptics_FW
    SOURCES src/stubs.m
    DEPENDENCIES system objc Foundation)

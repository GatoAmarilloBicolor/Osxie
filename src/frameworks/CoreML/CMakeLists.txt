project(CoreML)
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
set(FRAMEWORK_VERSION "A")
remove_sdk_framework(CoreML)
generate_sdk_framework(CoreML VERSION ${FRAMEWORK_VERSION} HEADER "include/CoreML")
add_framework(CoreML FAT CURRENT_VERSION VERSION ${FRAMEWORK_VERSION} TARGET_NAME CoreML_FW
    SOURCES src/stubs.m
    DEPENDENCIES system objc Foundation)

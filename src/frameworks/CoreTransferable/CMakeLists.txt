project(CoreTransferable)
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
set(FRAMEWORK_VERSION "A")
remove_sdk_framework(CoreTransferable)
generate_sdk_framework(CoreTransferable VERSION ${FRAMEWORK_VERSION} HEADER "include/CoreTransferable")
add_framework(CoreTransferable FAT CURRENT_VERSION VERSION ${FRAMEWORK_VERSION} TARGET_NAME CoreTransferable_FW
    SOURCES src/stubs.m
    DEPENDENCIES system objc Foundation)

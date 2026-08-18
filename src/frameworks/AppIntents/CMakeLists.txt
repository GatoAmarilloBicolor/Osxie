project(AppIntents)
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
set(FRAMEWORK_VERSION "A")
remove_sdk_framework(AppIntents)
generate_sdk_framework(AppIntents VERSION ${FRAMEWORK_VERSION} HEADER "include/AppIntents")
add_framework(AppIntents FAT CURRENT_VERSION VERSION ${FRAMEWORK_VERSION} TARGET_NAME AppIntents_FW
    SOURCES src/stubs.m
    DEPENDENCIES system objc Foundation AppKit)

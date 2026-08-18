project(DeveloperToolsSupport)
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
set(FRAMEWORK_VERSION "A")
remove_sdk_framework(DeveloperToolsSupport)
generate_sdk_framework(DeveloperToolsSupport VERSION ${FRAMEWORK_VERSION} HEADER "include/DeveloperToolsSupport")
add_framework(DeveloperToolsSupport FAT CURRENT_VERSION VERSION ${FRAMEWORK_VERSION} TARGET_NAME DeveloperToolsSupport_FW
    SOURCES src/stubs.m
    DEPENDENCIES system objc Foundation)

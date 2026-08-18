project(Symbols)
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
set(FRAMEWORK_VERSION "A")
remove_sdk_framework(Symbols)
generate_sdk_framework(Symbols VERSION ${FRAMEWORK_VERSION} HEADER "include/Symbols")
add_framework(Symbols FAT CURRENT_VERSION VERSION ${FRAMEWORK_VERSION} TARGET_NAME Symbols_FW
    SOURCES src/stubs.m
    DEPENDENCIES system objc Foundation)

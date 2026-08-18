project(CryptoKit)
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
set(FRAMEWORK_VERSION "A")
remove_sdk_framework(CryptoKit)
generate_sdk_framework(CryptoKit VERSION ${FRAMEWORK_VERSION} HEADER "include/CryptoKit")
add_framework(CryptoKit FAT CURRENT_VERSION VERSION ${FRAMEWORK_VERSION} TARGET_NAME CryptoKit_FW
    SOURCES src/stubs.m
    DEPENDENCIES system objc Foundation)

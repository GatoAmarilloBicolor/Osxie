project(StoreKit_SwiftUI)
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
set(FRAMEWORK_VERSION "A")
remove_sdk_framework(_StoreKit_SwiftUI)
generate_sdk_framework(_StoreKit_SwiftUI VERSION ${FRAMEWORK_VERSION} HEADER "include/StoreKit_SwiftUI")
add_framework(_StoreKit_SwiftUI FAT CURRENT_VERSION VERSION ${FRAMEWORK_VERSION} TARGET_NAME StoreKit_SwiftUI_FW
    SOURCES src/stubs.m
    DEPENDENCIES system objc Foundation)

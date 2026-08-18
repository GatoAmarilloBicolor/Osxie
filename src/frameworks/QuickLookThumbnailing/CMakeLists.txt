project(QuickLookThumbnailing)
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
set(FRAMEWORK_VERSION "A")
remove_sdk_framework(QuickLookThumbnailing)
generate_sdk_framework(QuickLookThumbnailing VERSION ${FRAMEWORK_VERSION} HEADER "include/QuickLookThumbnailing")
add_framework(QuickLookThumbnailing FAT CURRENT_VERSION VERSION ${FRAMEWORK_VERSION} TARGET_NAME QuickLookThumbnailing_FW
    SOURCES src/stubs.m
    DEPENDENCIES system objc Foundation)

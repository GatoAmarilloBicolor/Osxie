project(MediaPlayer)
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
set(FRAMEWORK_VERSION "A")
remove_sdk_framework(MediaPlayer)
generate_sdk_framework(MediaPlayer VERSION ${FRAMEWORK_VERSION} HEADER "include/MediaPlayer")
add_framework(MediaPlayer FAT CURRENT_VERSION VERSION ${FRAMEWORK_VERSION} TARGET_NAME MediaPlayer_FW
    SOURCES src/stubs.m
    DEPENDENCIES system objc Foundation AppKit)

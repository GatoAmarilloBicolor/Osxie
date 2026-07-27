project(Security)

include(darling_framework)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc")

add_compile_options(
    -Wno-deprecated-declarations
)

set(Security_SRCS
    Security.c
)

set(DYLIB_INSTALL_NAME "/System/Library/Frameworks/Security.framework/Versions/A/Security")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_darling_library(Security SHARED ${Security_SRCS})
target_link_libraries(Security
    system
    CoreFoundation
    ssl
    crypto
)

set_target_properties(Security PROPERTIES
    FRAMEWORK TRUE
    FRAMEWORK_VERSION A
    MACOSX_FRAMEWORK_IDENTIFIER com.osxie.Security
    VERSION ${DYLIB_CURRENT_VERSION}
    SOVERSION ${DYLIB_COMPAT_VERSION}
)

install(TARGETS Security
    FRAMEWORK DESTINATION /System/Library/Frameworks
    COMPONENT Security
)

install(FILES Security.h
    DESTINATION /System/Library/Frameworks/Security.framework/Headers
    COMPONENT Security
)
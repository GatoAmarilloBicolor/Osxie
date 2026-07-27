project(LAPACK)

set(DYLIB_INSTALL_NAME "/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A/libLAPACK.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_osxie_library(LAPACK SHARED
    src/LAPACK.c
)
make_fat(LAPACK)
target_link_libraries(LAPACK system)
install(TARGETS LAPACK DESTINATION libexec/osxie/usr/lib)

set_property(TARGET LAPACK PROPERTY DYLIB_INSTALL_NAME ${DYLIB_INSTALL_NAME})
set_property(TARGET LAPACK PROPERTY DYLIB_BUILD_NAME libLAPACK.dylib)
install(TARGETS LAPACK DESTINATION libexec/osxie/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A)
InstallSymlink(
	Versions/A/libLAPACK.dylib
	${CMAKE_INSTALL_PREFIX}/libexec/osxie/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/libLAPACK.dylib
)

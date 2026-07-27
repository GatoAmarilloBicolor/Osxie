project(BLAS)

set(DYLIB_INSTALL_NAME "/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A/libBLAS.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_osxie_library(BLAS SHARED
    src/BLAS.c
)
make_fat(BLAS)
target_link_libraries(BLAS system)
install(TARGETS BLAS DESTINATION libexec/osxie/usr/lib)

set_property(TARGET BLAS PROPERTY DYLIB_INSTALL_NAME ${DYLIB_INSTALL_NAME})
set_property(TARGET BLAS PROPERTY DYLIB_BUILD_NAME libBLAS.dylib)
install(TARGETS BLAS DESTINATION libexec/osxie/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A)
InstallSymlink(
	Versions/A/libBLAS.dylib
	${CMAKE_INSTALL_PREFIX}/libexec/osxie/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/libBLAS.dylib
)

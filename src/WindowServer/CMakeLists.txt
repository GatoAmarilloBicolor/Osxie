project(WindowServer)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=11.0")
set(CMAKE_AR "/usr/bin/llvm-ar")
set(CMAKE_RANLIB "/usr/bin/llvm-ranlib")

add_library(WindowServer STATIC
    OSXWindowServer.c
    CGSEventPipeline.c
    CGSWindow.c
    X11/X11Keymap.c
    X11/X11Display.c
)

target_include_directories(WindowServer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/X11
    ${CMAKE_SOURCE_DIR}/framework-include
)

find_package(X11 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(XTST REQUIRED xtst xi xfixes xkbcommon)

target_link_libraries(WindowServer
    ${X11_LIBRARIES}
    ${XTST_LIBRARIES}
    pthread
)

target_compile_options(WindowServer PRIVATE
    -isystem /usr/include
)

target_include_directories(WindowServer PRIVATE
    ${X11_INCLUDE_DIR}
    ${XTST_INCLUDE_DIRS}
)

install(TARGETS WindowServer
    LIBRARY DESTINATION lib
)

install(FILES OSXWindowServer.h CGSEvent.h
    DESTINATION include/WindowServer
)
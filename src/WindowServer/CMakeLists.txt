project(WindowServer)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -mmacosx-version-min=11.0")

add_library(WindowServer STATIC
    OSXWindowServer.c
)

target_include_directories(WindowServer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/external/cocotron/CoreGraphics
)

find_package(X11 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(XTST REQUIRED xtst)

target_link_libraries(WindowServer
    ${X11_LIBRARIES}
    ${XTST_LIBRARIES}
    pthread
)

target_include_directories(WindowServer PRIVATE
    ${X11_INCLUDE_DIR}
    ${XTST_INCLUDE_DIRS}
)

install(TARGETS WindowServer
    LIBRARY DESTINATION lib
)

install(FILES OSXWindowServer.h
    DESTINATION include/WindowServer
)
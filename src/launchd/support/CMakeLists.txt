project(launchd-support)

cmake_minimum_required(VERSION 3.13)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

add_definitions(
	-w -nostdinc
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../liblaunch/)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -fblocks -include ${CMAKE_SOURCE_DIR}/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/sys/fileport.h")

add_definitions(-DPRIVATE)

include(osxie_exe)
add_osxie_executable(launchctl launchctl.c)
target_link_libraries(launchctl xpc CoreFoundation edit)

add_osxie_executable(launchproxy launchproxy.c)
target_link_libraries(launchproxy xpc)

install(TARGETS launchctl DESTINATION libexec/osxie/bin)
install(TARGETS launchproxy DESTINATION libexec/osxie/usr/libexec)


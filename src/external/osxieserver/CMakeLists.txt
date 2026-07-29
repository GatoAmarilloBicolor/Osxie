project(osxieserver)

cmake_minimum_required(VERSION 3.13)

option(DSERVER_TOOLS "Build osxieserver tools (for debugging)" OFF)
option(DSERVER_ASAN "Build osxieserver with ASAN" OFF)
option(DSERVER_UBSAN "Build osxieserver with UBSAN" OFF)

if (DSERVER_TOOLS)
	add_subdirectory(tools)
endif()

if (DSERVER_ASAN)
	add_compile_definitions(
		DSERVER_ASAN=1
	)
else()
	add_compile_definitions(
		DSERVER_ASAN=0
	)
endif()

if (DSERVER_UBSAN)
	add_compile_definitions(
		DSERVER_UBSAN=1
	)
else()
	add_compile_definitions(
		DSERVER_UBSAN=0
	)
endif()

set(dserver_sanitizers "")
if (DSERVER_ASAN)
	if (NOT (dserver_sanitizers STREQUAL ""))
		set(dserver_sanitizers "${dserver_sanitizers},")
	endif()
	set(dserver_sanitizers "${dserver_sanitizers}address")
endif()
if (DSERVER_UBSAN)
	if (NOT (dserver_sanitizers STREQUAL ""))
		set(dserver_sanitizers "${dserver_sanitizers},")
	endif()
	set(dserver_sanitizers "${dserver_sanitizers}undefined")
endif()

option(DSERVER_EXTENDED_DEBUG "Build osxieserver with extended debugging support" OFF)

if (DSERVER_EXTENDED_DEBUG)
	add_compile_definitions(
		DSERVER_EXTENDED_DEBUG=1
	)
else()
	add_compile_definitions(
		DSERVER_EXTENDED_DEBUG=0
	)
endif()

option(DSERVER_SINGLE_THREADED "Only use a single thread per workqueue in osxieserver" ON)

if (DSERVER_SINGLE_THREADED)
	add_compile_definitions(
		DSERVER_SINGLE_THREADED=1
	)
else()
	add_compile_definitions(
		DSERVER_SINGLE_THREADED=0
	)
endif()

add_subdirectory(duct-tape)

set(OSXIESERVER_INIT_PROCESS "/sbin/launchd" CACHE STRING "The init process osxieserver should execute (default is \"/sbin/launchd\")")
set(OSXIESERVER_XDG_USER_DIR_CMD "xdg-user-dir" CACHE STRING "The xdg-user-dir binary osxieserver should execute (default is \"xdg-user-dir\")")

include_directories(
	include
	internal-include
	duct-tape/include
	${CMAKE_CURRENT_BINARY_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}/internal-include
	${CMAKE_BINARY_DIR}/src/startup # for `rtsig.h`
)

add_custom_command(
	OUTPUT
		${CMAKE_CURRENT_BINARY_DIR}/include/osxieserver/rpc.h
		${CMAKE_CURRENT_BINARY_DIR}/internal-include/osxieserver/rpc.internal.h
		${CMAKE_CURRENT_BINARY_DIR}/src/rpc.c
	COMMAND
		${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate-rpc-wrappers.py
		${CMAKE_CURRENT_BINARY_DIR}/include/osxieserver/rpc.h
		${CMAKE_CURRENT_BINARY_DIR}/internal-include/osxieserver/rpc.internal.h
		${CMAKE_CURRENT_BINARY_DIR}/src/rpc.c
		\"../include/osxieserver/rpc.h\"
	VERBATIM
	MAIN_DEPENDENCY
		${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate-rpc-wrappers.py
)

add_custom_target(generate_dserver_rpc_wrappers
	ALL
	DEPENDS
		${CMAKE_CURRENT_BINARY_DIR}/include/osxieserver/rpc.h
		${CMAKE_CURRENT_BINARY_DIR}/internal-include/osxieserver/rpc.internal.h
		${CMAKE_CURRENT_BINARY_DIR}/src/rpc.c
)

add_executable(osxieserver
	src/osxieserver.cpp
	src/server.cpp
	src/message.cpp
	src/call.cpp
	src/registry.cpp
	src/logging.cpp
	src/process.cpp
	src/thread.cpp
	src/utility.cpp
	src/kqchan.cpp
	src/async-writer.cpp
	src/stack-pool.cpp
)

add_dependencies(osxieserver
	generate_dserver_rpc_wrappers
	rtsig_h
)

target_link_libraries(osxieserver PRIVATE
	osxieserver_duct_tape
)

target_compile_options(osxieserver PRIVATE
	-pthread
	-std=c++17
)
target_link_options(osxieserver PRIVATE
	-pthread
	-Wl,--gc-sections
)

if (DSERVER_ASAN OR DSERVER_UBSAN)
	target_compile_options(osxieserver PRIVATE
		-fsanitize=${dserver_sanitizers}
		-fsanitize-recover=all
		-fno-omit-frame-pointer
		-g
		-Og
	)
	target_link_options(osxieserver PRIVATE
		-fsanitize=${dserver_sanitizers}
		-fsanitize-recover=all
	)
endif()

if (DSERVER_EXTENDED_DEBUG)
	target_compile_options(osxieserver PRIVATE
		-fno-omit-frame-pointer
	)
endif()

install(TARGETS osxieserver DESTINATION bin)

#file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${OSXIE_SDK_RELATIVE_PATH}/usr/include/osxieserver")
#create_symlink(
#	"${OSXIE_ROOT_RELATIVE_TO_SDK}/../../../src/osxieserver/include/osxieserver/rpc.h"
#	"${CMAKE_BINARY_DIR}/${OSXIE_SDK_RELATIVE_PATH}/usr/include/osxieserver/rpc.h"
#)

if (NOT "${OSXIESERVER_INIT_PROCESS}" STREQUAL "")
	target_compile_definitions(osxieserver PRIVATE
		OSXIESERVER_INIT_PROCESS="${OSXIESERVER_INIT_PROCESS}"
	)
endif()

if (NOT "${OSXIESERVER_XDG_USER_DIR_CMD}" STREQUAL "")
	target_compile_definitions(osxieserver PRIVATE
		OSXIESERVER_XDG_USER_DIR_CMD="${OSXIESERVER_XDG_USER_DIR_CMD}"
	)
endif()

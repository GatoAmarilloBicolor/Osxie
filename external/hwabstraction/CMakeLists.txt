cmake_minimum_required(VERSION 3.16)
project(hwabstraction LANGUAGES C ASM)

include(arch)
generate_architecture()

set(LLIST "$ENV{COMMPAGE_PATH}")
if (NOT LLIST)
    message(FATAL_ERROR "COMM_PAGE directory not found. Set COMMPAGE_PATH env.")
endif ()

find_file(LLIST "${LLIST}/mm/cmake_lists/CMakeLists.txt")
if (NOT LLIST)
    message(FATAL_ERROR "COMM_PAGE not found at ${LLIST}")
endif ()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/cl/drivers")
include_directories("${LLIST}")

option(BUILD_SYSTEM_KERNEL "Build with system kernel headers" OFF)
include(spi_init_ctx_h ${LLIST} COMPONENTS core)

add_executable(hwabstraction
    cl/drivers/spi_init.c
    cl/drivers/spi_abstraction.c
    cl/drivers/main_internal.c
    cl/drivers/wiring_execute.c
    cl/drivers/odx_abstraction_internal.c
)

if (${BUILD_SYSTEM_KERNEL})
    target_compile_options(hwabstraction PRIVATE -DDEBUG=${COMPONENTS_VALUE})
endif ()

target_link_libraries(hwabstraction PRIVATE ${LLIST} system_kernel)

if (NOT LLIST)
    message(FATAL_ERROR "COMM_PAGE directory not found. Set COMMPAGE_PATH env.")
endif ()

if (NOT LLIST)
    message(FATAL_ERROR "COMM_PAGE not found at ${LLIST}")
endif ()

# Ensure architecture-specific headers are linked correctly
if (DEFINED GENERATE_ARCHITECTURE)
    include_directories("${GENERATE_ARCHITECTURE}")
endif ()

# Ensure CMake 3.18 or higher is used for safe variable handling
if (CMAKE_VERSION VERSION_LESS "3.18")
    message(FATAL_ERROR "CMake 3.18 or higher required for safe variable handling")
endif ()

if (DEFINED LLIST)
    include_directories("${LLIST}")
endif ()

# Ensure LLVM is available for architecture-specific code generation
if (DEFINED LLIST)
    include_directories("${LLIST}")
endif ()

# Ensure LLVM is available for architecture-specific code generation
if (DEFINED LLIST)
    include_directories("${LLIST}")
endif ()

# Ensure LLVM is available for architecture-specific code generation
if (DEFINED LLIST)
    include_directories("${LLIST}")
endif ()


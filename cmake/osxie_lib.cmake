if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
    cmake_policy(SET CMP0011 NEW)
endif(COMMAND cmake_policy)

include(use_ld64)
include(CMakeParseArguments)
include(dsym)

FUNCTION(add_osxie_library name)
	foreach(f IN LISTS ARGN)
		set(files ${files} ${f})
	endforeach(f)

	set(CMAKE_SKIP_RPATH TRUE)
	add_library(${name} SHARED ${files})

	set_target_properties(${name} PROPERTIES
		SUFFIX ".dylib"
		NO_SONAME TRUE)

	set_target_properties(${name} PROPERTIES PREFIX "lib")

	set_property(TARGET ${name} APPEND_STRING PROPERTY
		LINK_FLAGS " ${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

	if (DYLIB_INSTALL_NAME)
		set_property(TARGET ${name} APPEND_STRING PROPERTY
			LINK_FLAGS " -Wl,-dylib_install_name,${DYLIB_INSTALL_NAME} ")
	endif (DYLIB_INSTALL_NAME)
	if (DYLIB_COMPAT_VERSION)
		set_property(TARGET ${name} APPEND_STRING PROPERTY
			LINK_FLAGS " -Wl,-compatibility_version,${DYLIB_COMPAT_VERSION} ")
	else (DYLIB_COMPAT_VERSION)
		set_property(TARGET ${name} APPEND_STRING PROPERTY
			LINK_FLAGS " -Wl,-compatibility_version,1.0.0 ")
	endif (DYLIB_COMPAT_VERSION)
	if (DYLIB_CURRENT_VERSION)
		set_property(TARGET ${name} APPEND_STRING PROPERTY
			LINK_FLAGS " -Wl,-current_version,${DYLIB_CURRENT_VERSION} ")
	endif (DYLIB_CURRENT_VERSION)

	set_property(TARGET ${name} APPEND_STRING PROPERTY COMPILE_FLAGS  " -B ${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/")
	add_dependencies(${name} lipo)

	if (BUILD_TARGET_64BIT AND NOT OSXIE_LIB_32BIT_ONLY)
		set_property(TARGET ${name} APPEND_STRING PROPERTY COMPILE_FLAGS  " -arch ${APPLE_ARCH_64BIT}")
		set_property(TARGET ${name} APPEND_STRING PROPERTY LINK_FLAGS " -arch ${APPLE_ARCH_64BIT}")
	endif (BUILD_TARGET_64BIT AND NOT OSXIE_LIB_32BIT_ONLY)
	if (BUILD_TARGET_32BIT AND NOT OSXIE_LIB_64BIT_ONLY)
		set_property(TARGET ${name} APPEND_STRING PROPERTY COMPILE_FLAGS " -arch ${APPLE_ARCH_32BIT}")
		set_property(TARGET ${name} APPEND_STRING PROPERTY LINK_FLAGS " -arch ${APPLE_ARCH_32BIT}")
	endif (BUILD_TARGET_32BIT AND NOT OSXIE_LIB_64BIT_ONLY)

	use_ld64(${name})

	if ((NOT NO_DSYM) AND (NOT ${name}_NO_DSYM))
		dsym(${name})
	endif ((NOT NO_DSYM) AND (NOT ${name}_NO_DSYM))
ENDFUNCTION(add_osxie_library)

FUNCTION(make_fat)
	if (BUILD_TARGET_32BIT AND BUILD_TARGET_64BIT)
		set_property(TARGET ${ARGV} APPEND_STRING PROPERTY
			COMPILE_FLAGS " -B ${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/ -arch ${APPLE_ARCH_32BIT} -arch ${APPLE_ARCH_64BIT}")
		set_property(TARGET ${ARGV} APPEND_STRING PROPERTY
			LINK_FLAGS " -arch ${APPLE_ARCH_32BIT} -arch ${APPLE_ARCH_64BIT}")
		foreach(tgt ${ARGV})
			add_dependencies(${tgt} lipo)
		endforeach(tgt)
	elseif (BUILD_TARGET_32BIT)
		set_property(TARGET ${ARGV} APPEND_STRING PROPERTY
			COMPILE_FLAGS " -B ${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/ -arch ${APPLE_ARCH_32BIT}")
		set_property(TARGET ${ARGV} APPEND_STRING PROPERTY
			LINK_FLAGS " -arch ${APPLE_ARCH_32BIT}")
		foreach(tgt ${ARGV})
			add_dependencies(${tgt} lipo)
		endforeach(tgt)
	elseif (BUILD_TARGET_64BIT)
		set_property(TARGET ${ARGV} APPEND_STRING PROPERTY
			COMPILE_FLAGS " -B ${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/ -arch ${APPLE_ARCH_64BIT}")
		set_property(TARGET ${ARGV} APPEND_STRING PROPERTY
			LINK_FLAGS " -arch ${APPLE_ARCH_64BIT}")
		foreach(tgt ${ARGV})
			add_dependencies(${tgt} lipo)
		endforeach(tgt)
	endif (BUILD_TARGET_32BIT AND BUILD_TARGET_64BIT)
ENDFUNCTION(make_fat)

FUNCTION(add_circular name)
	cmake_parse_arguments(CIRCULAR "FAT" "LINK_FLAGS" "SOURCES;OBJECTS;SIBLINGS;STRONG_SIBLINGS;DEPENDENCIES;UPWARD;STRONG_DEPENDENCIES" ${ARGN})

	set(all_objects "${CIRCULAR_OBJECTS}")

	if (CIRCULAR_SOURCES)
		add_library("${name}_obj" OBJECT ${CIRCULAR_SOURCES})
		if (CIRCULAR_FAT)
			make_fat("${name}_obj")
		endif (CIRCULAR_FAT)
		set(all_objects "${all_objects};$<TARGET_OBJECTS:${name}_obj>")
	endif (CIRCULAR_SOURCES)

	add_osxie_library("${name}_firstpass" SHARED ${all_objects})
	set_property(TARGET "${name}_firstpass" APPEND_STRING PROPERTY LINK_FLAGS " ${CIRCULAR_LINK_FLAGS} -Wl,-flat_namespace -Wl,-undefined,suppress")

	foreach(dep ${CIRCULAR_STRONG_SIBLINGS})
		target_link_libraries("${name}_firstpass" PRIVATE "${dep}_firstpass")
	endforeach(dep)

	target_link_libraries("${name}_firstpass" PRIVATE ${CIRCULAR_STRONG_DEPENDENCIES})
	
	if (CIRCULAR_FAT)
		make_fat("${name}_firstpass")
	endif (CIRCULAR_FAT)

	add_osxie_library(${name} SHARED ${all_objects})

	foreach(dep ${CIRCULAR_SIBLINGS})
		target_link_libraries("${name}" PRIVATE "${dep}_firstpass")
	endforeach(dep)
	foreach(dep ${CIRCULAR_UPWARD})
		target_link_libraries("${name}" PRIVATE -Wl,-upward_library,$<TARGET_FILE:${dep}_firstpass>)
		add_dependencies("${name}" "${dep}_firstpass")
	endforeach(dep)
	
	get_property(dylib_files GLOBAL PROPERTY FIRSTPASS_MAP)
	set_property(TARGET "${name}" APPEND_STRING PROPERTY LINK_FLAGS " ${CIRCULAR_LINK_FLAGS}")

	target_link_libraries("${name}" PRIVATE ${CIRCULAR_DEPENDENCIES})

	target_link_libraries("${name}" PRIVATE ${CIRCULAR_STRONG_DEPENDENCIES})

	if (CIRCULAR_FAT)
		make_fat(${name})
	endif (CIRCULAR_FAT)
ENDFUNCTION(add_circular)

function(add_osxie_object_library name)
	cmake_parse_arguments(OBJECT_LIB "32BIT_ONLY;64BIT_ONLY" "" "" ${ARGN})
	foreach(f IN LISTS OBJECT_LIB_UNPARSED_ARGUMENTS)
		set(files ${files} ${f})
	endforeach(f)

	add_library(${name} OBJECT ${files})
	add_dependencies(${name} lipo)
	set_property(TARGET ${name} APPEND_STRING PROPERTY COMPILE_FLAGS " -B ${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/")

	if (BUILD_TARGET_32BIT AND NOT OBJECT_LIB_64BIT_ONLY)
		set_property(TARGET ${name} APPEND_STRING PROPERTY COMPILE_FLAGS " -arch ${APPLE_ARCH_32BIT}")
	endif (BUILD_TARGET_32BIT AND NOT OBJECT_LIB_64BIT_ONLY)
	if (BUILD_TARGET_64BIT AND NOT OBJECT_LIB_32BIT_ONLY)
		set_property(TARGET ${name} APPEND_STRING PROPERTY COMPILE_FLAGS " -arch ${APPLE_ARCH_64BIT}")
	endif (BUILD_TARGET_64BIT AND NOT OBJECT_LIB_32BIT_ONLY)
endfunction(add_osxie_object_library)

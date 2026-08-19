project(dbus)

include(osxie_lib)

add_osxie_object_library(osxie_dbus_obj
	osxie_dbus.c
)

target_include_directories(osxie_dbus_obj PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	/usr/include/dbus-1.0
	/usr/lib/dbus-1.0/include
)

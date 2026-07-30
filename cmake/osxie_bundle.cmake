FUNCTION(add_osxie_bundle name loader)
	add_osxie_library(${name} ${ARGN})
	set_property(TARGET ${name} APPEND_STRING PROPERTY
		LINK_FLAGS " -Wl,-bundle ")

	if (loader)
		set_property(TARGET ${name} APPEND_STRING PROPERTY
			LINK_FLAGS " -Wl,-bundle_loader,${loader} ")
	endif (loader)
ENDFUNCTION(add_osxie_bundle)

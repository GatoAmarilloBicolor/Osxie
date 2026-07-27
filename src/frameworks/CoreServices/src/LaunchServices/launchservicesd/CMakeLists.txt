project(launchservicesd)

set(sources
	launchservicesd.m
	LSBundle.m
)

add_osxie_executable(launchservicesd ${sources})

target_link_libraries(launchservicesd system Foundation CoreFoundation CoreServices FMDB sqlite3 z)
install(TARGETS launchservicesd DESTINATION libexec/osxie/System/Library/CoreServices)
install(FILES schema.sql RENAME launchservicesd-schema.sql DESTINATION libexec/osxie/System/Library/Frameworks/CoreServices.framework/Versions/A/Resources)

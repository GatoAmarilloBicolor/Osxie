project(shellspawn)

add_definitions(-nostdinc)

add_osxie_executable(shellspawn shellspawn.c duct_signals.c)

install(TARGETS shellspawn DESTINATION libexec/osxie/usr/libexec)
install(FILES org.osxie.shellspawn.plist DESTINATION libexec/osxie/System/Library/LaunchDaemons)


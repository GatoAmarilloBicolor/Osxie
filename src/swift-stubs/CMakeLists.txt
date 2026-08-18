project(swift-stubs)
add_library(swiftUniformTypeIdentifiers SHARED stub.c)
set_target_properties(swiftUniformTypeIdentifiers PROPERTIES
    PREFIX "lib"
    SUFFIX ".dylib"
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SWITCHTOGREETER REQUIRED glib-2.0 gio-unix-2.0 libsystemd)

add_executable(dde-switchtogreeter switchtogreeter.c)

target_compile_options(dde-switchtogreeter PRIVATE -std=c99)
target_link_libraries(dde-switchtogreeter PRIVATE ${SWITCHTOGREETER_LIBRARIES})
target_include_directories(dde-switchtogreeter PRIVATE ${SWITCHTOGREETER_INCLUDE_DIRS})

install(TARGETS dde-switchtogreeter DESTINATION ${PREFIX}/bin)

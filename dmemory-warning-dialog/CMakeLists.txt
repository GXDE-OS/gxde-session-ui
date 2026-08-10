find_package(PkgConfig REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets Concurrent X11Extras DBus)
find_package(Dtk REQUIRED COMPONENTS Widget)

pkg_check_modules(DMW REQUIRED x11 dframeworkdbus dtkwidget)

file(GLOB SRCS "src/*.h" "src/*.cpp")

add_executable(dmemory-warning-dialog ${SRCS})

target_include_directories(dmemory-warning-dialog PRIVATE ${DMW_INCLUDE_DIRS})
target_link_libraries(dmemory-warning-dialog PRIVATE
    global_util
    ${DMW_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${Qt5X11Extras_LIBRARIES}
)

install(TARGETS dmemory-warning-dialog DESTINATION /usr/bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/com.deepin.dde.MemoryWarningDialog.service
    DESTINATION /usr/share/dbus-1/services/)

find_package(PkgConfig REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets DBus)

pkg_check_modules(WARNING REQUIRED dtkwidget)

add_executable(dde-warning-dialog
    main.cpp
    warningdialog.cpp
)

target_link_libraries(dde-warning-dialog PRIVATE
    ${Qt5Widgets_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${WARNING_LIBRARIES}
)
target_include_directories(dde-warning-dialog PRIVATE ${WARNING_INCLUDE_DIRS})

install(TARGETS dde-warning-dialog DESTINATION /usr/lib/deepin-daemon/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/com.deepin.dde.WarningDialog.service
    DESTINATION /usr/share/dbus-1/services/)

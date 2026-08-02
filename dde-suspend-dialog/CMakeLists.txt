find_package(PkgConfig REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets)

pkg_check_modules(SUSPEND REQUIRED dtkwidget)

add_executable(dde-suspend-dialog
    main.cpp
    suspenddialog.cpp
)

target_compile_definitions(dde-suspend-dialog PRIVATE
    QT_DISABLE_DEPRECATED_BEFORE=0x060000
)
target_link_libraries(dde-suspend-dialog PRIVATE
    ${Qt5Widgets_LIBRARIES}
    ${SUSPEND_LIBRARIES}
)
target_include_directories(dde-suspend-dialog PRIVATE ${SUSPEND_INCLUDE_DIRS})

install(TARGETS dde-suspend-dialog DESTINATION /usr/lib/deepin-daemon/)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/
    DESTINATION /usr/share/icons/hicolor/scalable/devices
    FILES_MATCHING PATTERN "*"
)

find_package(PkgConfig REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets DBus Xml X11Extras Svg)

pkg_check_modules(WELCOME REQUIRED dtkwidget dframeworkdbus gsettings-qt x11 xext xfixes xcursor)

add_executable(dde-welcome
    main.cpp
    updatecontent.cpp
    mainwidget.cpp
    utils.cpp
    resources.qrc
)

target_link_libraries(dde-welcome PRIVATE
    session-widgets
    ${Qt5Widgets_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${Qt5X11Extras_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    ${Qt5Svg_LIBRARIES}

    ${WELCOME_LIBRARIES}
)
target_include_directories(dde-welcome PRIVATE ${WELCOME_INCLUDE_DIRS})

install(TARGETS dde-welcome DESTINATION /usr/lib/deepin-daemon/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/com.deepin.dde.welcome.service
    DESTINATION /usr/share/dbus-1/services/)

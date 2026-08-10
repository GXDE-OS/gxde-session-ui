find_package(PkgConfig REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets DBus Xml X11Extras Svg Sql)

pkg_check_modules(OSD REQUIRED dframeworkdbus dtkwidget gsettings-qt xcb-ewmh)

set(OSD_MAIN_SOURCES
    main.cpp
    container.cpp
    manager.cpp
    listview.cpp
    delegate.cpp
    model.cpp
    abstractosdprovider.cpp
    osdprovider.cpp
    audioprovider.cpp
    common.cpp
    brightnessprovider.cpp
    kblayoutprovider.cpp
    displaymodeprovider.cpp
    indicatorprovider.cpp
    kblayoutindicator.cpp
)

set(OSD_NOTIFICATION_SOURCES
    notification/bubble.cpp
    notification/bubblemanager.cpp
    notification/notifications_dbus_adaptor.cpp
    notification/dbus_daemon_interface.cpp
    notification/notificationentity.cpp
    notification/dbuslogin1manager.cpp
    notification/actionbutton.cpp
    notification/appicon.cpp
    notification/dbusdock_interface.cpp
    notification/dbuscontrol.cpp
    notification/persistence.cpp
    notification/appbody.cpp
    notification/icondata.cpp
    notification/appbodylabel.cpp
)

set(OSD_SOURCES ${OSD_MAIN_SOURCES} ${OSD_NOTIFICATION_SOURCES})

set(OSD_RESOURCES
    image.qrc
    theme.qrc
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/notification/files/com.deepin.dde.freedesktop.Notification.service.in
    ${CMAKE_BINARY_DIR}/files/com.deepin.dde.freedesktop.Notification.service
    @ONLY
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/notification/files/com.deepin.dde.Notification.service.in
    ${CMAKE_BINARY_DIR}/files/com.deepin.dde.Notification.service
    @ONLY
)

add_executable(dde-osd ${OSD_SOURCES} ${OSD_RESOURCES})

target_include_directories(dde-osd PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/notification
)
target_link_libraries(dde-osd PRIVATE
    global_util
    ${Qt5Widgets_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${Qt5X11Extras_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5Sql_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    ${OSD_LIBRARIES}
)
target_include_directories(dde-osd PRIVATE ${OSD_INCLUDE_DIRS})
target_link_libraries(dde-osd PRIVATE -lX11 -lXext)

install(TARGETS dde-osd DESTINATION ${PREFIX}/lib/deepin-daemon/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/com.deepin.dde.osd.service
    DESTINATION ${PREFIX}/share/dbus-1/services/)
install(FILES
    ${CMAKE_BINARY_DIR}/files/com.deepin.dde.freedesktop.Notification.service
    ${CMAKE_BINARY_DIR}/files/com.deepin.dde.Notification.service
    DESTINATION ${PREFIX}/share/dbus-1/services/
)

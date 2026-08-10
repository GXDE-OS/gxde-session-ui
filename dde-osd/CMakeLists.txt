find_package(PkgConfig REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets DBus Xml Svg Sql)

# libdframeworkdbus built against Qt6 only ships a CMake package, no .pc file.
find_package(DFrameworkdbusQt6 REQUIRED)

# dtk2widget is the Qt6 build of the DtkWidget 5.x API (DWIDGET_NAMESPACE = Dtk::Widget).
# It must NOT be replaced by dtk6widget, whose API differs significantly.
pkg_check_modules(OSD REQUIRED IMPORTED_TARGET
    dtk2widget
    gsettings-qt6
    xcb-ewmh
    x11
    xext
)

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

# dde-osd only needs the session detection helper from global_util. That library
# is still built against Qt5, so compile the single translation unit directly
# instead of linking the Qt5 static library.
set(OSD_SHARED_SOURCES
    ${CMAKE_SOURCE_DIR}/global_util/sessiontype.cpp
)

set(OSD_SOURCES
    ${OSD_MAIN_SOURCES}
    ${OSD_NOTIFICATION_SOURCES}
    ${OSD_SHARED_SOURCES}
)

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

# Qt6 requires C++17, the rest of the project is still built as C++14.
target_compile_features(dde-osd PRIVATE cxx_std_17)

target_include_directories(dde-osd PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/notification
    ${CMAKE_SOURCE_DIR}/global_util
)

target_link_libraries(dde-osd PRIVATE
    Qt6::Widgets
    Qt6::DBus
    Qt6::Xml
    Qt6::Svg
    Qt6::Sql
    DFrameworkdbusQt6::DFrameworkdbusQt6
    PkgConfig::OSD
)

install(TARGETS dde-osd DESTINATION ${PREFIX}/lib/deepin-daemon/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/com.deepin.dde.osd.service
    DESTINATION ${PREFIX}/share/dbus-1/services/)
install(FILES
    ${CMAKE_BINARY_DIR}/files/com.deepin.dde.freedesktop.Notification.service
    ${CMAKE_BINARY_DIR}/files/com.deepin.dde.Notification.service
    DESTINATION ${PREFIX}/share/dbus-1/services/
)

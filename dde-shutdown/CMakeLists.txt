find_package(PkgConfig REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets DBus Xml X11Extras Svg)

pkg_check_modules(SHUTDOWN REQUIRED dframeworkdbus dtkcore)

set(SHUTDOWN_SOURCES
    main.cpp
    shutdownworker.cpp
    dbusshutdownagent.cpp
    view/contentwidget.cpp
    view/multiuserswarningview.cpp
    view/inhibitwarnview.cpp
    view/systemmonitor.cpp
    app/shutdownframe.cpp
)

set(SHUTDOWN_RESOURCES
    qss.qrc
    image.qrc
    ${CMAKE_SOURCE_DIR}/widgets/widgetstheme.qrc
    ${CMAKE_SOURCE_DIR}/widgets/widgetsimages.qrc
)

configure_file(
    ${CMAKE_SOURCE_DIR}/files/dde-shutdown.conf.in
    ${CMAKE_BINARY_DIR}/files/dde-shutdown.conf
    @ONLY
)

add_executable(dde-shutdown ${SHUTDOWN_SOURCES} ${SHUTDOWN_RESOURCES})

target_link_libraries(dde-shutdown PRIVATE
    session-widgets
    ${Qt5Widgets_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5X11Extras_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    ${SHUTDOWN_LIBRARIES}
)
target_include_directories(dde-shutdown PRIVATE ${SHUTDOWN_INCLUDE_DIRS})

install(TARGETS dde-shutdown DESTINATION ${PREFIX}/bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/com.deepin.dde.shutdownFront.service
    DESTINATION /usr/share/dbus-1/services/)
install(FILES ${CMAKE_BINARY_DIR}/files/dde-shutdown.conf
    DESTINATION ${PREFIX}/share/gxde-session-ui/)

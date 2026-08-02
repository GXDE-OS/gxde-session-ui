add_library(global_util STATIC
    util_updateui.cpp
    xkbparser.cpp
    public_func.cpp
    multiscreenmanager.cpp
    dbus/dbusdisplaymanager.cpp
    dbus/dbuslogin1manager.cpp
    dbus/dbusvariant.cpp
    dbus/dbussessionmanager.cpp
    dbus/displayinterface.cpp
    dbus/dbushotzone.cpp
    dbus/dbuslockservice.cpp
    dbus/dbuskeyboard.cpp
    dbus/dbusinputdevices.cpp
    dbus/dbuscontrolcenter.cpp
    dbus/dbuslockfront.cpp
    dbus/dbusmediaplayer2.cpp
)

target_include_directories(global_util PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

find_package(PkgConfig REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets Xml DBus X11Extras)

pkg_check_modules(GLOBAL_UTIL REQUIRED dframeworkdbus)

target_link_libraries(global_util PUBLIC
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${Qt5X11Extras_LIBRARIES}
    ${GLOBAL_UTIL_LIBRARIES}
)
target_include_directories(global_util PUBLIC ${GLOBAL_UTIL_INCLUDE_DIRS})

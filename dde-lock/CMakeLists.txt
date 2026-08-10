find_package(PkgConfig REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets DBus Xml X11Extras Svg)
find_package(Dtk REQUIRED COMPONENTS Widget Core)

pkg_check_modules(LOCK REQUIRED gsettings-qt xtst xext x11)

add_executable(dde-lock
    main.cpp
    timewidget.cpp
    lockframe.cpp
    lockworker.cpp
    dbus/dbuslockfrontservice.cpp
    dbus/dbuslockagent.cpp
    images.qrc
    logintheme.qrc
    ${CMAKE_SOURCE_DIR}/widgets/widgetstheme.qrc
    ${CMAKE_SOURCE_DIR}/widgets/widgetsimages.qrc
)

target_link_libraries(dde-lock PRIVATE
    session-widgets
    global_util
    ${Qt5Widgets_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${Qt5X11Extras_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    ${Qt5Svg_LIBRARIES}

    ${DtkWidget_LIBRARIES}
    ${LOCK_LIBRARIES}
    -lX11
)
target_include_directories(dde-lock PRIVATE ${LOCK_INCLUDE_DIRS})

string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" LOCK_ARCH)
if (LOCK_ARCH MATCHES "mips64")
    target_compile_definitions(dde-lock PRIVATE DISABLE_LOGIN_ANI)
endif()

if (LOCK_ARCH MATCHES "sw_64|sw64")
    target_compile_definitions(dde-lock PRIVATE DISABLE_LOGIN_ANI SHENWEI_PLATFORM)
endif()

install(TARGETS dde-lock DESTINATION ${PREFIX}/bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/com.deepin.dde.lockFront.service
    DESTINATION /usr/share/dbus-1/services/)

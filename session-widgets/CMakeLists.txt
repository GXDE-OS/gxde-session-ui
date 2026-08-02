find_package(PkgConfig REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets DBus Xml X11Extras Svg)
find_package(Dtk REQUIRED COMPONENTS Widget Core)

pkg_check_modules(SW REQUIRED dframeworkdbus dtkwidget dtkcore liblightdm-qt5-3 x11 xi xtst dframeworkdbus gsettings-qt)

find_program(QDBUSXML2CPP qdbusxml2cpp)

set(AUTH_DBUS_XML_DIR ${CMAKE_SOURCE_DIR}/libdde-auth/dbus)
set(AUTH_ADAPTOR_XML ${AUTH_DBUS_XML_DIR}/com.deepin.daemon.authority.Agent.xml)
set(AUTH_INTERFACE_XML ${AUTH_DBUS_XML_DIR}/com.deepin.daemon.Authority.xml)
set(AUTH_TRANSACTION_XML ${AUTH_DBUS_XML_DIR}/com.deepin.daemon.authority.Transaction.xml)

set(AUTH_ADAPTOR_HEADER ${CMAKE_CURRENT_BINARY_DIR}/agent_adaptor.h)
set(AUTH_ADAPTOR_SOURCE ${CMAKE_CURRENT_BINARY_DIR}/agent_adaptor.cpp)
set(AUTH_INTERFACE_HEADER ${CMAKE_CURRENT_BINARY_DIR}/authority_interface.h)
set(AUTH_INTERFACE_SOURCE ${CMAKE_CURRENT_BINARY_DIR}/authority_interface.cpp)
set(AUTH_TRANSACTION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/transaction_interface.h)
set(AUTH_TRANSACTION_SOURCE ${CMAKE_CURRENT_BINARY_DIR}/transaction_interface.cpp)

add_custom_command(
    OUTPUT ${AUTH_ADAPTOR_HEADER}
    COMMAND ${QDBUSXML2CPP} -a ${AUTH_ADAPTOR_HEADER}: ${AUTH_ADAPTOR_XML}
    DEPENDS ${AUTH_ADAPTOR_XML}
)
add_custom_command(
    OUTPUT ${AUTH_ADAPTOR_SOURCE}
    COMMAND ${QDBUSXML2CPP} -i ${AUTH_ADAPTOR_HEADER} -a :${AUTH_ADAPTOR_SOURCE} ${AUTH_ADAPTOR_XML}
    DEPENDS ${AUTH_ADAPTOR_XML} ${AUTH_ADAPTOR_HEADER}
)
add_custom_command(
    OUTPUT ${AUTH_INTERFACE_HEADER}
    COMMAND ${QDBUSXML2CPP} -p ${AUTH_INTERFACE_HEADER}: ${AUTH_INTERFACE_XML}
    DEPENDS ${AUTH_INTERFACE_XML}
)
add_custom_command(
    OUTPUT ${AUTH_INTERFACE_SOURCE}
    COMMAND ${QDBUSXML2CPP} -i ${AUTH_INTERFACE_HEADER} -p :${AUTH_INTERFACE_SOURCE} ${AUTH_INTERFACE_XML}
    DEPENDS ${AUTH_INTERFACE_XML} ${AUTH_INTERFACE_HEADER}
)
add_custom_command(
    OUTPUT ${AUTH_TRANSACTION_HEADER}
    COMMAND ${QDBUSXML2CPP} -N -p ${AUTH_TRANSACTION_HEADER}: ${AUTH_TRANSACTION_XML}
    DEPENDS ${AUTH_TRANSACTION_XML}
)
add_custom_command(
    OUTPUT ${AUTH_TRANSACTION_SOURCE}
    COMMAND ${QDBUSXML2CPP} -N -i ${AUTH_TRANSACTION_HEADER} -p :${AUTH_TRANSACTION_SOURCE} ${AUTH_TRANSACTION_XML}
    DEPENDS ${AUTH_TRANSACTION_XML} ${AUTH_TRANSACTION_HEADER}
)

set(LIBDDE_AUTH_SOURCES
    ${CMAKE_SOURCE_DIR}/libdde-auth/authagent.cpp
    ${CMAKE_SOURCE_DIR}/libdde-auth/deepinauthframework.cpp
    ${AUTH_ADAPTOR_HEADER}
    ${AUTH_ADAPTOR_SOURCE}
    ${AUTH_INTERFACE_HEADER}
    ${AUTH_INTERFACE_SOURCE}
    ${AUTH_TRANSACTION_HEADER}
    ${AUTH_TRANSACTION_SOURCE}
)

set(SESSION_WIDGETS_SOURCES
    authinterface.cpp
    sessionbasewindow.cpp
    userinputwidget.cpp
    sessionbasemodel.cpp
    userinfo.cpp
    userframe.cpp
    lockcontent.cpp
    framedatabind.cpp
    lockpasswordwidget.cpp
)

set(WIDGETS_SOURCES
    ${CMAKE_SOURCE_DIR}/widgets/useravatar.cpp
    ${CMAKE_SOURCE_DIR}/widgets/userbutton.cpp
    ${CMAKE_SOURCE_DIR}/widgets/sessionwidget.cpp
    ${CMAKE_SOURCE_DIR}/widgets/userbreathinglabel.cpp
    ${CMAKE_SOURCE_DIR}/widgets/rounditembutton.cpp
    ${CMAKE_SOURCE_DIR}/widgets/kblayoutwidget.cpp
    ${CMAKE_SOURCE_DIR}/widgets/shutdownwidget.cpp
    ${CMAKE_SOURCE_DIR}/widgets/darrowrectangle.cpp
    ${CMAKE_SOURCE_DIR}/widgets/errortooltip.cpp
    ${CMAKE_SOURCE_DIR}/widgets/keyboardmonitor.cpp
    ${CMAKE_SOURCE_DIR}/widgets/mediawidget.cpp
    ${CMAKE_SOURCE_DIR}/widgets/controlwidget.cpp
    ${CMAKE_SOURCE_DIR}/widgets/fullscreenbackground.cpp
    ${CMAKE_SOURCE_DIR}/widgets/otheruserinput.cpp
    ${CMAKE_SOURCE_DIR}/widgets/virtualkbinstance.cpp
    ${CMAKE_SOURCE_DIR}/widgets/loginbutton.cpp
    ${CMAKE_SOURCE_DIR}/widgets/propertygroup.cpp
    ${CMAKE_SOURCE_DIR}/widgets/widgetstheme.qrc
    ${CMAKE_SOURCE_DIR}/widgets/widgetsimages.qrc
)

add_library(session-widgets STATIC
    ${LIBDDE_AUTH_SOURCES}
    ${SESSION_WIDGETS_SOURCES}
    ${WIDGETS_SOURCES}
)

target_include_directories(session-widgets BEFORE PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/libdde-auth
    ${CMAKE_SOURCE_DIR}/libdde-auth/interface
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/widgets
    ${CMAKE_SOURCE_DIR}/global_util
    ${CMAKE_SOURCE_DIR}/global_util/dbus
)

target_link_libraries(session-widgets PUBLIC
    global_util
    ${Qt5Widgets_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    ${Qt5X11Extras_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${SW_LIBRARIES}
)
target_include_directories(session-widgets PUBLIC ${SW_INCLUDE_DIRS} ${DtkWidget_INCLUDE_DIRS} ${DtkCore_INCLUDE_DIRS})

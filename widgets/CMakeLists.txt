find_package(PkgConfig REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets DBus Xml X11Extras Svg)

pkg_check_modules(WIDGETS REQUIRED dtkwidget dframeworkdbus liblightdm-qt5-3 x11 xi xtst)

set(WIDGETS_HEADERS
    useravatar.h
    userbutton.h
    sessionwidget.h
    userbreathinglabel.h
    rounditembutton.h
    kblayoutwidget.h
    shutdownwidget.h
    darrowrectangle.h
    errortooltip.h
    keyboardmonitor.h
    mediawidget.h
    controlwidget.h
    fullscreenbackground.h
    otheruserinput.h
    virtualkbinstance.h
    loginbutton.h
    propertygroup.h
)

set(WIDGETS_SOURCES
    useravatar.cpp
    userbutton.cpp
    sessionwidget.cpp
    userbreathinglabel.cpp
    rounditembutton.cpp
    kblayoutwidget.cpp
    shutdownwidget.cpp
    darrowrectangle.cpp
    errortooltip.cpp
    keyboardmonitor.cpp
    mediawidget.cpp
    controlwidget.cpp
    fullscreenbackground.cpp
    otheruserinput.cpp
    virtualkbinstance.cpp
    loginbutton.cpp
    propertygroup.cpp
)

set(WIDGETS_RESOURCES
    widgetstheme.qrc
    widgetsimages.qrc
)

add_library(widgets STATIC ${WIDGETS_SOURCES} ${WIDGETS_RESOURCES})

target_include_directories(widgets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(widgets PUBLIC
    session-widgets
    global_util
    ${WIDGETS_LIBRARIES}
)
target_include_directories(widgets PUBLIC ${WIDGETS_INCLUDE_DIRS})

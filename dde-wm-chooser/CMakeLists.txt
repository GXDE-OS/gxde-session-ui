find_package(PkgConfig REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets DBus Xml X11Extras Svg)

pkg_check_modules(WM_CHOOSER REQUIRED dtkwidget gsettings-qt)

add_executable(dde-wm-chooser
    main.cpp
    wmchooser.cpp
    wmframe.cpp
)

target_link_libraries(dde-wm-chooser PRIVATE
    session-widgets
    global_util
    ${Qt5Widgets_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${Qt5X11Extras_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${WM_CHOOSER_LIBRARIES}
)
target_include_directories(dde-wm-chooser PRIVATE ${WM_CHOOSER_INCLUDE_DIRS})

install(TARGETS dde-wm-chooser DESTINATION /usr/bin)

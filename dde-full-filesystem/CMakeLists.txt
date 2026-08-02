find_package(PkgConfig REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets DBus Xml X11Extras Svg)

pkg_check_modules(FULL_FS REQUIRED gsettings-qt xtst xext x11)

add_executable(dde-full-filesystem
    main.cpp
    fullfilesystem.cpp
    fullfilesystemframe.cpp
    resources.qrc
)

target_link_libraries(dde-full-filesystem PRIVATE
    session-widgets
    ${Qt5Widgets_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${Qt5X11Extras_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    ${Qt5Svg_LIBRARIES}

    ${FULL_FS_LIBRARIES}
)
target_include_directories(dde-full-filesystem PRIVATE ${FULL_FS_INCLUDE_DIRS})

find_package(PkgConfig REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets DBus Xml X11Extras Svg)
find_package(Dtk REQUIRED COMPONENTS Widget Core)

pkg_check_modules(GREETER REQUIRED gsettings-qt liblightdm-qt5-3 x11 xext xfixes xcursor dtkwidget xrandr)

add_executable(lightdm-deepin-greeter
    main.cpp
    greeterworkek.cpp
    view/logowidget.cpp
    loginwindow.cpp
    image.qrc
    logintheme.qrc
)

target_link_libraries(lightdm-deepin-greeter PRIVATE
    session-widgets
    ${Qt5Widgets_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${Qt5X11Extras_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    ${Qt5Svg_LIBRARIES}

    ${GREETER_LIBRARIES}
)
target_include_directories(lightdm-deepin-greeter PRIVATE ${GREETER_INCLUDE_DIRS})

string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" GREETER_ARCH)
if (GREETER_ARCH MATCHES "mips64")
    target_compile_definitions(lightdm-deepin-greeter PRIVATE DISABLE_LOGIN_ANI)
endif()

if (GREETER_ARCH MATCHES "sw_64|sw64")
    target_compile_definitions(lightdm-deepin-greeter PRIVATE DISABLE_LOGIN_ANI SHENWEI_PLATFORM)
endif()

install(TARGETS lightdm-deepin-greeter DESTINATION ${PREFIX}/bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/lightdm-deepin-greeter.desktop
    DESTINATION ${PREFIX}/share/xgreeters/)
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/deepin-greeter
    DESTINATION ${PREFIX}/bin)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/
    DESTINATION /etc/deepin/greeters.d
    FILES_MATCHING PATTERN "*"
)

find_package(PkgConfig REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets DBus)

add_executable(dde-lowpower
    main.cpp
    window.cpp
    images.qrc
)

target_link_libraries(dde-lowpower PRIVATE
    ${Qt5Widgets_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
)

install(TARGETS dde-lowpower DESTINATION /usr/lib/deepin-daemon/)

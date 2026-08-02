find_package(PkgConfig REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets)

pkg_check_modules(NETWORK_SECRET REQUIRED dtkwidget)

add_executable(dnetwork-secret-dialog
    main.cpp
    networksecretdialog.cpp
    resources.qrc
)

target_link_libraries(dnetwork-secret-dialog PRIVATE
    ${Qt5Widgets_LIBRARIES}
    ${NETWORK_SECRET_LIBRARIES}
)
target_include_directories(dnetwork-secret-dialog PRIVATE ${NETWORK_SECRET_INCLUDE_DIRS})

install(TARGETS dnetwork-secret-dialog DESTINATION /usr/lib/deepin-daemon/)

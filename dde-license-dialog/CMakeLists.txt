find_package(PkgConfig REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets)

pkg_check_modules(LICENSE_DIALOG REQUIRED dtkwidget)

set(SOURCES
    main.cpp
    content.cpp
    mainwindow.cpp
)

set(HEADERS
    content.h
    mainwindow.h
)

add_executable(dde-license-dialog ${SOURCES} ${HEADERS})

target_link_libraries(dde-license-dialog PRIVATE
    global_util
    ${Qt5Widgets_LIBRARIES}
    ${LICENSE_DIALOG_LIBRARIES}
)
target_include_directories(dde-license-dialog PRIVATE ${LICENSE_DIALOG_INCLUDE_DIRS})

install(TARGETS dde-license-dialog DESTINATION ${PREFIX}/bin)

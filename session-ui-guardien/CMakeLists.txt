find_package(Qt5 REQUIRED COMPONENTS Widgets)

add_executable(session-ui-guardien
    main.cpp
    guardien.cpp
)

target_link_libraries(session-ui-guardien PRIVATE
    ${Qt5Widgets_LIBRARIES}
)

install(TARGETS session-ui-guardien DESTINATION /usr/bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/session-ui-guardien.desktop
    DESTINATION /etc/xdg/autostart/)

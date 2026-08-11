cmake_minimum_required(VERSION 3.16)

project(dde-shutdown)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 安装前缀。
# 注意：这里刻意不使用名为 PREFIX 的变量——CMake 会把 PREFIX 视为
# find_package 的搜索前缀，定义成 /usr 会导致 find_package(Qt6) 失败。
if (NOT DEFINED SHUTDOWN_INSTALL_PREFIX)
    set(SHUTDOWN_INSTALL_PREFIX /usr)
endif()

if (NOT DEFINED ENABLE_SYSTEM_MONITOR)
    set(ENABLE_SYSTEM_MONITOR true)
endif()

# 项目根目录（支持独立构建 dde-shutdown，也支持被上级工程 add_subdirectory）
get_filename_component(SESSION_UI_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/.. ABSOLUTE)

find_package(PkgConfig REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets DBus Svg)
find_package(Dtk2widget REQUIRED)

find_package(DFrameworkdbusQt6 REQUIRED)

# Wayland 下使用 layer-shell 置顶全屏窗口（X11 仍走原窗口标志逻辑）
find_package(LayerShellQt REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS WaylandClient)

# dtk2widget(Qt6 版) 依赖 dtk6core 提供 dtkcore_global.h / DDBusSender 等
pkg_check_modules(DTK6CORE REQUIRED IMPORTED_TARGET dtk6core)

# dtk2widget 间接引用 gsettings-qt6 的符号，需显式链接
pkg_check_modules(GSETTINGSQT REQUIRED IMPORTED_TARGET gsettings-qt6)

# dde-shutdown 只用到 global_util / widgets / session-widgets 中的一小部分能力。
# session-widgets 静态库整体依赖 liblightdm-qt5 与 PAM 认证（目前均无 Qt6 版本），
# 因此这里不链接该库，而是直接编译 dde-shutdown 真正需要的源文件，
# 从而彻底避开 lightdm/PAM 这条 Qt5-only 依赖链。
set(SHUTDOWN_DEPS_SOURCES
    ${SESSION_UI_ROOT}/global_util/public_func.cpp
    ${SESSION_UI_ROOT}/global_util/util_updateui.cpp
    ${SESSION_UI_ROOT}/global_util/multiscreenmanager.cpp
    ${SESSION_UI_ROOT}/global_util/dbus/dbusvariant.cpp
    ${SESSION_UI_ROOT}/global_util/dbus/dbushotzone.cpp
    ${SESSION_UI_ROOT}/global_util/dbus/dbuslogin1manager.cpp
    ${SESSION_UI_ROOT}/global_util/dbus/dbussessionmanager.cpp
    ${SESSION_UI_ROOT}/global_util/dbus/dbuscontrolcenter.cpp
    ${SESSION_UI_ROOT}/session-widgets/sessionbasemodel.cpp
    ${SESSION_UI_ROOT}/session-widgets/authinterface.cpp
    ${SESSION_UI_ROOT}/session-widgets/userinfo.cpp
    ${SESSION_UI_ROOT}/session-widgets/framedatabind.cpp
    ${SESSION_UI_ROOT}/widgets/rounditembutton.cpp
    ${SESSION_UI_ROOT}/widgets/shutdownwidget.cpp
    ${SESSION_UI_ROOT}/widgets/fullscreenbackground.cpp
    ${SESSION_UI_ROOT}/widgets/propertygroup.cpp
)

set(SHUTDOWN_SOURCES
    main.cpp
    shutdownworker.cpp
    dbusshutdownagent.cpp
    view/contentwidget.cpp
    view/multiuserswarningview.cpp
    view/inhibitwarnview.cpp
    view/systemmonitor.cpp
    app/shutdownframe.cpp
)

set(SHUTDOWN_RESOURCES
    qss.qrc
    image.qrc
    ${SESSION_UI_ROOT}/widgets/widgetstheme.qrc
    ${SESSION_UI_ROOT}/widgets/widgetsimages.qrc
)

add_executable(dde-shutdown
    ${SHUTDOWN_SOURCES}
    ${SHUTDOWN_DEPS_SOURCES}
    ${SHUTDOWN_RESOURCES}
)

target_include_directories(dde-shutdown PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/view
    ${CMAKE_CURRENT_SOURCE_DIR}/app
    ${SESSION_UI_ROOT}/widgets
    ${SESSION_UI_ROOT}/global_util
    ${SESSION_UI_ROOT}/global_util/dbus
    ${SESSION_UI_ROOT}/session-widgets
)

target_link_libraries(dde-shutdown PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    Qt6::Svg
    Qt6::WaylandClient
    LayerShellQt::Interface
    ${Dtk2widget_LIBRARIES}
    ${DFrameworkdbusQt6_LIBRARIES}
    PkgConfig::DTK6CORE
    PkgConfig::GSETTINGSQT
)

target_include_directories(dde-shutdown PRIVATE
    ${DFrameworkdbusQt6_INCLUDE_DIR}
    ${DTK2WIDGET_INCLUDE_DIR}
)

install(TARGETS dde-shutdown DESTINATION ${SHUTDOWN_INSTALL_PREFIX}/bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/com.deepin.dde.shutdownFront.service
    DESTINATION ${SHUTDOWN_INSTALL_PREFIX}/share/dbus-1/services/)

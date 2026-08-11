cmake_minimum_required(VERSION 3.7)

if (POLICY CMP0071)
    cmake_policy(SET CMP0071 NEW)
endif()

project(dde-session-ui
    VERSION 1.0.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (NOT PREFIX)
    set(PREFIX /usr)
endif()

add_definitions(-DQT_MESSAGELOGCONTEXT)

string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" ARCH)
if (ARCH MATCHES "mips64|mips32")
    add_definitions(-DARCH_MIPSEL)
endif()

option(DISABLE_TEXT_SHADOW "Disable text shadow" NO)
option(DISABLE_ANIMATIONS "Disable animations" NO)

if (DISABLE_TEXT_SHADOW)
    add_definitions(-DDISABLE_TEXT_SHADOW)
endif()

if (DISABLE_ANIMATIONS)
    add_definitions(-DDISABLE_ANIMATIONS)
endif()

enable_testing()

add_subdirectory(global_util)
add_subdirectory(session-widgets)

# dde-shutdown 已迁移到 Qt6 + dtk2widget-qt6。
# 同一个 CMake 工程中无法同时使用 Qt5 与 Qt6（两者的 AUTOMOC/target 会互相冲突），
# 因此这里将其作为独立的外部工程构建，仍然随主工程一起 build/install。
include(ExternalProject)

# 定位 Qt6（父工程锁定了 Qt5，子工程需要独立的查找路径）
find_path(QT6_CMAKE_DIR
    NAMES Qt6Config.cmake qt6-config.cmake
    PATH_SUFFIXES cmake/Qt6
    PATHS /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE} /usr/lib /usr/lib64 /usr/local/lib
    NO_DEFAULT_PATH
)
if (NOT QT6_CMAKE_DIR)
    message(FATAL_ERROR "未找到 Qt6，dde-shutdown 需要 Qt6 与 dtk2widget-qt6")
endif()
# 由 <prefix>/lib/<arch>/cmake/Qt6 反推出安装前缀（如 /usr）。
# Qt6Config 需要借助 CMAKE_PREFIX_PATH 才能找到 Qt6GuiTools 等配套包。
get_filename_component(QT6_INSTALL_PREFIX ${QT6_CMAKE_DIR}/../../../.. ABSOLUTE)
# 只有在父工程确实设置了构建类型时才向下传递：
# 传入空的 CMAKE_BUILD_TYPE 会导致 Qt6 组件（Qt6Gui 等）的 config 加载失败。
set(SHUTDOWN_QT6_ARGS "")
if (CMAKE_BUILD_TYPE)
    list(APPEND SHUTDOWN_QT6_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
endif()

ExternalProject_Add(dde-shutdown-qt6
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/dde-shutdown
    BINARY_DIR ${CMAKE_BINARY_DIR}/dde-shutdown-qt6
    CMAKE_ARGS
        ${SHUTDOWN_QT6_ARGS}
        # 不要传 -DPREFIX：CMake 会把 PREFIX 当作 find_package 的搜索前缀，
        # 传入 /usr 反而会导致 Qt6 组件（Qt6Gui 等）查找失败，
        # 故改用不冲突的变量名传递安装前缀。
        -DSHUTDOWN_INSTALL_PREFIX:PATH=${PREFIX}
        # 父工程已锁定 Qt5，需显式告知子工程 Qt6 的位置。
        # 必须带上 :PATH 类型，否则会被存成 UNINITIALIZED，
        # 导致 Qt6 解析 Gui/Widgets 等子组件时失败。
        -DQt6_DIR:PATH=${QT6_CMAKE_DIR}
        -DCMAKE_PREFIX_PATH:PATH=${QT6_INSTALL_PREFIX}
    # 构建阶段不执行安装，安装交由父工程的 install 目标统一触发，
    # 否则普通 make 也会尝试写入 /usr 而失败。
    INSTALL_COMMAND ""
    BUILD_ALWAYS ON
)

# 让 make install 时才真正安装 Qt6 子工程（支持 DESTDIR 打包）
install(CODE "
    execute_process(
        COMMAND \${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR}/dde-shutdown-qt6
        RESULT_VARIABLE _shutdown_install_result
    )
    if (NOT _shutdown_install_result EQUAL 0)
        message(FATAL_ERROR \"安装 dde-shutdown(Qt6) 失败\")
    endif()
")

add_subdirectory(dde-license-dialog)
add_subdirectory(dde-lock)
add_subdirectory(lightdm-deepin-greeter)
add_subdirectory(dde-switchtogreeter)
add_subdirectory(dde-lowpower)
add_subdirectory(dde-osd)
add_subdirectory(dde-suspend-dialog)
add_subdirectory(dde-warning-dialog)
add_subdirectory(dde-welcome)
add_subdirectory(dde-wm-chooser)
add_subdirectory(dmemory-warning-dialog)
add_subdirectory(dnetwork-secret-dialog)
add_subdirectory(dde-full-filesystem)

if (ARCH MATCHES "mips64|mips32")
    add_subdirectory(session-ui-guardien)
endif()

find_program(TRANSLATE_DESKTOP_SH translate_desktop.sh
    PATHS ${CMAKE_SOURCE_DIR}
    NO_DEFAULT_PATH
)

find_program(TRANSLATE_GENERATION_SH translate_generation.sh
    PATHS ${CMAKE_SOURCE_DIR}
    NO_DEFAULT_PATH
)

add_custom_target(translations ALL
    COMMAND ${CMAKE_SOURCE_DIR}/translate_desktop.sh
    COMMAND ${CMAKE_SOURCE_DIR}/translate_generation.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/translations/
    DESTINATION ${PREFIX}/share/gxde-session-ui/translations/
    FILES_MATCHING PATTERN "*.qm"
)

set(ENABLE_SLEEP true)
set(ENABLE_HIBERNATE true)
set(SHOW_SWITCH_USER_BUTTON "ondemand")
set(LOGIN_PROMPT_AVATAR true)
set(LOGIN_PROMPT_INPUT false)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    set(ENABLE_SLEEP false)
    set(ENABLE_HIBERNATE false)
endif()

set(IS_DEEPIN false)
if (DEFINED DDE_DISTRIBUTION_NAME AND DDE_DISTRIBUTION_NAME STREQUAL "deepin")
    set(IS_DEEPIN true)
endif()

configure_file(
    ${CMAKE_SOURCE_DIR}/files/dde-session-ui.conf.in
    ${CMAKE_BINARY_DIR}/files/gxde-session-ui.conf
    @ONLY
)

install(FILES ${CMAKE_BINARY_DIR}/files/gxde-session-ui.conf
    DESTINATION ${PREFIX}/share/gxde-session-ui/
)

install(FILES ${CMAKE_SOURCE_DIR}/misc/applications/deepin-toggle-desktop.desktop
    DESTINATION ${PREFIX}/share/applications/
)

install(FILES
    ${CMAKE_SOURCE_DIR}/misc/icons/48x48/deepin-toggle-desktop.png
    DESTINATION ${PREFIX}/share/icons/hicolor/48x48/apps/
)
install(FILES
    ${CMAKE_SOURCE_DIR}/misc/icons/64x64/deepin-toggle-desktop.png
    DESTINATION ${PREFIX}/share/icons/hicolor/64x64/apps/
)
install(FILES
    ${CMAKE_SOURCE_DIR}/misc/icons/96x96/deepin-toggle-desktop.png
    DESTINATION ${PREFIX}/share/icons/hicolor/96x96/apps/
)
install(FILES
    ${CMAKE_SOURCE_DIR}/misc/icons/scalable/deepin-toggle-desktop.svg
    DESTINATION ${PREFIX}/share/icons/hicolor/scalable/apps/
)

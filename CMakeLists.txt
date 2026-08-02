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

add_subdirectory(dde-shutdown)
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

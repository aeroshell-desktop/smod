cmake_minimum_required(VERSION 3.16.0)

project(effect_smodglow)

option(BUILD_KF6 "Build for KDE Plasma 6" ON)

option(KWIN_BUILD_WAYLAND "Build with Wayland support" OFF)

if (BUILD_KF6)
set(QT_MAJOR_VERSION "6")
set(TARGET_NAME "smodglow")
add_compile_definitions(BUILD_KF6)
else ()
set(QT_MAJOR_VERSION "5")
set(TARGET_NAME "kwin4_effect_smodglow")
endif ()

if (QT_MAJOR_VERSION STREQUAL "5")
set(QT_MIN_VERSION "5.15.2")
set(KF5_MIN_VERSION "5.78")
else ()
set(QT_MIN_VERSION "6.5.0")
set(KF5_MIN_VERSION "5.240.0")
endif ()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)

list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

find_package(Qt${QT_MAJOR_VERSION} REQUIRED
    COMPONENTS
        Core
        DBus
)

find_package(KF${QT_MAJOR_VERSION} REQUIRED
    COMPONENTS
        ConfigWidgets
)

if (QT_MAJOR_VERSION STREQUAL "5")
find_package(KWinEffects REQUIRED
    COMPONENTS
        kwineffects
        kwinglutils
)
find_package(XCB REQUIRED XCB)
find_package(epoxy REQUIRED)
else ()

if(KWIN_BUILD_WAYLAND)
    find_package(KWin REQUIRED kwineffects)
else()
    find_package(KWinX11 REQUIRED kwineffects)
endif()

endif ()

find_package(KDecoration3 REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(KF6Config)

pkg_check_modules(smoddecoration REQUIRED IMPORTED_TARGET smoddecoration)

feature_summary(WHAT ALL)

if(KWIN_BUILD_WAYLAND)
    set(kwin4_effect_smodglow_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/smodglow.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/smodglow.qrc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/smodglow-kf6.cpp
    )
else()
    set(kwin4_effect_smodglow_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src_x11/smodglow.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src_x11/smodglow.qrc
        ${CMAKE_CURRENT_SOURCE_DIR}/src_x11/plugin.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src_x11/smodglow-kf6.cpp
    )
endif()

if(KWIN_BUILD_WAYLAND)
    add_library(${TARGET_NAME} SHARED ${kwin4_effect_smodglow_SOURCES})
else()
    add_library(${TARGET_NAME}-x11 SHARED ${kwin4_effect_smodglow_SOURCES})
endif()

if (QT_MAJOR_VERSION STREQUAL "6")

if(KWIN_BUILD_WAYLAND)
    set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "")
else()
    set_target_properties(${TARGET_NAME}-x11 PROPERTIES PREFIX "")
endif()

endif ()

if(NOT KWIN_BUILD_WAYLAND)
    set_target_properties(${TARGET_NAME}-x11 PROPERTIES OUTPUT_NAME x11/${TARGET_NAME})
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin/x11/)
endif()


if(KWIN_BUILD_WAYLAND)
    target_link_directories(${TARGET_NAME}
        PRIVATE
            ${smoddecoration_LIBRARY_DIRS})
    target_link_libraries(${TARGET_NAME}
        PRIVATE
            KDecoration3::KDecoration
            PkgConfig::smoddecoration
            KF6::ConfigCore
            KF6::ConfigGui
            Qt${QT_MAJOR_VERSION}::Core
            Qt${QT_MAJOR_VERSION}::DBus
            KF${QT_MAJOR_VERSION}::ConfigGui
            KWin::kwin
    )
else()
    target_link_directories(${TARGET_NAME}-x11
        PRIVATE
            ${smoddecoration_LIBRARY_DIRS})
    target_link_libraries(${TARGET_NAME}-x11
        PRIVATE
            KDecoration3::KDecoration
            PkgConfig::smoddecoration
            KF6::ConfigCore
            KF6::ConfigGui
            Qt${QT_MAJOR_VERSION}::Core
            Qt${QT_MAJOR_VERSION}::DBus
            KF${QT_MAJOR_VERSION}::ConfigGui
            KWinX11::kwin
    )
endif()


if (QT_MAJOR_VERSION STREQUAL "5")
target_link_libraries(${TARGET_NAME}
    PRIVATE
        KWinEffects::kwineffects
        KWinEffects::kwinglutils
)
endif ()

if(KWIN_BUILD_WAYLAND)
    install(
        TARGETS
            ${TARGET_NAME}
        DESTINATION
            ${KDE_INSTALL_PLUGINDIR}/kwin/effects/plugins
    )

else()
    install(
        TARGETS
            ${TARGET_NAME}-x11
        DESTINATION
            ${KDE_INSTALL_PLUGINDIR}/kwin-x11/effects/plugins
    )
endif()

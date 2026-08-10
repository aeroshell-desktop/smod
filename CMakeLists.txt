cmake_minimum_required(VERSION 3.16)
project(smod)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_VERSION "6.7.4")
set(PROJECT_VERSION_MAJOR 6)

set(KF6_MIN_VERSION "6.10.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")
set(QT_MIN_VERSION "6.7.0")
set(QT_MAJOR_VERSION 6)

option(BUILD_EFFECTX11 "Build the caption glow effect for X11" OFF)
option(BUILD_EFFECTWL "Build the caption glow effect for Wayland" ON)
option(BUILD_DECORATION "Build the decoration" ON)

# We need this because we can't include KDECMakeSettings here, because that
# would need KDEInstallDirs, which we can only include in the qt{5,6} builds
# to get the correct installation dirs for each Qt version.
option(BUILD_TESTING "Build the testing tree." ON)
if(BUILD_TESTING)
    enable_testing()
endif()

include(GenerateExportHeader)
include(WriteBasicConfigVersionFile)
include(FeatureSummary)
include(GNUInstallDirs)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(CMakePackageConfigHelpers)
include(ECMInstallIcons)
include(ECMGenerateHeaders)
include(ECMGeneratePkgConfigFile)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(KDEInstallDirs)
include(KDECMakeSettings)

set(QT_NO_CREATE_VERSIONLESS_TARGETS ON)
set(QT_NO_CREATE_VERSIONLESS_FUNCTIONS ON)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Quick Widgets DBus
)

if(BUILD_DECORATION)
    find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
        CoreAddons ColorScheme
        Config GuiAddons I18n
        IconThemes WindowSystem
        KCMUtils
    )
    find_package(KDecoration3 REQUIRED)
    add_subdirectory(kdecoration)
endif()

if(BUILD_EFFECTWL OR BUILD_EFFECTX11)
    find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
        ConfigWidgets
    )

    # assume decoration library is already installed
    if (NOT BUILD_DECORATION)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(smoddecoration REQUIRED IMPORTED_TARGET smoddecoration)
        add_library(smoddecoration ALIAS PkgConfig::smoddecoration)
    endif()

    if(BUILD_EFFECTX11)
        find_package(KWinX11 REQUIRED kwineffects)
    else()
        find_package(KWin REQUIRED kwineffects)
    endif()

    add_subdirectory(gloweffect)
endif()

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

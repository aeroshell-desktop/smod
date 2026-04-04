add_definitions(-DTRANSLATION_DOMAIN="smod_deco")

################# newt target #################
### plugin classes
set(smoddecoration_SRCS
    smodbutton.cpp
    smoddecoration.cpp
    smodsettingsprovider.cpp
    smodexceptionlist.cpp
    frametexture.cpp
    sizingmargins.cpp
)

### build library
kconfig_add_kcfg_files(smoddecoration_SRCS smodsettings.kcfgc)
add_library(smoddecoration SHARED
    ${smoddecoration_SRCS}
    ${smoddecoration_config_PART_FORMS_HEADERS})

set_target_properties(smoddecoration PROPERTIES
    OUTPUT_NAME aeroshell.smod
    PREFIX ""
)

target_compile_definitions(smoddecoration PRIVATE MYSHAREDLIB_LIBRARY)

set_target_properties(smoddecoration PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
set_target_properties(smoddecoration PROPERTIES ENABLE_EXPORTS TRUE)

add_compile_options(-fPIC)

target_link_libraries(smoddecoration
    PRIVATE
        Qt6::Widgets
        Qt6::DBus
        KF6::CoreAddons
        KF6::ConfigGui
        KF6::GuiAddons
        KF6::I18n
        KF6::IconThemes
        KDecoration3::KDecoration
)

install(TARGETS smoddecoration DESTINATION ${KDE_INSTALL_PLUGINDIR}/${KDECORATION_PLUGIN_DIR})

add_subdirectory(config)

set(smod_HEADERS
  smod.h
  smodbutton.h
  smoddecoration.h
  smodexceptionlist.h
  smodsettingsprovider.h
  sizingmargins.h
)

kconfig_add_kcfg_files(smod_HEADERS smodsettings.kcfgc)

ecm_generate_headers(smod_CamelCase_HEADERS
    HEADER_NAMES
        SMOD
        SMODButton
        SMODDecoration
        SMODExceptionList
        SMODSettingsProvider
        SizingMargins

    REQUIRED_HEADERS
        smod_HEADERS
)

ecm_generate_pkgconfig_file(
    BASE_NAME "smoddecoration"
    LIB_NAME ":aeroshell.smod.so"
    INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR}
    LIB_INSTALL_DIR ${KDE_INSTALL_PLUGINDIR}/org.kde.kdecoration3
    FILENAME_VAR pkgconfig_filename
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/decorations DESTINATION ${CMAKE_INSTALL_PREFIX}/share/smod)

install(
    FILES
        ${smod_HEADERS}
        ${breezedecoration_config_PART_FORMS_HEADERS}
        ${smod_CamelCase_HEADERS}

    DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}/SMOD/Decoration

    COMPONENT
        Devel
)

install(
    FILES
        ${pkgconfig_filename}

    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

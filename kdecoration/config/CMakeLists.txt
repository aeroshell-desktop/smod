set(smoddecoration_config_SRCS
    ../smodexceptionlist.cpp
    smodconfigwidget.cpp
    smoddetectwidget.cpp
    smodexceptiondialog.cpp
    smodexceptionlistwidget.cpp
    smodexceptionmodel.cpp
    smoditemmodel.cpp
)
ki18n_wrap_ui(smoddecoration_config_SRCS
   ui/smodconfigurationui.ui
   ui/smodexceptiondialog.ui
   ui/smodexceptionlistwidget.ui
)

kcoreaddons_add_plugin(kcm_smoddecoration SOURCES kcm_smoddecoration.cpp ${smoddecoration_config_SRCS} INSTALL_NAMESPACE "${KDECORATION_KCM_PLUGIN_DIR}")
kconfig_add_kcfg_files(kcm_smoddecoration ../smodsettings.kcfgc)
target_include_directories(kcm_smoddecoration PRIVATE ${CMAKE_SOURCE_DIR}/kdecoration ${CMAKE_BINARY_DIR}/kdecoration/)
target_link_libraries(kcm_smoddecoration
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::DBus
    PRIVATE
        KF6::ConfigCore
        KF6::CoreAddons
        KF6::GuiAddons
        KF6::I18n
        KF6::IconThemes
        KF6::KCMUtils
)
kcmutils_generate_desktop_file(kcm_smoddecoration)

set(effect_SRCS
    plugin.cpp

    smodglow.cpp
    smodglow-kf6.cpp

    smodglow.qrc
)

add_library(smodglow SHARED)
set_target_properties(smodglow PROPERTIES
    PREFIX ""
)
target_link_libraries(smodglow
    PRIVATE
        smoddecoration
        Qt6::Core Qt6::DBus
        KF6::ConfigCore KF6::ConfigGui
        KWin::kwin
)

install(TARGETS smodglow DESTINATION ${KDE_INSTALL_PLUGINDIR}/kwin/effects/plugins)

set(effect_SRCS
    plugin.cpp

    smodglow.cpp
    smodglow-kf6.cpp

    smodglow.qrc
)

add_library(smodglow-x11 SHARED ${effect_SRCS})
set_target_properties(smodglow-x11 PROPERTIES
    PREFIX ""
)
if (NOT BUILD_DECORATION)
    target_link_directories(smodglow-x11 PRIVATE ${smoddecoration_LIBRARY_DIRS})
endif()
target_link_libraries(smodglow-x11
    PRIVATE
        smoddecoration
        Qt6::Core Qt6::DBus
        KF6::ConfigCore KF6::ConfigGui
        KWinX11::kwin
        KDecoration3::KDecoration
)

install(TARGETS smodglow-x11 DESTINATION ${KDE_INSTALL_PLUGINDIR}/kwin-x11/effects/plugins)

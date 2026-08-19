set(effect_SRCS
    plugin.cpp

    smodglow.cpp
    smodglow-kf6.cpp

    smodglow.qrc
)

add_library(smodglow SHARED ${effect_SRCS})
set_target_properties(smodglow PROPERTIES
    PREFIX ""
)
if (NOT BUILD_DECORATION)
    target_link_directories(smodglow PRIVATE ${smoddecoration_LIBRARY_DIRS})
endif()
target_link_libraries(smodglow
    PRIVATE
        smoddecoration
        Qt6::Core Qt6::DBus
        KF6::ConfigCore KF6::ConfigGui
        KWin::kwin
        KDecoration3::KDecoration
)

install(TARGETS smodglow DESTINATION ${KDE_INSTALL_PLUGINDIR}/kwin/effects/plugins)

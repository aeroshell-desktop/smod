include_directories(${CMAKE_CURRENT_SOURCE_DIR}/kdecoration)

if(BUILD_EFFECTX11)
    add_subdirectory(x11)
else()
    add_subdirectory(wayland)
endif()


add_executable(browserselector
    browserfinder.cpp
    browserfinder.h
    browserlauncher.cpp
    browserlauncher.h
    browserselector.cpp
    browserselector.desktop
    browserselector.h
    config.cpp
    config.h
    desktopentry.cpp
    desktopentry.h
    draggablepopup.cpp
    draggablepopup.h
    main.cpp
    selectorwidget.cpp
    selectorwidget.h
    urlparser.cpp
    urlparser.h
)

target_link_libraries(browserselector PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

include(GNUInstallDirs)
install(TARGETS browserselector DESTINATION bin)

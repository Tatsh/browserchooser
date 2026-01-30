
add_executable(browserchooser
    browseroption.cpp
    browseroption.h
    browserfinder.cpp
    browserfinder.h
    browserchooser.cpp
    browserchooser.desktop
    browserchooser.h
    browserlauncher.cpp
    browserlauncher.h
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

target_link_libraries(browserchooser PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

include(GNUInstallDirs)
install(TARGETS browserchooser DESTINATION bin)

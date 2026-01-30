set(BACKEND_SOURCES backend_common.cpp)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND BACKEND_SOURCES backend_linux.cpp)
elseif(WIN32)
    list(APPEND BACKEND_SOURCES backend_win.cpp)
elseif(APPLE)
    list(APPEND BACKEND_SOURCES backend_mac.mm desktopentry_mac.mm)
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

add_executable(browserchooser
    backend.h
    ${BACKEND_SOURCES}
    browseroption.cpp
    browseroption.h
    chrome_profile.cpp
    chrome_profile.h
    firefox_profile.cpp
    firefox_profile.h
    browserchooser.cpp
    browserchooser.desktop
    browserchooser.h
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
if(WIN32)
    target_link_libraries(browserchooser PRIVATE advapi32)
elseif(APPLE)
    find_library(FOUNDATION_LIBRARY Foundation)
    target_link_libraries(browserchooser PRIVATE ${FOUNDATION_LIBRARY})
endif()

include(GNUInstallDirs)
install(TARGETS browserchooser DESTINATION bin)

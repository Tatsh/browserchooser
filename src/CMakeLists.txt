set(DISPLAY_NAME "Browser Chooser")
configure_file(browserchooser.desktop.in browserchooser.desktop @ONLY)

set(HELP_URL "https://github.com/Tatsh/browserchooser" CACHE STRING "URL for help documentation (F1)")

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

target_compile_definitions(browserchooser PRIVATE HELP_URL="${HELP_URL}")
target_include_directories(browserchooser PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
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
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/browserchooser.desktop
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
endif()

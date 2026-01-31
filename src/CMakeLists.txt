set(DISPLAY_NAME "Browser Chooser")
set(BACKEND_SOURCES backendcommon.cpp)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  list(APPEND BACKEND_SOURCES backendlinux.cpp)
elseif(WIN32)
  list(APPEND BACKEND_SOURCES backendwin.cpp)
elseif(APPLE)
  list(APPEND BACKEND_SOURCES backendmac.mm desktopentrymac.mm)
else()
  message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}.")
endif()

add_executable(
  browserchooser
  ${BACKEND_SOURCES}
  backend.h
  basedomain.cpp
  basedomain.h
  browserchooser.cpp
  browserchooser.h
  browserinfoformat.cpp
  browserinfoformat.h
  browseroption.cpp
  browseroption.h
  chromeprofile.cpp
  chromeprofile.h
  config.cpp
  config.h
  desktopentry.cpp
  domainmatch.cpp
  domainmatch.h
  desktopentry.h
  draggablepopup.cpp
  draggablepopup.h
  firefoxprofile.cpp
  firefoxprofile.h
  main.cpp
  selectorwidget.cpp
  selectorwidget.h
  urlparser.cpp
  urlparser.h)

target_compile_definitions(browserchooser PRIVATE BROWSERCHOOSER_HELP_URL="${HELP_URL}")
target_include_directories(browserchooser PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(browserchooser PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
if(WIN32)
  target_link_libraries(browserchooser PRIVATE advapi32)
elseif(APPLE)
  find_library(FOUNDATION_LIBRARY Foundation)
  target_link_libraries(browserchooser PRIVATE ${FOUNDATION_LIBRARY})
endif()

string(TIMESTAMP YEAR "%Y")
configure_file(sh.tat.${CMAKE_PROJECT_NAME}.metainfo.xml.in
               sh.tat.${CMAKE_PROJECT_NAME}.metainfo.xml @ONLY)

if(WIN32)
  configure_file("version.rc.in" version.rc @ONLY)
  target_sources(browserchooser PRIVATE version.rc)
  set_source_files_properties(version.rc PROPERTIES COMPILE_DEFINITIONS
                                                    $<$<CONFIG:Release>:NDEBUG=1>)
endif()

if(APPLE)
  # Temporary: Generate icns file from PNG icons until a proper macOS icon is made.
  set(ICONSET_DIR "${CMAKE_CURRENT_BINARY_DIR}/browserchooser.iconset")
  set(ICNS_FILE "${CMAKE_CURRENT_BINARY_DIR}/browserchooser.icns")
  file(MAKE_DIRECTORY ${ICONSET_DIR})
  add_custom_command(
    OUTPUT ${ICNS_FILE}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_16.png
            ${ICONSET_DIR}/icon_16x16.png
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_32.png
            ${ICONSET_DIR}/icon_16x16@2x.png
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_32.png
            ${ICONSET_DIR}/icon_32x32.png
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_64.png
            ${ICONSET_DIR}/icon_32x32@2x.png
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_128.png
            ${ICONSET_DIR}/icon_128x128.png
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_256.png
            ${ICONSET_DIR}/icon_128x128@2x.png
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_256.png
            ${ICONSET_DIR}/icon_256x256.png
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_512.png
            ${ICONSET_DIR}/icon_256x256@2x.png
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_512.png
            ${ICONSET_DIR}/icon_512x512.png
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_512@2x.png
            ${ICONSET_DIR}/icon_512x512@2x.png
    COMMAND iconutil -c icns -o ${ICNS_FILE} ${ICONSET_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_16.png
            ${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_32.png
            ${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_64.png
            ${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_128.png
            ${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_256.png
            ${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_512.png
            ${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_512@2x.png
    COMMENT "Generating browserchooser.icns from PNG icons."
    VERBATIM)
  add_custom_target(browserchooser_icns ALL DEPENDS ${ICNS_FILE})
  set_target_properties(
    browserchooser
    PROPERTIES OUTPUT_NAME ${DISPLAY_NAME}
               MACOSX_BUNDLE TRUE
               MACOSX_BUNDLE_BUNDLE_NAME ${DISPLAY_NAME}
               MACOSX_BUNDLE_ICON_FILE "browserchooser.icns"
               MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
               MACOSX_BUNDLE_GUI_IDENTIFIER "sh.tat.${CMAKE_PROJECT_NAME}"
               MACOSX_BUNDLE_COPYRIGHT "Copyright (c) ${YEAR} ${DISPLAY_NAME} authors."
               MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")
  add_dependencies(browserchooser browserchooser_icns)
  set(deploy_tool_options_arg "")
  set(executable_path "$<TARGET_FILE_NAME:browserchooser>.app")
  set(executable_dir ${BUNDLE_INSTALL_PREFIX})
elseif(WIN32)
  set(deploy_tool_options_arg --no-compiler-runtime)
  set(executable_path "$<TARGET_FILE_NAME:browserchooser>")
  set(executable_dir "${CMAKE_INSTALL_BINDIR}")
endif()
if(APPLE OR WIN32)
  qt_generate_deploy_script(
    TARGET
    browserchooser
    OUTPUT_SCRIPT
    deploy_script
    CONTENT
    [[
qt_deploy_runtime_dependencies(
  EXECUTABLE "${executable_dir}/${executable_path}"
  DEPLOY_TOOL_OPTIONS ${deploy_tool_options_arg}
  GENERATE_QT_CONF
  VERBOSE)
]])
endif()
install(TARGETS browserchooser RUNTIME_DEPENDENCY_SET browserchooser-deps
        BUNDLE DESTINATION ${BUNDLE_INSTALL_PREFIX})
if(APPLE OR WIN32)
  install(SCRIPT ${deploy_script})
endif()
if(APPLE)
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/browserchooser.icns
    DESTINATION "${BUNDLE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:browserchooser>.app/Contents/Resources"
  )
  if(APPLE)
    install(
      FILES ../LICENSE.txt ../README.md ../CHANGELOG.md ../SECURITY.md ../CITATION.cff
      DESTINATION
        "${BUNDLE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:browserchooser>.app/Contents/Resources")
  endif()
elseif(WIN32)
  install(FILES ../LICENSE.txt ../README.md ../CHANGELOG.md ../SECURITY.md TYPE DOC)
  install(FILES ../CITATION.cff DESTINATION ${CMAKE_INSTALL_DATADIR}/browserchooser)
  if(MINGW)
    get_filename_component(_mingw_bin_dir ${CMAKE_CXX_COMPILER} DIRECTORY)
    install(
      RUNTIME_DEPENDENCY_SET
      browserchooser-deps
      PRE_EXCLUDE_REGEXES
      [=[api-ms-]=]
      [=[ext-ms-]=]
      [[kernel32\.dll]]
      POST_EXCLUDE_REGEXES
      [=[.*system32\/.*\.dll]=]
      DIRECTORIES
      ${_mingw_bin_dir}
      $<TARGET_FILE_DIR:browserchooser>)
  endif()
else()
  # Linux, MSYS package, macOS package without creating a bundle.
  install(TARGETS browserchooser RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}.desktop.in"
                 "${CMAKE_CURRENT_BINARY_DIR}/sh.tat.${CMAKE_PROJECT_NAME}.desktop" @ONLY)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/sh.tat.${CMAKE_PROJECT_NAME}.desktop"
          DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
  foreach(
    size
    16
    32
    48
    64
    96
    128
    256
    512)
    install(
      FILES "${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_${size}.png"
      DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/${size}x${size}/apps"
      RENAME sh.tat.${CMAKE_PROJECT_NAME}.png)
  endforeach()
  install(FILES ../README.md TYPE DOC)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sh.tat.${CMAKE_PROJECT_NAME}.metainfo.xml
          DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo)
  install(FILES ${CMAKE_SOURCE_DIR}/man/browserchooser.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
endif()

# CPack configuration
if(WIN32)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/extra.nsh.in ${CMAKE_BINARY_DIR}/extra.nsh @ONLY)
  string(REPLACE "/" "\\\\" CMAKE_BINARY_DIR_ESCAPED ${CMAKE_BINARY_DIR})
  set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS
      "  !include \\\"${CMAKE_BINARY_DIR_ESCAPED}\\\\extra.nsh\\\""
      PARENT_SCOPE)
  set(CPACK_NSIS_DEFINES
      "RequestExecutionLevel user
!define /date CURRENT_YEAR \\\"%Y\\\"
VIProductVersion \\\"${PROJECT_VERSION}.0\\\"
VIAddVersionKey \\\"ProductName\\\" \\\"${DISPLAY_NAME}\\\"
VIAddVersionKey \\\"CompanyName\\\" \\\"Tatsh\\\"
VIAddVersionKey \\\"FileDescription\\\" \\\"${DISPLAY_NAME} installer.\\\"
VIAddVersionKey \\\"FileVersion\\\" \\\"${PROJECT_VERSION}\\\"
VIAddVersionKey \\\"ProductVersion\\\" \\\"${PROJECT_VERSION}.0\\\"
VIAddVersionKey \\\"LegalCopyright\\\" \\\"Copyright © \\\${CURRENT_YEAR} Tatsh.\\\"
VIAddVersionKey \\\"Comments\\\" \\\"${PROJECT_HOMEPAGE_URL}\\\""
      PARENT_SCOPE)
  set(CPACK_NSIS_INSTALL_ROOT
      "$PROGRAMFILES64"
      PARENT_SCOPE)
  set(CPACK_NSIS_CONTACT
      "https://bsky.app/profile/tatsh.bsky.social"
      PARENT_SCOPE)
  set(CPACK_NSIS_INSTALLED_ICON_NAME
      "bin\\\\${CMAKE_PROJECT_NAME}.exe"
      PARENT_SCOPE)
  set(CPACK_PACKAGE_EXECUTABLES
      "${CMAKE_PROJECT_NAME};${DISPLAY_NAME}"
      PARENT_SCOPE)
  set(CPACK_CREATE_DESKTOP_LINKS
      "${CMAKE_PROJECT_NAME};${DISPLAY_NAME}"
      PARENT_SCOPE)
  set(CPACK_NSIS_MANIFEST_DPI_AWARE
      ON
      PARENT_SCOPE)
  set(CPACK_NSIS_DISPLAY_NAME
      ${DISPLAY_NAME}
      PARENT_SCOPE)
  set(CPACK_NSIS_PACKAGE_NAME
      ${DISPLAY_NAME}
      PARENT_SCOPE)
  set(CPACK_NSIS_MODIFY_PATH
      ON
      PARENT_SCOPE)
  set(CPACK_NSIS_URL_INFO_ABOUT
      ${PROJECT_HOMEPAGE_URL}/issues
      PARENT_SCOPE)
  set(CPACK_PACKAGE_INSTALL_DIRECTORY
      ${DISPLAY_NAME}
      PARENT_SCOPE)
endif()

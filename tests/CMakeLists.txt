find_package(Qt6 REQUIRED COMPONENTS Test)

set(BROWSERCHOOSER_TEST_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/testdata")
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(BROWSERCHOOSER_BACKEND_PLATFORM_SRC ${CMAKE_SOURCE_DIR}/src/backend_linux.cpp)
elseif(WIN32)
  set(BROWSERCHOOSER_BACKEND_PLATFORM_SRC ${CMAKE_SOURCE_DIR}/src/backend_win.cpp)
elseif(APPLE)
  set(BROWSERCHOOSER_BACKEND_PLATFORM_SRC ${CMAKE_SOURCE_DIR}/src/backend_mac.mm)
else()
  set(BROWSERCHOOSER_BACKEND_PLATFORM_SRC "")
endif()
set(BROWSERCHOOSER_TEST_DATA_DIR_QUOTED "\"${BROWSERCHOOSER_TEST_DATA_DIR}\"")
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/testdata_dir.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/testdata_dir.h
  @ONLY)
set(BROWSERCHOOSER_TEST_DATA_DIR "${BROWSERCHOOSER_TEST_DATA_DIR}" PARENT_SCOPE)
set(BROWSERCHOOSER_TEST_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(BROWSERCHOOSER_SRC_DIR ${CMAKE_SOURCE_DIR}/src)

function(browserchooser_add_test test_name test_src extra_sources extra_libs)
  add_executable(${test_name} ${test_src} ${extra_sources})
  target_include_directories(${test_name} PRIVATE ${BROWSERCHOOSER_SRC_DIR}
                                            ${BROWSERCHOOSER_TEST_INCLUDE_DIR})
  target_link_libraries(${test_name} PRIVATE Qt6::Core Qt6::Test ${extra_libs})
  add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

# urlparsertest
browserchooser_add_test(urlparsertest
  ${CMAKE_CURRENT_SOURCE_DIR}/urlparsertest.cpp
  ${BROWSERCHOOSER_SRC_DIR}/urlparser.cpp
  "")

# domainmatchtest
browserchooser_add_test(domainmatchtest
  ${CMAKE_CURRENT_SOURCE_DIR}/domainmatchtest.cpp
  ${BROWSERCHOOSER_SRC_DIR}/domainmatch.cpp
  "")

# basedomaintest
browserchooser_add_test(basedomaintest
  ${CMAKE_CURRENT_SOURCE_DIR}/basedomaintest.cpp
  ${BROWSERCHOOSER_SRC_DIR}/basedomain.cpp
  "")

# browserinfoformattest
browserchooser_add_test(browserinfoformattest
  ${CMAKE_CURRENT_SOURCE_DIR}/browserinfoformattest.cpp
  ${BROWSERCHOOSER_SRC_DIR}/browserinfoformat.cpp
  "")

# desktopentrytest (needs testdata)
browserchooser_add_test(desktopentrytest
  ${CMAKE_CURRENT_SOURCE_DIR}/desktopentrytest.cpp
  ${BROWSERCHOOSER_SRC_DIR}/desktopentry.cpp
  "")
set_tests_properties(desktopentrytest PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# chrome_profiletest (needs testdata)
browserchooser_add_test(chrome_profiletest
  ${CMAKE_CURRENT_SOURCE_DIR}/chrome_profiletest.cpp
  ${BROWSERCHOOSER_SRC_DIR}/chrome_profile.cpp
  "")
set_tests_properties(chrome_profiletest PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# firefox_profiletest (needs testdata)
browserchooser_add_test(firefox_profiletest
  ${CMAKE_CURRENT_SOURCE_DIR}/firefox_profiletest.cpp
  ${BROWSERCHOOSER_SRC_DIR}/firefox_profile.cpp
  "")
set_tests_properties(firefox_profiletest PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# configtest (needs temp config path override; SavedBrowsers uses getConfigFilePath, getChromeProfileDisplayName)
browserchooser_add_test(configtest
  ${CMAKE_CURRENT_SOURCE_DIR}/configtest.cpp
  "${BROWSERCHOOSER_SRC_DIR}/config.cpp;${BROWSERCHOOSER_SRC_DIR}/domainmatch.cpp;${BROWSERCHOOSER_SRC_DIR}/backend_common.cpp;${BROWSERCHOOSER_BACKEND_PLATFORM_SRC};${BROWSERCHOOSER_SRC_DIR}/browseroption.cpp;${BROWSERCHOOSER_SRC_DIR}/desktopentry.cpp;${BROWSERCHOOSER_SRC_DIR}/chrome_profile.cpp;${BROWSERCHOOSER_SRC_DIR}/firefox_profile.cpp"
  "")
set_tests_properties(configtest PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# backend_commontest (listContainsIdentifier, isBrowserHidden, readCommaSeparatedList with temp config)
browserchooser_add_test(backend_commontest
  ${CMAKE_CURRENT_SOURCE_DIR}/backend_commontest.cpp
  "${BROWSERCHOOSER_SRC_DIR}/backend_common.cpp;${BROWSERCHOOSER_BACKEND_PLATFORM_SRC};${BROWSERCHOOSER_SRC_DIR}/chrome_profile.cpp;${BROWSERCHOOSER_SRC_DIR}/firefox_profile.cpp;${BROWSERCHOOSER_SRC_DIR}/browseroption.cpp;${BROWSERCHOOSER_SRC_DIR}/desktopentry.cpp"
  "")
set_tests_properties(backend_commontest PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

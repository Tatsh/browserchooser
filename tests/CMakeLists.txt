find_package(Qt6 REQUIRED COMPONENTS Test)

set(BROWSERCHOOSER_TEST_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/testdata")
set(BROWSERCHOOSER_BACKEND_PLATFORM_SRC ${CMAKE_SOURCE_DIR}/src/backendlinux.cpp)
set(BROWSERCHOOSER_TEST_DATA_DIR_QUOTED "\"${BROWSERCHOOSER_TEST_DATA_DIR}\"")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testdatadir.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/testdatadir.h @ONLY)
set(BROWSERCHOOSER_TEST_DATA_DIR
    "${BROWSERCHOOSER_TEST_DATA_DIR}"
    PARENT_SCOPE)
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
browserchooser_add_test(urlparsertest ${CMAKE_CURRENT_SOURCE_DIR}/urlparsertest.cpp
                        ${BROWSERCHOOSER_SRC_DIR}/urlparser.cpp "")

# domainmatchtest
browserchooser_add_test(domainmatchtest ${CMAKE_CURRENT_SOURCE_DIR}/domainmatchtest.cpp
                        ${BROWSERCHOOSER_SRC_DIR}/domainmatch.cpp "")

# basedomaintest
browserchooser_add_test(basedomaintest ${CMAKE_CURRENT_SOURCE_DIR}/basedomaintest.cpp
                        ${BROWSERCHOOSER_SRC_DIR}/basedomain.cpp "")

# browserinfoformattest
browserchooser_add_test(browserinfoformattest ${CMAKE_CURRENT_SOURCE_DIR}/browserinfoformattest.cpp
                        ${BROWSERCHOOSER_SRC_DIR}/browserinfoformat.cpp "")

# desktopentrytest (needs testdata)
browserchooser_add_test(desktopentrytest ${CMAKE_CURRENT_SOURCE_DIR}/desktopentrytest.cpp
                        ${BROWSERCHOOSER_SRC_DIR}/desktopentry.cpp "")
set_tests_properties(desktopentrytest PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# chromeprofiletest (needs testdata)
browserchooser_add_test(chromeprofiletest ${CMAKE_CURRENT_SOURCE_DIR}/chromeprofiletest.cpp
                        ${BROWSERCHOOSER_SRC_DIR}/chromeprofile.cpp "")
set_tests_properties(chromeprofiletest PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# firefoxprofiletest (needs testdata)
browserchooser_add_test(firefoxprofiletest ${CMAKE_CURRENT_SOURCE_DIR}/firefoxprofiletest.cpp
                        ${BROWSERCHOOSER_SRC_DIR}/firefoxprofile.cpp "")
set_tests_properties(firefoxprofiletest PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# configtest (needs temp config path override; SavedBrowsers uses getConfigFilePath,
# getChromeProfileDisplayName)
set(CONFIGTEST_SOURCES
    ${BROWSERCHOOSER_SRC_DIR}/config.cpp
    ${BROWSERCHOOSER_SRC_DIR}/domainmatch.cpp
    ${BROWSERCHOOSER_SRC_DIR}/backendcommon.cpp
    ${BROWSERCHOOSER_BACKEND_PLATFORM_SRC}
    ${BROWSERCHOOSER_SRC_DIR}/browseroption.cpp
    ${BROWSERCHOOSER_SRC_DIR}/desktopentry.cpp
    ${BROWSERCHOOSER_SRC_DIR}/chromeprofile.cpp
    ${BROWSERCHOOSER_SRC_DIR}/firefoxprofile.cpp)
browserchooser_add_test(configtest ${CMAKE_CURRENT_SOURCE_DIR}/configtest.cpp
                        "${CONFIGTEST_SOURCES}"
                        "")
set_tests_properties(configtest PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# backendcommontest (listContainsIdentifier, isBrowserHidden, readCommaSeparatedList with temp
# config)
set(BACKENDCOMMONTEST_SOURCES
    ${BROWSERCHOOSER_SRC_DIR}/backendcommon.cpp
    ${BROWSERCHOOSER_BACKEND_PLATFORM_SRC}
    ${BROWSERCHOOSER_SRC_DIR}/chromeprofile.cpp
    ${BROWSERCHOOSER_SRC_DIR}/firefoxprofile.cpp
    ${BROWSERCHOOSER_SRC_DIR}/browseroption.cpp
    ${BROWSERCHOOSER_SRC_DIR}/desktopentry.cpp
    )
browserchooser_add_test(backendcommontest ${CMAKE_CURRENT_SOURCE_DIR}/backendcommontest.cpp
                        "${BACKENDCOMMONTEST_SOURCES}"
                        "")
set_tests_properties(backendcommontest PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

cmake_minimum_required(VERSION 3.16)
project(
  browserchooser
  VERSION 0.0.1
  LANGUAGES CXX
  DESCRIPTION "A simple browser chooser."
  HOMEPAGE_URL "https://github.com/Tatsh/browserchooser")

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(BUILD_DOCS "Build documentation." OFF)
option(BUILD_DOCS_ONLY "Build documentation only." OFF)

if(BUILD_DOCS OR BUILD_DOCS_ONLY)
  set(DOXYGEN_GENERATE_TAGFILE "${CMAKE_BINARY_DIR}/html/browserchooser.tags")
  set(DOXYGEN_INTERACTIVE_SVG YES)
  set(DOXYGEN_MARKDOWN_ID_STYLE GITHUB)
  set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md")
  set(DOXYGEN_TAGFILES
      ./.docs/cppreference-doxygen-web.tag.xml=https://en.cppreference.com/w/
      ./.docs/qtcore.tags=http://doc.qt.io/qt-6/ ./.docs/qtgui.tags=http://doc.qt.io/qt-6/
      ./.docs/qtwidgets.tags=http://doc.qt.io/qt-6/)
  find_package(Doxygen REQUIRED COMPONENTS dot)
  doxygen_add_docs(docs README.md src ALL)
endif()

if(BUILD_DOCS_ONLY)
  return()
endif()

option(BROWSERCHOOSER_CXX_WARN_ALL "Do not use." OFF)
mark_as_advanced(BROWSERCHOOSER_CXX_WARN_ALL)
set(BUNDLE_INSTALL_PREFIX
    .
    CACHE FILEPATH "Install the bundle to a custom directory. This can be an absolute path or \
relative to CMAKE_INSTALL_PREFIX.")
set(HELP_URL
    "https://github.com/Tatsh/browserchooser"
    CACHE STRING "URL for help documentation (F1).")

find_package(ECM NO_MODULE REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(ECMQtDeclareLoggingCategory)

find_package(Qt6Core 6.7 CONFIG REQUIRED)
find_package(Qt6Gui 6.7 CONFIG REQUIRED)
find_package(Qt6Widgets 6.7 CONFIG REQUIRED)

qt_standard_project_setup()
add_compile_definitions(
  QT_NO_CAST_FROM_ASCII
  QT_NO_CAST_FROM_BYTEARRAY
  QT_NO_CAST_TO_ASCII
  QT_NO_SIGNALS_SLOTS_KEYWORDS
  QT_NO_URL_CAST_FROM_STRING
  QT_STRICT_ITERATORS
  QT_USE_FAST_OPERATOR_PLUS
  QT_USE_QSTRINGBUILDER
  $<$<BOOL:${NO_GUI}>:NO_GUI>)
add_compile_options($<$<AND:$<CXX_COMPILER_ID:Clang,GNU>,$<BOOL:${COVERAGE}>>:--coverage>)
add_link_options($<$<AND:$<CXX_COMPILER_ID:Clang,GNU>,$<BOOL:${COVERAGE}>>:--coverage>)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND BROWSERCHOOSER_CXX_WARN_ALL)
    add_compile_options(
      -fsafe-buffer-usage-suggestions
      -Weverything
      -Wno-c++98-compat
      -Wno-c++98-compat-pedantic
      -Wno-covered-switch-default # Conflicts with -Wswitch-default.
      -Wno-ctad-maybe-unsupported # Qt's moc uses CTAD without deduction guides.
      -Wno-deprecated-copy-with-user-provided-dtor
      -Wno-exit-time-destructors
      -Wno-global-constructors
      -Wno-inconsistent-missing-destructor-override
      -Wno-nrvo # Conflicts with -Wpessimizing-move in some cases.
      -Wno-padded
      -Wno-redundant-parens
      -Wno-unsafe-buffer-usage # Qt moc uses strcmp.
      -Wno-unsafe-buffer-usage-in-libc-call) # Qt moc uses strcmp.
  endif()
endif()

add_subdirectory(src)

# Testing
option(BUILD_TESTS "Build tests." OFF)
option(COVERAGE "Enable code coverage (GCC/Clang only)." OFF)
if(COVERAGE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(--coverage)
    add_link_options(--coverage)
  else()
    message(WARNING "COVERAGE is only supported with GCC or Clang.")
  endif()
endif()
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

set(CPACK_PACKAGE_VENDOR "Tatsh")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE.txt)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(__cpack_system_name ${CMAKE_SYSTEM_NAME})
if(__cpack_system_name MATCHES "Windows")
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(__cpack_system_name win64)
  else()
    set(__cpack_system_name win32)
  endif()
elseif(APPLE)
  if(CMAKE_APPLE_SILICON_PROCESSOR)
    set(__cpack_system_name Darwin-arm64)
  else()
    set(__cpack_system_name Darwin-x86_64)
  endif()
endif()
set(CPACK_THREADS 0)
set(CPACK_PACKAGE_FILE_NAME ${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}-${__cpack_system_name})

if(WIN32)
  set(CPACK_GENERATOR NSIS ZIP)
elseif(APPLE)
    set(CPACK_GENERATOR DragNDrop ZIP)
  endif()
else()
  set(CPACK_GENERATOR ZIP)
endif()
include(CPack)

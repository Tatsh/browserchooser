cmake_minimum_required(VERSION 3.16)
project(browserselector VERSION 0.0.1 LANGUAGES CXX DESCRIPTION "A simple browser selector." HOMEPAGE_URL "https://github.com/Tatsh/browserselector")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(BROWSERSELECTOR_CXX_WARN_ALL "Do not use." OFF)
mark_as_advanced(BROWSERSELECTOR_CXX_WARN_ALL)

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
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND BROWSERSELECTOR_CXX_WARN_ALL)
    add_compile_options(
      -fsafe-buffer-usage-suggestions
      -Weverything
      -Wno-c++98-compat
      -Wno-c++98-compat-pedantic
      -Wno-ctad-maybe-unsupported
      -Wno-exit-time-destructors
      -Wno-global-constructors
      -Wno-padded)
  endif()
endif()

add_subdirectory(src)

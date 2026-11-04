cmake_minimum_required(VERSION 3.16)

project(QCustomPlot VERSION 2.1.1 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(QCUSTOMPLOT_TOP_LEVEL ON)
else()
  set(QCUSTOMPLOT_TOP_LEVEL OFF)
endif()

option(QCUSTOMPLOT_BUILD_EXAMPLES "Build the QCustomPlot example applications" ${QCUSTOMPLOT_TOP_LEVEL})
option(QCUSTOMPLOT_USE_OPENGL "Enable OpenGL hardware acceleration (defines QCUSTOMPLOT_USE_OPENGL)" OFF)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets PrintSupport)

add_library(QCustomPlot qcustomplot.cpp qcustomplot.h)
add_library(QCustomPlot::QCustomPlot ALIAS QCustomPlot)

target_compile_features(QCustomPlot PUBLIC cxx_std_11)

target_link_libraries(QCustomPlot
  PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::PrintSupport
)

target_include_directories(QCustomPlot
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/QCustomPlot>
)

set_target_properties(QCustomPlot PROPERTIES
  AUTOMOC ON
  DEBUG_POSTFIX d
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
  EXPORT_NAME QCustomPlot
)

if(QCUSTOMPLOT_USE_OPENGL)
  find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS OpenGL)
  target_compile_definitions(QCustomPlot PUBLIC QCUSTOMPLOT_USE_OPENGL)
  target_link_libraries(QCustomPlot PUBLIC Qt${QT_VERSION_MAJOR}::OpenGL)
endif()

get_target_property(QCUSTOMPLOT_LIBRARY_TYPE QCustomPlot TYPE)
if(QCUSTOMPLOT_LIBRARY_TYPE STREQUAL "SHARED_LIBRARY")
  target_compile_definitions(QCustomPlot
    PRIVATE QCUSTOMPLOT_COMPILE_LIBRARY
    INTERFACE QCUSTOMPLOT_USE_LIBRARY
  )
endif()

if(QCUSTOMPLOT_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

install(TARGETS QCustomPlot
  EXPORT QCustomPlotTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES qcustomplot.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/QCustomPlot)

install(EXPORT QCustomPlotTargets
  FILE QCustomPlotTargets.cmake
  NAMESPACE QCustomPlot::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QCustomPlot
)

configure_package_config_file(
  cmake/QCustomPlotConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/QCustomPlotConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QCustomPlot
)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/QCustomPlotConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/QCustomPlotConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/QCustomPlotConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QCustomPlot
)

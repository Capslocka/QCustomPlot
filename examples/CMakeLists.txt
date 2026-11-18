set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# The .ui files promote QCustomPlot with the plain header "QCustomPlot.h";
# the QCustomPlot target exports the src directory, so the generated
# ui_*.h files resolve it through the target's include directories.

add_executable(axis-tags-example
  axis-tags/main.cpp
  axis-tags/mainwindow.cpp
  axis-tags/mainwindow.h
  axis-tags/mainwindow.ui
  axis-tags/axistag.cpp
  axis-tags/axistag.h
)
target_link_libraries(axis-tags-example PRIVATE QCustomPlot::QCustomPlot)

add_executable(interaction-example
  interactions/main.cpp
  interactions/mainwindow.cpp
  interactions/mainwindow.h
  interactions/mainwindow.ui
)
target_link_libraries(interaction-example PRIVATE QCustomPlot::QCustomPlot)

add_executable(plot-examples
  plots/main.cpp
  plots/mainwindow.cpp
  plots/mainwindow.h
  plots/mainwindow.ui
)
target_link_libraries(plot-examples PRIVATE QCustomPlot::QCustomPlot)

# The plots example loads its images relative to the working directory,
# so keep them next to the executable.
add_custom_command(TARGET plot-examples POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/plots/balboa.jpg
    ${CMAKE_CURRENT_SOURCE_DIR}/plots/solarpanels.jpg
    ${CMAKE_CURRENT_SOURCE_DIR}/plots/sun.png
    $<TARGET_FILE_DIR:plot-examples>
  VERBATIM
)

add_executable(scrollbar-axis-range-control
  scrollbar-axis-range-control/main.cpp
  scrollbar-axis-range-control/mainwindow.cpp
  scrollbar-axis-range-control/mainwindow.h
  scrollbar-axis-range-control/mainwindow.ui
)
target_link_libraries(scrollbar-axis-range-control PRIVATE QCustomPlot::QCustomPlot)

add_executable(text-document-integration
  text-document-integration/main.cpp
  text-document-integration/mainwindow.cpp
  text-document-integration/mainwindow.h
  text-document-integration/mainwindow.ui
  text-document-integration/qcpdocumentobject.cpp
  text-document-integration/qcpdocumentobject.h
)
target_link_libraries(text-document-integration PRIVATE QCustomPlot::QCustomPlot)

add_executable(sync-demo-example
  sync-demo/main.cpp
  sync-demo/mainwindow.cpp
  sync-demo/mainwindow.h
)
target_link_libraries(sync-demo-example PRIVATE QCustomPlot::QCustomPlot)

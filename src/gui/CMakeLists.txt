#####################
# QT Widget Support #
#####################

# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)
# Find the QtWidgets library

################
# Applications #
################

#include headers
include_directories("${PROJECT_BINARY_DIR}")

add_subdirectory(model_dummy)
add_subdirectory(adapters_qt)

qt5_add_resources(IMAGES ${PROJECT_SOURCE_DIR}/gfx/images.qrc)  #Dir for ResourceFile with Images
set_source_files_properties(${IMAGES} PROPERTIES GENERATED ON)  #Use generated Images for App (Compiled)

# lte_gui
set(gui
    mainwindow.ui
    mainwindow.cpp
    main.cpp
    spectrum.cpp
    file_input_output.cpp
    settings.cpp   
    plots.cpp
    setting_slots.cpp
    dynamic_options.cpp
    ${IMAGES}
  )

include_directories("gui")

add_executable(gui ${gui})

target_link_libraries(gui  
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  Qt5::Charts
  ${OPENGL_LIBRARIES}
  ${Boost_LIBRARIES}
  model_dummy
  adapters_qt
  falcon_common
  falcon_meas
  qcustomplot
  rangewidget
  ${SRSLTE_LIBRARIES}
  ${FFT_LIBRARIES}  
  )

  INSTALL(  TARGETS gui
            DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")

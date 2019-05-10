file(GLOB SOURCES "*.cpp")

# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)
# Find the QtWidgets library

add_library(qcustomplot STATIC ${SOURCES})
target_link_libraries(qcustomplot Qt5::Core)



file(GLOB SOURCES "*.cc")
include_directories(${Qt5Widgets_INCLUDE_DIRS})
add_library(adapters_qt STATIC ${SOURCES})
target_compile_options(adapters_qt PUBLIC "-std=gnu++11")

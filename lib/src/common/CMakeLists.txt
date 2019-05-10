 
file(GLOB SOURCES "*.cc" "*.c")
add_library(falcon_common STATIC ${SOURCES})
target_compile_options(falcon_common PUBLIC "-std=c++11")
#target_link_libraries(falcon_common)

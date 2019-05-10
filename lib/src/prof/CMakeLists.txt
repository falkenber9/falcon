 
file(GLOB SOURCES "*.cc" "*.c")
add_library(falcon_prof STATIC ${SOURCES})
target_compile_options(falcon_prof PUBLIC "-std=c++11")

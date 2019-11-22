 
file(GLOB SOURCES "*.cc" "*.c")
add_library(falcon_prof STATIC ${SOURCES})
target_compile_options(falcon_prof PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-std=c++11>)

 
file(GLOB SOURCES "*.cc")
add_library(falcon_util STATIC ${SOURCES})
target_compile_options(falcon_util PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-std=c++11>)

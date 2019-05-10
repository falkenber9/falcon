 
file(GLOB SOURCES "*.cc")
add_library(falcon_util STATIC ${SOURCES})
target_compile_options(falcon_util PUBLIC "-std=c++11")

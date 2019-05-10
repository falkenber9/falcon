 
file(GLOB SOURCES "*.cc" "*.c")
add_library(falcon_meas STATIC ${SOURCES})
target_compile_options(falcon_meas PUBLIC "-std=c++11")

target_link_libraries(falcon_meas
  falcon_common
  falcon_prof
  pthread
#  srslte_phy
  ${CMNALIB_LIBRARIES}
  ${SRSLTE_LIBRARIES}
)

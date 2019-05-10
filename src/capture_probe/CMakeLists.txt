
file(GLOB SOURCES "*.cc" "*.c")

add_library(falcon_capture_probe STATIC ${SOURCES})
target_link_libraries(falcon_capture_probe
#  srslte_phy
#  srslte_common
#  srslte_rf
#  srslte_rf_utils
  falcon_phy
  falcon_util
  falcon_meas
  falcon_common
  ${CMNALIB_LIBRARIES}
  ${SRSLTE_LIBRARIES}
  ${FFT_LIBRARIES}
)
target_compile_options(falcon_capture_probe PUBLIC "-std=c++11")


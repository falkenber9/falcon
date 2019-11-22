
add_subdirectory(phy)

file(GLOB SOURCES "*.cc" "*.c")

add_library(falcon_eye STATIC ${SOURCES})
target_link_libraries(falcon_eye
#  srslte_rf
#  srslte_phy
#  srslte_common
  eye_phy
  falcon_phy
  falcon_util
  falcon_meas
  falcon_common
  falcon_prof
  ${SRSLTE_LIBRARIES}
  ${FFT_LIBRARIES}
)
target_compile_options(falcon_eye PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-std=c++11>)


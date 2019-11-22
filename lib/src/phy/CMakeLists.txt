
add_subdirectory(falcon_rf)
add_subdirectory(falcon_ue)
add_subdirectory(falcon_phch)

set(falcon_srcs
  $<TARGET_OBJECTS:falcon_rf>
  $<TARGET_OBJECTS:falcon_ue>
  $<TARGET_OBJECTS:falcon_phch>
)

add_library(falcon_phy STATIC ${falcon_srcs})
target_link_libraries(falcon_phy
#  srslte_phy
  ${SRSLTE_LIBRARIES}
  ${FFT_LIBRARIES}
  pthread
  ${Boost_LIBRARIES}
#  model_dummy
  )

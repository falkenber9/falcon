
file(GLOB SOURCES "*.cc")

#add_library(falcon_lte ${SOURCES})
add_library(falcon_lte STATIC ${SOURCES})
target_link_libraries(falcon_lte
  srsue_mac
  srsue_phy
  srsue_upper
  srslte_common
  srslte_phy
  srslte_upper
  srslte_radio
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
)
#target_compile_options(falcon_lte PUBLIC "-std=gnu++11")


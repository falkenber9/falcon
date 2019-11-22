
file(GLOB SOURCES "*.cc" "*.c")
add_library(model_dummy STATIC ${SOURCES})

#if(SRSLTE_LIBRARY_RADIO AND SRSLTE_LIBRARY_RF AND SRSLTE_LIBRARY_RF_UTILS)
#
target_link_libraries(model_dummy pthread eye_phy falcon_phy falcon_util falcon_common ${Boost_LIBRARIES} ${SRSLTE_LIBRARIES} ${FFT_LIBRARIES})
#
#else()
#
#target_link_libraries(model_dummy pthread falcon_phy falcon_util ${Boost_LIBRARIES} srslte_phy srslte_upper srslte_common ${FFT_LIBRARIES})
#add_definitions(-DDISABLE_RF)
#
#endif(SRSLTE_LIBRARY_RADIO AND SRSLTE_LIBRARY_RF AND SRSLTE_LIBRARY_RF_UTILS)



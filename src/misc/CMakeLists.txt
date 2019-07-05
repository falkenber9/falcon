

add_executable(SearchSpace SearchSpace.cc)

target_link_libraries(SearchSpace  
  falcon_phy
  ${SRSLTE_LIBRARIES}
  )

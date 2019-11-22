
# Link libstdc++ and libgcc
if(BUILD_STATIC)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
endif(BUILD_STATIC)

find_package(SRSGUI)
if(SRSGUI_FOUND)
  add_definitions(-DENABLE_GUI)
  include_directories(${SRSGUI_INCLUDE_DIRS})
  link_directories(${SRSGUI_LIBRARY_DIRS})
else(SRSGUI_FOUND)
  add_definitions(-DDISABLE_GRAPHICS)
  set(SRSGUI_LIBRARIES "")
endif(SRSGUI_FOUND)

if(RF_FOUND)
  add_executable(imdea_capture_sync imdea_capture_sync.c)
  target_link_libraries(imdea_capture_sync
    ${SRSLTE_LIBRARIES}
    ${FFT_LIBRARIES}
    falcon_phy
    falcon_util
  )
else(RF_FOUND)
  add_definitions(-DDISABLE_RF)
  #unset(SRSGUI_LIBRARIES)
endif(RF_FOUND)

message("### STATUS GUI:")
message(SRSGUI_FOUND)

if(SRSGUI_FOUND)
    add_executable(imdea_cc_decoder imdea_cc_decoder.c)
    target_link_libraries(imdea_cc_decoder
      eye_phy
      falcon_phy
      falcon_util
      ${SRSLTE_LIBRARIES}
      ${SRSGUI_LIBRARIES}
      ${FFT_LIBRARIES}
    )

    add_executable(cni_cc_decoder cni_cc_decoder.c)
    target_link_libraries(cni_cc_decoder
      eye_phy
      falcon_phy
      falcon_util
      falcon_common
      ${SRSLTE_LIBRARIES}
      ${SRSGUI_LIBRARIES}
      ${FFT_LIBRARIES}
    )
else(SRSGUI_FOUND)
    message(WARNING "imdea_cc_decoder and cni_cc_decoder won't be built sind SRSGUI was not found.")
endif(SRSGUI_FOUND)

#################################################################
# Compile options (compatibility)
#################################################################

if(RF_FOUND)
    target_compile_options(imdea_capture_sync PUBLIC "-std=gnu99")
endif(RF_FOUND)
if(SRSGUI_FOUND)
    target_compile_options(imdea_cc_decoder PUBLIC "-std=gnu99")
    target_compile_options(cni_cc_decoder PUBLIC "-std=gnu99")
endif(SRSGUI_FOUND)



add_subdirectory(capture_probe)
add_subdirectory(eye)
add_subdirectory(gui)
add_subdirectory(misc)

# Link libstdc++ and libgcc
#if(BUILD_STATIC)
#  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
#endif(BUILD_STATIC)

set(INSTALL_BUNDLE)

add_executable(FalconCaptureWarden FalconCaptureWarden.cc)
target_link_libraries(FalconCaptureWarden falcon_meas)
target_compile_options(FalconCaptureWarden PUBLIC "-std=c++11")
list(APPEND INSTALL_BUNDLE FalconCaptureWarden)

if(RF_FOUND)
    add_executable(FalconCaptureProbe FalconCaptureProbe.cc)
    target_link_libraries(FalconCaptureProbe falcon_capture_probe)
    target_compile_options(FalconCaptureProbe PUBLIC "-std=c++11")
    list(APPEND INSTALL_BUNDLE FalconCaptureProbe)
endif(RF_FOUND)

add_executable(FalconEye FalconEye.cc)
target_link_libraries(FalconEye falcon_eye)
target_compile_options(FalconEye PUBLIC "-std=c++11")
list(APPEND INSTALL_BUNDLE FalconEye)

# nested install targets are not supported by old Cmake on Ubuntu 18
#list (APPEND INSTALL_BUNDLE
#    falcon_capture_probe
#    falcon_eye)
install(
    TARGETS ${INSTALL_BUNDLE}
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})
message(STATUS "Install Bundle: ${INSTALL_BUNDLE}")

#Copy wrapper scripts into build dir

file(GLOB SCRIPTS "*.sh" "*.py")
message(STATUS "Copy scripts: ${SCRIPTS}")
foreach(SCRIPTFILE ${SCRIPTS})
  get_filename_component(FILENAME ${SCRIPTFILE} NAME)
  message(STATUS "Copy file: ${FILENAME}")
  configure_file("${FILENAME}" "${FILENAME}" COPYONLY)
endforeach(SCRIPTFILE)

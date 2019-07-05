
add_subdirectory(capture_probe)
add_subdirectory(eye)
add_subdirectory(gui)
add_subdirectory(misc)
#...

# Link libstdc++ and libgcc
#if(BUILD_STATIC)
#  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
#endif(BUILD_STATIC)

add_executable(FalconCaptureWarden FalconCaptureWarden.cc)
target_link_libraries(FalconCaptureWarden falcon_meas)
target_compile_options(FalconCaptureWarden PUBLIC "-std=c++11")

if(RF_FOUND)
    add_executable(FalconCaptureProbe FalconCaptureProbe.cc)
    target_link_libraries(FalconCaptureProbe falcon_capture_probe)
    target_compile_options(FalconCaptureProbe PUBLIC "-std=c++11")
endif(RF_FOUND)

add_executable(FalconEye FalconEye.cc)
target_link_libraries(FalconEye falcon_eye)
target_compile_options(FalconEye PUBLIC "-std=c++11")

#Copy the scripts

file(GLOB SCRIPTS "*.sh" "*.py")
message(STATUS "### Processing all: ${SCRIPTS}")
foreach(SCRIPTFILE ${SCRIPTS})
  get_filename_component(FILENAME ${SCRIPTFILE} NAME)
  message(STATUS "Copy file: ${FILENAME}")
  configure_file("${FILENAME}" "${FILENAME}" COPYONLY)
endforeach(SCRIPTFILE)
  #configure_file("Loop-FalconCaptureProbe.sh" "Loop-FalconCaptureProbe.sh" COPYONLY)
  #configure_file("PullCaptureFiles.sh" "PullCaptureFiles.sh" COPYONLY)


add_subdirectory(src)
add_subdirectory(test)

########################################################################
# Install library headers
########################################################################
INSTALL(  DIRECTORY include/
          DESTINATION "${INCLUDE_DIR}"
          FILES_MATCHING PATTERN "*.h" )

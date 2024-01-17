get_target_name(TARGET)
add_library(${TARGET} SHARED
  src/file.cpp
  src/resource.cpp
  src/threadPool.cpp
)

target_include_directories(${TARGET} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  $<INSTALL_INTERFACE:${SDK_INSTALL_INCLUDE_DIR}>
)
target_link_libraries(${TARGET} PUBLIC pthread)


#---------------------------------------------------------------------------------
install(
  DIRECTORY   include
  DESTINATION ${CMAKE_INSTALL_PREFIX}
  FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "*.inl"
)

install(
  TARGETS ${TARGET}
  EXPORT ${TARGET}Config
  DESTINATION ${INSTALL_LIBRARY_DIR}
)

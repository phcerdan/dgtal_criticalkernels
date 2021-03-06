set(SG_MODULE_NAME "ITKBridge")
set(SG_MODULE_${SG_MODULE_NAME}_LIBRARY "SG${SG_MODULE_NAME}")
set(SG_LIBRARIES ${SG_LIBRARIES} ${SG_MODULE_${SG_MODULE_NAME}_LIBRARY} PARENT_SCOPE)

set(enabled_internal_libs_ SGCore)
set(enabled_include_dirs_)
set(enabled_external_libs_
  ${ITK_LIBRARIES}
  )
set(enabled_include_system_dirs_)
set(enabled_compile_definitions_)

set(SG_MODULE_INTERNAL_DEPENDS ${enabled_internal_libs_})
set(SG_MODULE_${SG_MODULE_NAME}_DEPENDS
  ${SG_MODULE_INTERNAL_DEPENDS}
  ${enabled_external_libs_}
  )

set(SG_MODULE_${SG_MODULE_NAME}_SOURCES
  fill_holes_function.cpp
  resample_image_function.cpp
  voxelize_graph.cpp
  morphological_watershed.cpp
  create_vertex_to_radius_map.cpp
  segmentation_functions.cpp
  )

list(TRANSFORM SG_MODULE_${SG_MODULE_NAME}_SOURCES PREPEND "src/")

add_library(${SG_MODULE_${SG_MODULE_NAME}_LIBRARY} ${SG_MODULE_${SG_MODULE_NAME}_SOURCES})
add_library(SGEXT::${SG_MODULE_${SG_MODULE_NAME}_LIBRARY} ALIAS ${SG_MODULE_${SG_MODULE_NAME}_LIBRARY})
target_include_directories(${SG_MODULE_${SG_MODULE_NAME}_LIBRARY} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )

target_link_libraries(${SG_MODULE_${SG_MODULE_NAME}_LIBRARY}
  ${SG_MODULE_${SG_MODULE_NAME}_DEPENDS}
  )
add_dependencies(${SG_MODULE_${SG_MODULE_NAME}_LIBRARY} ${SG_MODULE_INTERNAL_DEPENDS})

if(SG_BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS ${SG_MODULE_${SG_MODULE_NAME}_LIBRARY}
        EXPORT SGEXTTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT development
        )

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sgext
  PATTERN "*.txt" EXCLUDE)

# export to the build tree
export( TARGETS ${SG_MODULE_${SG_MODULE_NAME}_LIBRARY}
  NAMESPACE SGEXT::
  APPEND FILE ${sgext_export_file})

find_package(Poco COMPONENTS ${Poco_FIND_COMPONENTS} CONFIG QUIET)
if(Poco_FOUND)
  return()
endif()

find_path(Poco_INCLUDE_DIR Poco/Poco.h)
mark_as_advanced(FORCE Poco_INCLUDE_DIR)

foreach(component ${Poco_FIND_COMPONENTS})
  set(component_var "Poco_${component}_LIBRARY")
  find_library(${component_var} Poco${component})
  mark_as_advanced(FORCE ${component_var})
  if(${component_var})
    set(Poco_${component}_FOUND TRUE)
    list(APPEND Poco_LIBRARIES ${component})
    if(NOT TARGET Poco::${component})
      add_library(Poco::${component} SHARED IMPORTED)
      set_target_properties(Poco::${component} PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${Poco_INCLUDE_DIR}
        IMPORTED_LOCATION ${${component_var}}
      )
    endif()
  endif()
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Poco
  FOUND_VAR Poco_FOUND
  REQUIRED_VARS Poco_INCLUDE_DIR Poco_LIBRARIES
  VERSION_VAR Poco_VERSION
  HANDLE_COMPONENTS
)

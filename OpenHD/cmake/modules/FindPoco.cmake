set(_poco_hint_roots)
if(Poco_ROOT)
  list(APPEND _poco_hint_roots ${Poco_ROOT})
endif()
if(DEFINED ENV{POCO_STATIC_ROOT})
  list(APPEND _poco_hint_roots $ENV{POCO_STATIC_ROOT})
endif()
list(APPEND _poco_hint_roots /usr/local/poco-static /usr/local)

find_path(Poco_INCLUDE_DIR Poco/Poco.h
  HINTS ${_poco_hint_roots}
  PATH_SUFFIXES include
)
mark_as_advanced(FORCE Poco_INCLUDE_DIR)

find_package(Threads REQUIRED)
if(UNIX AND NOT APPLE)
  find_library(Poco_RT_LIBRARY rt)
endif()

set(_poco_common_deps Threads::Threads)
if(CMAKE_DL_LIBS)
  list(APPEND _poco_common_deps ${CMAKE_DL_LIBS})
endif()
if(Poco_RT_LIBRARY)
  list(APPEND _poco_common_deps ${Poco_RT_LIBRARY})
endif()

foreach(component ${Poco_FIND_COMPONENTS})
  set(component_var "Poco_${component}_LIBRARY")

  set(_poco_original_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
  set(CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_STATIC_LIBRARY_SUFFIX}")
  find_library(${component_var} Poco${component}
    HINTS ${_poco_hint_roots}
    PATH_SUFFIXES lib
  )
  set(CMAKE_FIND_LIBRARY_SUFFIXES "${_poco_original_suffixes}")
  mark_as_advanced(FORCE ${component_var})
  if(${component_var})
    set(Poco_${component}_FOUND TRUE)
    list(APPEND Poco_LIBRARIES ${component})
    if(NOT TARGET Poco::${component})
      get_filename_component(_poco_library_ext "${${component_var}}" LAST_EXT)
      if(NOT _poco_library_ext STREQUAL "${CMAKE_STATIC_LIBRARY_SUFFIX}")
        message(FATAL_ERROR "Static Poco library for component ${component} not found")
      endif()
      add_library(Poco::${component} STATIC IMPORTED)
      set_target_properties(Poco::${component} PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${Poco_INCLUDE_DIR}
        IMPORTED_LOCATION ${${component_var}}
      )
      if(component STREQUAL "Foundation")
        set_property(TARGET Poco::${component} PROPERTY
          INTERFACE_LINK_LIBRARIES "${_poco_common_deps}"
        )
      elseif(component STREQUAL "Net")
        set_property(TARGET Poco::${component} PROPERTY
          INTERFACE_LINK_LIBRARIES "Poco::Foundation;${_poco_common_deps}"
        )
      endif()
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

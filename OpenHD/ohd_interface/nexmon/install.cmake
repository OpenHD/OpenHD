# The supplied driver and nexutil are ARM32 binaries for the OpenHD Pi kernel.
# Other architectures must not install a bundle that selects this backend.
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm|armv[6-8]l|armhf)$" AND
   CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(_nexmon_bundle "${CMAKE_CURRENT_LIST_DIR}/pi4-6.1.29-v7l")
    file(STRINGS "${_nexmon_bundle}/SHA256SUMS" _nexmon_checksums)
    foreach(_line IN LISTS _nexmon_checksums)
        string(REGEX MATCH "^([0-9a-f]+)  ([A-Za-z0-9_.-]+)$" _match "${_line}")
        if(NOT _match)
            message(FATAL_ERROR "Malformed Nexmon bundle checksum: ${_line}")
        endif()
        set(_expected "${CMAKE_MATCH_1}")
        set(_filename "${CMAKE_MATCH_2}")
        file(SHA256 "${_nexmon_bundle}/${_filename}" _actual)
        if(NOT _actual STREQUAL _expected)
            message(FATAL_ERROR "Nexmon bundle checksum mismatch: ${_filename}")
        endif()
    endforeach()
    install(DIRECTORY "${_nexmon_bundle}/"
            DESTINATION lib/openhd/nexmon COMPONENT Nexmon
            PATTERN nexutil EXCLUDE)
    install(PROGRAMS "${_nexmon_bundle}/nexutil"
            DESTINATION lib/openhd/nexmon COMPONENT Nexmon)
    install(PROGRAMS "${CMAKE_CURRENT_LIST_DIR}/../../../scripts/openhd-nexmon-scout"
            DESTINATION libexec COMPONENT Nexmon)
    message(STATUS "Bundling Pi 4 Nexmon scout for 6.1.29-v7l+")
endif()

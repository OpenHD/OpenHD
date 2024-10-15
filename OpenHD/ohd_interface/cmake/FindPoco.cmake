# Build by Raphael Scholle for OpenHD

message("Searching for Poco Foundation and Net libraries")

if (Poco_FIND_REQUIRED)
    message("  (required)")
else()
    message("  (not required)")
endif()

# Search for Poco include directories (using Foundation.h for better precision)
find_path(Poco_INCLUDE_DIR Poco/Foundation.h
    HINTS
        $ENV{Poco_DIR}
        ${Poco_DIR}
        /usr/local/include
        /usr/include
        /opt/reCamera/output/sg2002_recamera_emmc/buildroot-2021.05/output/cvitek_CV181X_musl_riscv64/per-package/openhd/host/riscv64-buildroot-linux-musl/sysroot/usr/include
)

# List of libraries to search for (Foundation and Net only)
set(Poco_COMPONENTS Foundation Net)

foreach(component ${Poco_COMPONENTS})
    message(STATUS "Searching for Poco ${component} in ${Poco_DIR}/lib and sysroot paths...")
    
    # Search for each Poco component's library
    find_library(Poco_${component}_LIBRARY
        NAMES Poco${component}
        HINTS
            $ENV{Poco_DIR}/lib
            ${Poco_DIR}/lib
            /usr/local/lib
            /usr/lib
            /opt/reCamera/output/sg2002_recamera_emmc/buildroot-2021.05/output/cvitek_CV181X_musl_riscv64/per-package/openhd/host/riscv64-buildroot-linux-musl/sysroot/usr/lib
    )
    
    if (Poco_${component}_LIBRARY)
        message("Found Poco ${component} library: ${Poco_${component}_LIBRARY}")
        list(APPEND Poco_LIBRARIES ${Poco_${component}_LIBRARY})
    else()
        if (Poco_FIND_REQUIRED)
            message(FATAL_ERROR "Could not find Poco component ${component} library.")
        endif()
    endif()
endforeach()

# Check if both Foundation and Net were found
if (Poco_INCLUDE_DIR AND Poco_LIBRARIES)
    set(Poco_FOUND TRUE)
    message("Found Poco libraries: ${Poco_LIBRARIES}")
    message("Found Poco includes: ${Poco_INCLUDE_DIR}")
else()
    if (Poco_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find Poco libraries or includes.")
    endif()
endif()

# Set the output variables
set(Poco_INCLUDE_DIRS ${Poco_INCLUDE_DIR})
set(Poco_LIBRARIES ${Poco_LIBRARIES})

# Define imported targets for Poco::Foundation and Poco::Net
if (Poco_FOUND)
    add_library(Poco::Foundation UNKNOWN IMPORTED)
    set_target_properties(Poco::Foundation PROPERTIES
        IMPORTED_LOCATION "${Poco_FOUNDATION_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Poco_INCLUDE_DIRS}"
    )

    add_library(Poco::Net UNKNOWN IMPORTED)
    set_target_properties(Poco::Net PROPERTIES
        IMPORTED_LOCATION "${Poco_NET_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Poco_INCLUDE_DIRS}"
    )
endif()

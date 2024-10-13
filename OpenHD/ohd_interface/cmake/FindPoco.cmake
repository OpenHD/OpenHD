# Build by Raphael Scholle for OpenHD

message("Searching for Poco libraries")

if (Poco_FIND_REQUIRED)
    message("  (required)")
else()
    message("  (not required)")
endif()

# Search for Poco include directories
find_path(Poco_INCLUDE_DIR Poco/Util.h
    HINTS
        $ENV{Poco_DIR}
        ${Poco_DIR}
        /usr/local/include
        /usr/include
)

# List of libraries to search for, based on requested components
set(Poco_COMPONENTS
    Foundation
    Util
    JSON
    XML
    Zip
    Crypto
    Data
    Net
    NetSSL
    OSP
)

foreach(component ${Poco_FIND_COMPONENTS})
    if(NOT component IN_LIST Poco_COMPONENTS)
        message(FATAL_ERROR "Requested component '${component}' is not supported by FindPoco.")
    endif()

    # Search for each Poco component's library
    find_library(Poco_${component}_LIBRARY
        NAMES Poco${component}
        HINTS
            $ENV{Poco_DIR}/lib
            ${Poco_DIR}/lib
            /usr/local/lib
            /usr/lib
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

# Check if all components were found
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

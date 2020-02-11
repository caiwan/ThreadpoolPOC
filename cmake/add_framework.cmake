MACRO( ADD_FRAMEWORK fwname fwpath appname )
ENDMACRO()

MACRO( ADD_SYSTEM_FRAMEWORK fwname appname )
    FIND_LIBRARY( 
        FRAMEWORK_${fwname} 
        NAMES ${fwname} 
        PATHS ${CMAKE_OSX_SYSROOT}/System/Library 
        PATH_SUFFIXES Frameworks 
        NO_DEFAULT_PATH 
    )

    if( ${FRAMEWORK_${fwname}} STREQUAL FRAMEWORK_${fwname}-NOTFOUND )
        MESSAGE( ERROR ": Framework ${fwname} not found" )
    else()
        MARK_AS_ADVANCED (${FRAMEWORK_${fwname}})
        TARGET_LINK_LIBRARIES( ${appname} ${FRAMEWORK_${fwname}})
        MESSAGE( STATUS "Framework ${fwname} found at ${FRAMEWORK_${fwname}}" )
    endif()
ENDMACRO( ADD_SYSTEM_FRAMEWORK )

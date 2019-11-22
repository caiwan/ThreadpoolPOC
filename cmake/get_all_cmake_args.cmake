# Caiwan / IR
function(get_all_cmake_args RET_ARGS)
    get_cmake_property(vars CACHE_VARIABLES)
    foreach(var ${vars})
    get_property(currentHelpString CACHE "${var}" PROPERTY HELPSTRING)
        if("${currentHelpString}" MATCHES "No help, variable specified on the command line." OR "${currentHelpString}" STREQUAL "")
            # message("${var} = [${${var}}]  --  ${currentHelpString}") # uncomment to see the variables being processed
            list(APPEND CL_ARGS "-D${var}=${${var}}")
        endif()
    endforeach()
    SET(${RET_ARGS} ${CL_ARGS} PARENT_SCOPE)
endfunction()

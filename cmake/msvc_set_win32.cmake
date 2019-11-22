# sets win32 env for windows on msvc
# caiwan/IR

function(msvc_set_win32 _application)
  if(MSVC)
    set_target_properties(${_application} PROPERTIES LINK_FLAGS_DEVDEBUG "/SUBSYSTEM:WINDOWS")
    set_target_properties(${_application} PROPERTIES LINK_FLAGS_DEVRELEASE "/SUBSYSTEM:WINDOWS")
    set_target_properties(${_application} PROPERTIES LINK_FLAGS_DEVRELWITHDEBINFO "/SUBSYSTEM:WINDOWS")
    set_target_properties(${_application} PROPERTIES LINK_FLAGS_DEVMINSIZEREL "/SUBSYSTEM:WINDOWS")
    set_target_properties(${_application} PROPERTIES LINK_FLAGS_PRODDEBUG "/SUBSYSTEM:WINDOWS")
    set_target_properties(${_application} PROPERTIES LINK_FLAGS_PRODRELEASE "/SUBSYSTEM:WINDOWS")
    set_target_properties(${_application} PROPERTIES LINK_FLAGS_PRODRELWITHDEBINFO "/SUBSYSTEM:WINDOWS")
    set_target_properties(${_application} PROPERTIES LINK_FLAGS_PRODMINSIZEREL "/SUBSYSTEM:WINDOWS")

    set_target_properties(${_application} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
    set_target_properties(${_application} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS")
    set_target_properties(${_application} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
    set_target_properties(${_application} PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
  endif(MSVC)
endfunction(msvc_set_win32)

# sets win32 console env for windows on msvc
function(msvc_set_console _application)
  if(MSVC)
    set_target_properties(${_application} PROPERTIES LINK_FLAGS_DEVDEBUG "/SUBSYSTEM:CONSOLE")
    set_target_properties(${_application} PROPERTIES LINK_FLAGS_DEVRELEASE "/SUBSYSTEM:CONSOLE")
    set_target_properties(${_application} PROPERTIES LINK_FLAGS_DEVRELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
    set_target_properties(${_application} PROPERTIES LINK_FLAGS_DEVMINSIZEREL "/SUBSYSTEM:CONSOLE")
    set_target_properties(${_application} PROPERTIES LINK_FLAGS_PRODDEBUG "/SUBSYSTEM:CONSOLE")
    set_target_properties(${_application} PROPERTIES LINK_FLAGS_PRODRELEASE "/SUBSYSTEM:CONSOLE")
    set_target_properties(${_application} PROPERTIES LINK_FLAGS_PRODRELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
    set_target_properties(${_application} PROPERTIES LINK_FLAGS_PRODMINSIZEREL "/SUBSYSTEM:CONSOLE")

    set_target_properties(${_application} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
    set_target_properties(${_application} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
    set_target_properties(${_application} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:CONSOLE")
    set_target_properties(${_application} PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:CONSOLE")
  endif(MSVC)
endfunction(msvc_set_console)


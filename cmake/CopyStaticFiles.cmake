# Copies and install static files
# caiwan/ir

include(CMakeParseArguments)

# Copies static files within the binary dirs
# Usage:
# TARGET cmake target
# DESTINATION destination
# CONFIGURATIONS list of build configurations to be excuted at. If not provided will executed at both Release and Debug.
# FILES list of file to be copied
function(copy_static_files)
  cmake_parse_arguments(
    ARGS                   # prefix
    ""                     # flags
    "TARGET;DESTINATION"   # single-values
    "FILES;CONFIGURATIONS" # lists
    ${ARGN}
  )

  # check input
  if (NOT ARGS_TARGET)
    message(FATAL_ERROR "You must provide a target")
  endif ()

  if (NOT ARGS_DESTINATION)
    message(FATAL_ERROR "You must provide a destination path")
  endif ()

  if (NOT ARGS_FILES)
    message(FATAL_ERROR "You must provide the list of files to be copied")
  endif ()

  # provide default parameters

  if (NOT ARGS_CONFIGURATIONS)
    set(ARGS_CONFIGURATIONS, "Release;Debug")
  endif ()

  # create copy commands
  set(dst_path "${ARGS_DESTINATION}")
  file(MAKE_DIRECTORY ${dst_path})

  foreach (_file IN ITEMS ${ARGS_FILES})

    if (IS_ABSOLUTE "${_file}")
      file(RELATIVE_PATH _file_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_file}")
    else ()
      set(_file_rel "${_file}")
    endif ()

    get_filename_component(_file_path "${_file_rel}" PATH)
    get_filename_component(_file_name "${_file_rel}" NAME)

    # Skip internal files
    # TODO: These should be avoided BEFORE macro call anyways
    if ("${_source}" MATCHES "CMake*\\.(txt)\\s?")
      continue()
    elseif ("${_source}" MATCHES ".*\\.(git*)\\s?")
      continue()
    endif ()

    # Note: It's a list which will be expanded w/ COMMAND_EXPAND_LISTS
    set(_cmd "")
    if (GENERATOR_IS_MULTI_CONFIG)
      foreach (_config IN ITEMS ${ARGS_CONFIGURATIONS})
        string(
          APPEND _cmd
          "${CMAKE_COMMAND};-E;"
          "$<IF:$<CONFIG:${_config}>,"
          "echo;${_config}:$<CONFIG>;" # ---
          "copy_if_different;${_file};${dst_path};"
          ","
          "echo;${_config}:$<CONFIG>;" # ---
          "echo;.;"
          ">"
        )
      endforeach (_config)
    else (GENERATOR_IS_MULTI_CONFIG)
      string(
        APPEND _cmd
        "${CMAKE_COMMAND};-E;"
        "copy_if_different;${_file};${dst_path};"
      )
    endif ()

    add_custom_command(
      TARGET ${ARGS_TARGET}
      POST_BUILD
      COMMAND "${_cmd}"
      COMMAND_EXPAND_LISTS
    )

  endforeach (_file)

endfunction(copy_static_files)

# Recurse copy static files within the binary dirs
# Usage:
# TARGET cmake target
# SOURCE source path
# DESTINATION destination path
# CONFIGURATIONS list of build configurations to be excuted at. If not provided will executed at both Release and Debug.
# PATTERNS list of patterns. "*" default
function(recurse_copy_static_files)
  cmake_parse_arguments(
    ARGS
    ""
    "TARGET;DESTINATION;SOURCE"
    "PATTERNS;CONFIGURATIONS"
    ${ARGN}
  )

  # check input
  if (NOT ARGS_TARGET)
    message(FATAL_ERROR "You must provide a target")
  endif ()

  if (NOT ARGS_DESTINATION)
    message(FATAL_ERROR "You must provide a destination path")
  endif ()

  if (NOT ARGS_SOURCE)
    message(FATAL_ERROR "You must provide the list of files to be copied")
  endif ()

  if (NOT ARGS_PATTERNS)
    set(ARGS_PATTERNS "*")
  endif ()

  if (NOT ARGS_CONFIGURATIONS)
    set(ARGS_CONFIGURATIONS, "Release;Debug")
  endif ()

  foreach (_pattern IN ITEMS ${ARGS_PATTERNS})
    message("file(GLOB_RECURSE _files ${ARGS_SOURCE}/${_pattern})")
    file(GLOB_RECURSE _files "${ARGS_SOURCE}/${_pattern}")
    foreach (_file IN ITEMS ${_files})
      file(RELATIVE_PATH _file_rel_path ${ARGS_SOURCE} ${_file})

      get_filename_component(_file_path "${_file_rel_path}" PATH)
      get_filename_component(_file_name "${_file_rel_path}" NAME)

      if ("${_file_name}" MATCHES "CMake*\\.(txt)\\s?")
        continue()
      elseif ("${_file_name}" MATCHES ".*\\.(git*)\\s?")
        continue()
      elseif ("${_file_name}" MATCHES ".*\\.(md5)\\s?")
        continue()
      endif ()

      copy_static_files(
        TARGET ${ARGS_TARGET}
        DESTINATION ${ARGS_DESTINATION}/${_file_path}
        FILES ${ARGS_SOURCE}/${_file_rel_path}
        CONFIGURATIONS ${ARGS_CONFIGURATIONS}
      )

    endforeach ()
  endforeach ()

endfunction(recurse_copy_static_files)

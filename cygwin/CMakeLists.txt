﻿add_subdirectory(cyglaunch)
set_target_properties(
  cyglaunch
  PROPERTIES FOLDER cygwin)

add_subdirectory(cyglib)
set_target_properties(
  cyglib
  PROPERTIES FOLDER cygwin)

if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
  # linuxでは、以下はビルドしない
  return()
endif()

# cygtool, innosetupで利用する 32bit nativeなdll
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  # 32bit build
  add_subdirectory(cygtool)

  set_target_properties(
    cygtool
    PROPERTIES
    FOLDER cygwin
    )
  set_target_properties(
    cygtool_tester
    PROPERTIES
    FOLDER cygwin
    )
else()
  # other build system...
  if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12.0")
    add_subdirectory(cygtool_build)
    if(TARGET cygtool)
      set_target_properties(
        cygtool
        PROPERTIES FOLDER cygwin)
    endif()
  endif()
endif()

# cygterm
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12.0")
  add_subdirectory(cygterm_build)
  set_target_properties(
    cygterm_build
    PROPERTIES FOLDER cygwin)
endif()

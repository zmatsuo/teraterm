﻿set(PACKAGE_NAME "TTXOutputBuffering")

project(${PACKAGE_NAME})

add_library(
  ${PACKAGE_NAME} SHARED
  TTXOutputBuffering.c
  ReadMe.txt
  ReadMe-ja.txt
  )

if(SUPPORT_OLD_WINDOWS)
  if(MSVC)
    target_sources(
      ${PACKAGE_NAME}
      PRIVATE
      ../../teraterm/common/compat_w95_vs2005.c
      )
  endif()
  if(MINGW)
    target_sources(
      ${PACKAGE_NAME}
      PRIVATE
      ../../teraterm/libmingw/tlssup.c
      )
  endif()
endif(SUPPORT_OLD_WINDOWS)

set_target_properties(
  ${PACKAGE_NAME}
  PROPERTIES
  PREFIX ""
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/"
  )

target_include_directories(
  ${PACKAGE_NAME}
  PRIVATE
  ../../teraterm/teraterm
  ../../teraterm/common
  )

install(
  TARGETS ${PACKAGE_NAME}
  RUNTIME
  DESTINATION .
)

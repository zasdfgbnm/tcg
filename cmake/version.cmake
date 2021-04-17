option(VERSION "Version of this project")
if(NOT VERSION)
  message("-- Version not specified, will try to infer version")

  # infer version
  execute_process(COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/shells/xonsh/setup.py --version
                  OUTPUT_VARIABLE INFERRED_VERSION ERROR_QUIET)
  string(REGEX REPLACE "\n$" "" INFERRED_VERSION "${INFERRED_VERSION}")
  if("${INFERRED_VERSION}" STREQUAL "" OR "${INFERRED_VERSION}" STREQUAL "unknown")
    message("--   Unable to infer version")
  else()
    set(VERSION "${INFERRED_VERSION}")
    message("--   Version inferred as ${VERSION}")
  endif()

  # infer git commit
  execute_process(COMMAND git -C ${CMAKE_CURRENT_SOURCE_DIR} rev-parse HEAD
                  OUTPUT_VARIABLE GIT_COMMIT ERROR_QUIET)
  string(REGEX REPLACE "\n$" "" GIT_COMMIT "${GIT_COMMIT}")
  if("${GIT_COMMIT}" STREQUAL "")
    message("--   Unknown git commit")
  else()
    message("--   Git commit is ${GIT_COMMIT}")
  endif()
endif()

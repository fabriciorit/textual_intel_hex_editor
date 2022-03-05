cmake_minimum_required(VERSION 3.00)
project(TIHex VERSION 2022.03.02.001)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(CTest)
enable_testing()

add_executable(tihex
  TIHex.cpp
  main.cpp
  )
target_link_libraries(tihex)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# Get the current working branch
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
# Get the latest commit date in a custom format
execute_process(
  COMMAND git log -1 --format=%cd --date=format:%Y.%m.%d
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_DATE
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
# Get the latest commit hash
execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_definitions("-DGIT_BRANCH=\"${GIT_BRANCH}\"")
add_definitions("-DGIT_COMMIT_DATE=\"${GIT_COMMIT_DATE}\"")
add_definitions("-DGIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\"")


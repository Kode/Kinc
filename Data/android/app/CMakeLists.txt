cmake_minimum_required(VERSION 3.4.1)

add_library(
  kore
  SHARED
{files})

find_library(
  log-lib
  log
)

target_link_libraries(
  kore
  ${log-lib}
)

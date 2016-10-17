cmake_minimum_required(VERSION 3.4.1)

add_definitions(
{defines})

include_directories(
{includes})

add_library(
  kore
  SHARED
{files})

{libraries1}
target_link_libraries(
  kore
{libraries2})

cmake_minimum_required(VERSION 3.0)

project(DGBP
  LANGUAGES C
  DESCRIPTION "A debug print for SDL2"
  HOMEPAGE_URL "https://github.com/superzazu/DBGP")

find_package(SDL2 REQUIRED)

add_library(DBGP STATIC DBGP.c)
target_include_directories(DBGP PRIVATE ${SDL2_INCLUDE_DIRS} .)
target_link_libraries(DBGP PRIVATE ${SDL2_LIBRARIES})
set_target_properties(DBGP PROPERTIES C_STANDARD 99)
if (MSVC)
  target_compile_options(DBGP PRIVATE /W4)
else()
  target_compile_options(DBGP PRIVATE -Wall -Wextra -pedantic)
endif()

# example executable
add_executable(DBGP-example example.c)
target_include_directories(DBGP-example PRIVATE ${SDL2_INCLUDE_DIRS} .)
target_link_libraries(DBGP-example PRIVATE ${SDL2_LIBRARIES} DBGP)
set_target_properties(DBGP-example PROPERTIES C_STANDARD 99)

cmake_minimum_required(VERSION 3.5)

project(SDL_DBGP
  LANGUAGES C
  DESCRIPTION "Display UTF-8 text in SDL3 programs (in a VGA-like text mode)"
  HOMEPAGE_URL "https://github.com/superzazu/SDL_DBGP")

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3)

add_library(SDL_DBGP STATIC SDL_DBGP.c)
target_link_libraries(SDL_DBGP PRIVATE SDL3::SDL3)
set_target_properties(SDL_DBGP PROPERTIES C_STANDARD 99)
if (MSVC)
  target_compile_options(SDL_DBGP PRIVATE /W4)
else()
  target_compile_options(SDL_DBGP PRIVATE -Wall -Wextra -pedantic)
endif()

# example executable
add_executable(SDL_DBGP-example example.c)
target_link_libraries(SDL_DBGP-example PRIVATE SDL3::SDL3 SDL_DBGP)
set_target_properties(SDL_DBGP-example PROPERTIES C_STANDARD 99)
if (MSVC)
  target_compile_options(SDL_DBGP-example PRIVATE /W4)
else()
  target_compile_options(SDL_DBGP-example PRIVATE -Wall -Wextra -pedantic)
endif()

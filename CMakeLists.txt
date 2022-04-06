cmake_minimum_required(VERSION 3.0)

project(SDL_DBGP
  LANGUAGES C
  DESCRIPTION "Display ASCII text in SDL2 programs (in a VGA-like text mode)"
  HOMEPAGE_URL "https://github.com/superzazu/SDL_DBGP")

find_package(SDL2 REQUIRED)

add_library(SDL_DBGP STATIC SDL_DBGP.c)
target_include_directories(SDL_DBGP PRIVATE ${SDL2_INCLUDE_DIRS} .)
target_link_libraries(SDL_DBGP PRIVATE ${SDL2_LIBRARIES})
set_target_properties(SDL_DBGP PROPERTIES C_STANDARD 99)
if (MSVC)
  target_compile_options(SDL_DBGP PRIVATE /W4)
else()
  target_compile_options(SDL_DBGP PRIVATE -Wall -Wextra -pedantic)
endif()

# example executable
add_executable(SDL_DBGP-example example.c)
target_include_directories(SDL_DBGP-example PRIVATE ${SDL2_INCLUDE_DIRS} .)
target_link_libraries(SDL_DBGP-example PRIVATE ${SDL2_LIBRARIES} SDL_DBGP)
set_target_properties(SDL_DBGP-example PROPERTIES C_STANDARD 99)

cmake_minimum_required(VERSION 3.16)

project(terminal-renderer LANGUAGES C)

add_executable(terminal-renderer
  src/core/grid.c
  src/main.c
  src/renderer/renderer.c
  src/renderer/lighting.c
  src/renderer/framebuffer.c
  src/renderer/rasterizer/rasterizer.c
  src/renderer/rasterizer/shapes.c
  src/renderer/raymarcher/raymarcher.c
  src/renderer/raymarcher/sdf.c
  src/terminal/tui.c
  src/terminal/ui.c
)

target_include_directories(terminal-renderer PRIVATE src)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(terminal-renderer PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_compile_definitions(terminal-renderer PRIVATE _POSIX_C_SOURCE=200809L)

target_link_libraries(terminal-renderer PRIVATE m)

set_target_properties(
  terminal-renderer
  PROPERTIES C_STANDARD 99
              C_STANDARD_REQUIRED YES
              C_EXTENSIONS NO)

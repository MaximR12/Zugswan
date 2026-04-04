add_library(GUI STATIC)

target_sources(GUI PRIVATE
  src/Window.cpp
)

target_include_directories(GUI PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

target_link_libraries(GUI PRIVATE glad::glad glfw Engine Rendering)

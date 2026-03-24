add_library(Rendering STATIC)

target_sources(Rendering PRIVATE
  src/Renderer.cpp
  src/ResourceManager.cpp
  src/Shader.cpp
  src/SpriteRenderer.cpp
  src/Texture.cpp
)

find_package(Stb REQUIRED)

target_include_directories(Rendering PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${Stb_INCLUDE_DIR}
)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

target_link_libraries(Rendering PRIVATE glad::glad glfw glm::glm GUI Engine)
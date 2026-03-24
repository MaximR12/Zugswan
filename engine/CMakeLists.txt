set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(Engine STATIC)

target_sources(Engine PRIVATE
  src/GameState.cpp
  src/MoveGen.cpp
  src/Board.cpp
)

target_include_directories(Engine PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(ChessEngine main.cpp)

target_link_libraries(ChessEngine PRIVATE GUI Engine Rendering)
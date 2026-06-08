set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(Engine STATIC)

target_sources(Engine PRIVATE
  src/GameState.cpp
  src/MoveGen.cpp
  src/Board.cpp
  src/Engine.cpp
  src/Tables.cpp
)

target_include_directories(Engine PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(ChessEngine Game.cpp)
add_executable(PerftTests Perft.cpp)

target_link_libraries(ChessEngine PRIVATE GUI Engine Rendering)
target_link_libraries(PerftTests PRIVATE Engine)
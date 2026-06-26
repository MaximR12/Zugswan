set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(Engine STATIC)

target_sources(Engine PRIVATE
  src/gamestate.cpp
  src/movegen.cpp
  src/board.cpp
  src/engine.cpp
  src/tables.cpp
  src/search.cpp
  tests/perft.cpp
  src/uci.cpp
)

target_include_directories(Engine PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(Zugswan src/main.cpp)

target_link_libraries(Zugswan PRIVATE Engine)
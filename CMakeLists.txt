cmake_minimum_required(VERSION 3.10)
project(XadrezIA CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

set(ENGINE_SOURCES
    src/board/Board.cpp
    src/game/GameState.cpp
    src/ai/Evaluator.cpp
    src/ai/MinimaxAI.cpp
    src/ai/AIFactory.cpp
    src/ui/CLI.cpp
)

add_executable(xadrez ${ENGINE_SOURCES} src/main.cpp)
target_include_directories(xadrez PRIVATE include)

add_executable(tests ${ENGINE_SOURCES} tests/tests.cpp)
target_include_directories(tests PRIVATE include)

enable_testing()
add_test(NAME unit_tests COMMAND tests)

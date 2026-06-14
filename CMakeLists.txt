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
)

find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

add_executable(xadrez ${ENGINE_SOURCES} src/ui/GUI.cpp src/main.cpp)
target_include_directories(xadrez PRIVATE include)
target_link_libraries(xadrez PRIVATE SFML::Graphics SFML::Window SFML::System)

add_executable(tests ${ENGINE_SOURCES} src/ui/CLI.cpp tests/tests.cpp)
target_include_directories(tests PRIVATE include)

enable_testing()
add_test(NAME unit_tests COMMAND tests)

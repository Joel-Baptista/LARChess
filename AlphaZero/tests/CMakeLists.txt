cmake_minimum_required(VERSION 3.2)

project(test_bot)

SET(CURRENT_USER "joel" CACHE STRING "Some user-specified option")

# set(CURRENT_USER joel)

message(STATUS "Current user: ${CURRENT_USER}")
message(STATUS "/home/${CURRENT_USER}/projects/libtorch")

# set(CMAKE_PREFIX_PATH /home/${CURRENT_USER}/projects/libtorch)
set(CMAKE_PREFIX_PATH /home/${CURRENT_USER}/projects/libtorch-cuda118)
# set(CMAKE_PREFIX_PATH /home/${CURRENT_USER}/projects/libtorch-cuda124)

set(PROJECT_ROOT /home/${CURRENT_USER}/projects/LARChess)

find_package(Torch REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -flto")

add_executable(eval_bot
        
        ${PROJECT_ROOT}/AlphaZero/tests/src/eval_bot.cpp
        
        ${PROJECT_ROOT}/BBChessEngine/src/bit_board.cpp

        ${PROJECT_ROOT}/AlphaZero/src/utils/utils.cpp
        ${PROJECT_ROOT}/AlphaZero/src/agent/mcts.cpp

        ${PROJECT_ROOT}/AlphaZero/src/game/game.cpp
        ${PROJECT_ROOT}/AlphaZero/src/include/ResNet.h
        ${PROJECT_ROOT}/AlphaZero/src/include/network.h
        
)
target_link_libraries(eval_bot "${TORCH_LIBRARIES}" pthread)

set_property(TARGET eval_bot PROPERTY CXX_STANDARD 17)

add_executable(test_game
        
        ${PROJECT_ROOT}/AlphaZero/tests/src/encoding_decoding.cpp
        
        ${PROJECT_ROOT}/BBChessEngine/src/bit_board.cpp

        ${PROJECT_ROOT}/AlphaZero/src/utils/utils.cpp
        ${PROJECT_ROOT}/AlphaZero/src/agent/mcts.cpp

        ${PROJECT_ROOT}/AlphaZero/src/game/game.cpp
        
)
target_link_libraries(test_game "${TORCH_LIBRARIES}" pthread)

set_property(TARGET test_game PROPERTY CXX_STANDARD 17)

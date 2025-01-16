cmake_minimum_required(VERSION 3.2)

project(test_alpha)

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
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")

add_executable(${PROJECT_NAME} 
        
        ${PROJECT_ROOT}/BBChessEngine/src/bit_board.cpp

        ${PROJECT_ROOT}/AlphaZero/src/tests/test_alpha.cpp
        ${PROJECT_ROOT}/AlphaZero/src/agent/mcts.cpp
        ${PROJECT_ROOT}/AlphaZero/src/agent/mctsMT.cpp
        ${PROJECT_ROOT}/AlphaZero/src/game/game.cpp
        ${PROJECT_ROOT}/AlphaZero/src/utils/utils.cpp
        ${PROJECT_ROOT}/AlphaZero/src/include/ResNet.h
        ${PROJECT_ROOT}/AlphaZero/src/include/network.h
        
)
target_link_libraries(${PROJECT_NAME} PUBLIC "${TORCH_LIBRARIES}" pthread)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_ROOT}/AlphaZero/src/headers)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

add_executable(test_mctsMT 
        
        ${PROJECT_ROOT}/BBChessEngine/src/bit_board.cpp

        ${PROJECT_ROOT}/AlphaZero/src/tests/test_mctsMT.cpp
        ${PROJECT_ROOT}/AlphaZero/src/agent/mcts.cpp
        ${PROJECT_ROOT}/AlphaZero/src/agent/mctsMT.cpp
        ${PROJECT_ROOT}/AlphaZero/src/game/game.cpp
        ${PROJECT_ROOT}/AlphaZero/src/utils/utils.cpp
        ${PROJECT_ROOT}/AlphaZero/src/include/ResNet.h
        ${PROJECT_ROOT}/AlphaZero/src/include/network.h
        
)
target_link_libraries(test_mctsMT PUBLIC "${TORCH_LIBRARIES}" pthread)
target_include_directories(test_mctsMT PRIVATE ${PROJECT_ROOT}/AlphaZero/src/headers)
set_property(TARGET test_mctsMT PROPERTY CXX_STANDARD 17)

add_executable(big_o_test 
        
        ${PROJECT_ROOT}/BBChessEngine/src/bit_board.cpp

        ${PROJECT_ROOT}/AlphaZero/src/tests/big_o_test.cpp
        ${PROJECT_ROOT}/AlphaZero/src/agent/mcts.cpp
        ${PROJECT_ROOT}/AlphaZero/src/agent/mctsMT.cpp
        ${PROJECT_ROOT}/AlphaZero/src/game/game.cpp
        ${PROJECT_ROOT}/AlphaZero/src/utils/utils.cpp
        ${PROJECT_ROOT}/AlphaZero/src/include/ResNet.h
        ${PROJECT_ROOT}/AlphaZero/src/include/network.h
        
)
target_link_libraries(big_o_test PUBLIC "${TORCH_LIBRARIES}" pthread)
target_include_directories(big_o_test PRIVATE ${PROJECT_ROOT}/AlphaZero/src/headers)
set_property(TARGET big_o_test PROPERTY CXX_STANDARD 17)
                


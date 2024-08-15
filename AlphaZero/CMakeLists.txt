cmake_minimum_required(VERSION 3.2)

project(AlphaZero)

set(CMAKE_PREFIX_PATH /home/joel/projects/libtorch)
set(PROJECT_ROOT "/home/joel/projects/YACE")


find_package(Torch REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

add_executable(${PROJECT_NAME} 
        
        ${PROJECT_ROOT}/BBChessEngine/src/bit_board.cpp

        ${PROJECT_ROOT}/AlphaZero/src/main.cpp
        ${PROJECT_ROOT}/AlphaZero/src/mcts.cpp
        ${PROJECT_ROOT}/AlphaZero/src/AlphaZero.cpp
        ${PROJECT_ROOT}/AlphaZero/src/game.cpp
        ${PROJECT_ROOT}/AlphaZero/src/include/ResNet.h
        ${PROJECT_ROOT}/AlphaZero/src/include/network.h
        
)

target_link_libraries(${PROJECT_NAME} "${TORCH_LIBRARIES}")

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
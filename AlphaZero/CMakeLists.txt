cmake_minimum_required(VERSION 3.2)

project(AlphaZero)

SET(CURRENT_USER "joel" CACHE STRING "Some user-specified option")

# set(CURRENT_USER joel)

message(STATUS "Current user: ${CURRENT_USER}")
message(STATUS "/home/${CURRENT_USER}/projects/libtorch")

# set(CMAKE_PREFIX_PATH /home/${CURRENT_USER}/projects/libtorch)
set(CMAKE_PREFIX_PATH /home/${CURRENT_USER}/projects/libtorch-cuda118)
# set(CMAKE_PREFIX_PATH /home/${CURRENT_USER}/projects/libtorch-cuda124)

set(PROJECT_ROOT /home/${CURRENT_USER}/projects/YACE)

find_package(Torch REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -flto")

add_executable(${PROJECT_NAME} 
        
        ${PROJECT_ROOT}/BBChessEngine/src/bit_board.cpp

        ${PROJECT_ROOT}/AlphaZero/src/main.cpp
        ${PROJECT_ROOT}/AlphaZero/src/utils.cpp
        ${PROJECT_ROOT}/AlphaZero/src/mcts.cpp
#        ${PROJECT_ROOT}/AlphaZero/src/AlphaZero.cpp
        ${PROJECT_ROOT}/AlphaZero/src/AlphaZeroMT.cpp
        ${PROJECT_ROOT}/AlphaZero/src/game.cpp
        ${PROJECT_ROOT}/AlphaZero/src/include/ResNet.h
        ${PROJECT_ROOT}/AlphaZero/src/include/network.h
        
)
target_link_libraries(${PROJECT_NAME} "${TORCH_LIBRARIES}" pthread)

add_executable(${PROJECT_NAME}_sl 

        ${PROJECT_ROOT}/BBChessEngine/src/bit_board.cpp
        ${PROJECT_ROOT}/AlphaZero/src/utils.cpp
        ${PROJECT_ROOT}/AlphaZero/src/dataset.cpp
        ${PROJECT_ROOT}/AlphaZero/src/supLearn.cpp
        ${PROJECT_ROOT}/AlphaZero/src/game.cpp
        ${PROJECT_ROOT}/AlphaZero/src/include/ResNet.h
        ${PROJECT_ROOT}/AlphaZero/src/include/network.h

)

target_link_libraries(${PROJECT_NAME}_sl "${TORCH_LIBRARIES}" pthread)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

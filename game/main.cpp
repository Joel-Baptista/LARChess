// Chess Engine
#include "../src/board.h"
#include "../src/move.h"
#include "../src/utils.h"
#include "../src/minmax.h"

// Misc Imports
#include <iostream>
#include <random>
#include <fstream>
#include <sstream>
#include <thread>

// My OpenGL Engine
#include "include/OpenGLEngine/Renderer.h"
#include "include/OpenGLEngine/VertexBuffer.h"
#include "include/OpenGLEngine/IndexBuffer.h"
#include "include/OpenGLEngine/VertexArray.h"  
#include "include/OpenGLEngine/Shader.h"
#include "include/OpenGLEngine/VertexBufferLayout.h"
#include "include/OpenGLEngine/Texture.h"
#include <GL/glew.h>
#include <GL/glut.h>
#include <GLFW/glfw3.h>

// glm for matrix transformations
#include "include/glm/glm.hpp"
#include "include/glm/gtc/matrix_transform.hpp"

// ImGui for debugging
#include "include/imgui/imgui.h"
#include "include/imgui/imgui_impl_glfw_gl3.h"

// ChessGUI
#include "ChessGUI.h"

Move get_random_move(std::unordered_map<std::string, std::vector<Move>> all_legal_moves);
std::vector<Move> flatten_moves(std::unordered_map<std::string, std::vector<Move>> all_legal_moves);
Move human_move();
int king_count(std::array<std::array<int, 8>, 8> board);

int main() {

    GLFWwindow* window;
    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    
    std::cout << "Number of monitors: " << count << std::endl;
    int width_mm, height_mm;
    // glfwGetMonitorPhysicalSize(primary, &width_mm, &height_mm);

    // std::cout << "Width: " << width_mm << "mm, Height: " << height_mm << "mm" << std::endl;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Set the major version of OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Set the minor version of OpenGL
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Set the profile of OpenGL to core (No backwards compatibility)

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1920, 1080, "Lar Chess", NULL, NULL);
    glfwSetWindowAspectRatio(window, 19, 10);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1); // Enable VSync

    // Initialize GLEW only after creating the window
    if (glewInit() != GLEW_OK) 
        std::cout << "Error!" << std::endl;

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)); // Set the blending function
    GLCall(glEnable(GL_BLEND)); // Enable blending

    Renderer renderer; // Instantiate a renderer

    ImGui::CreateContext();

    ImGui_ImplGlfwGL3_Init(window, true);

    ImGui::StyleColorsDark();

    ChessGUI chessGUI(window);

    Board board;

    // board.set_from_fen("2Rk4/8/7p/r7/4K3/1P5P/2B5/r7 b  - 6 109");
    bool human_vs_human = false;
    bool human_white = true;
    bool machine_vs_machine = false;
    bool stop_program = false;
    int num_games = 1;
    int depth = 3;

    board.reset();
    
    // /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {   
        /* Render here */
        GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f)); // Set the clear color

        // Get the current window size

        renderer.Clear(); // Clear the color buffer
        ImGui_ImplGlfwGL3_NewFrame();

        chessGUI.OnUpdate(0.0f);
        chessGUI.OnRender();

        ImGui::Begin("Test");

        chessGUI.OnImGuiRender();
        ImGui::End();

        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();


        if (board.turn_player == 1){
            std::string gui_move = chessGUI.get_player_move();
            if (gui_move != ""){
                Move player_move(gui_move.substr(0,2), gui_move.substr(2,2));
                board.make_move(player_move);
                chessGUI.set_board(board.board);
            }
        }else if (board.turn_player == -1){
            std::tuple<std::string, double> result = minmax(board, depth, false, -1000000, 1000000);
            Move player_move(std::get<0>(result).substr(0,2), std::get<0>(result).substr(2,2));
            std::cout << "Best move: " << player_move.getFrom() << player_move.getTo() << " with value " << std::get<1>(result) << std::endl;
            board.make_move(player_move);
            chessGUI.set_board(board.board);
        }

        // Move player_move;
        // if (board.turn_player == 1){
        //     std::string move_input;
            
        //     // std::cout << "White to move" << std::endl;
            
        //     if (machine_vs_machine){
        //         std::tuple<std::string, double> result = minmax(board, depth, true, -1000000, 1000000);
        //         player_move = Move(std::get<0>(result).substr(0,2), std::get<0>(result).substr(2,2));
        //         std::cout << "Best move: " << player_move.getFrom() << player_move.getTo() << " with value " << std::get<1>(result) << std::endl;
        //     }else if (human_vs_human){
        //         player_move = human_move();
        //     }else if (!human_vs_human && human_white){
        //         player_move = human_move();
        //     }else{
        //         std::tuple<std::string, double> result = minmax(board, depth, true, -1000000, 1000000);
        //         player_move = Move(std::get<0>(result).substr(0,2), std::get<0>(result).substr(2,2));
        //         std::cout << "Best move: " << player_move.getFrom() << player_move.getTo() << " with value " << std::get<1>(result) << std::endl;
        //     }
        //     // player_move = get_random_move(board.legal_moves);
        //     // std::cout << "White move: " << player_move.getFrom() << player_move.getTo() << std::endl;

        // }else if (board.turn_player == -1){
        //     // std::cout << "Black to move" << std::endl;

        //     if (machine_vs_machine){
        //         std::tuple<std::string, double> result = minmax(board, depth, true, -1000000, 1000000);
        //         player_move = Move(std::get<0>(result).substr(0,2), std::get<0>(result).substr(2,2));
        //         std::cout << "Best move: " << player_move.getFrom() << player_move.getTo() << " with value " << std::get<1>(result) << std::endl;
        //     }else if (human_vs_human){
        //         player_move = human_move();
        //     }else if (!human_vs_human && !human_white){
        //         player_move = human_move();
        //     }else{
        //         std::tuple<std::string, double> result = minmax(board, depth, false, -1000000, 1000000);
        //         player_move = Move(std::get<0>(result).substr(0,2), std::get<0>(result).substr(2,2));
        //         std::cout << "Best move: " << player_move.getFrom() << player_move.getTo() << " with value " << std::get<1>(result) << std::endl;
        //     }
        //     // player_move = get_random_move(board.legal_moves);
        //     // std::cout << "Black move: " << player_move.getFrom() << player_move.getTo() << std::endl;
        // }

    }

    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();

    // if (human_vs_human){
    //     machine_vs_machine = false;
    // }

    // for (int i=0; i<num_games; i++){
    //     board.reset();
    //     std::cout << "Starting position in game " << i+1 << std::endl;
    //     while (!board.is_terminal() && !stop_program){
            
    //         if (!machine_vs_machine){
    //             board.show();
    //         }
    //         // std::cout << "FEN: " << board.get_fen() << std::endl;

            // if (king_count(board.board) != 2){
            //     std::cout << "King count: " << king_count(board.board) << std::endl;
            //     stop_program = true;
            //     break;
            // }

            // board.get_all_legal_moves();

            // std::string debug_square = "a5";
            // for (int i=0; i<board.legal_moves[debug_square].size(); i++){
            //     std::cout << "Legal move: " << board.legal_moves[debug_square][i].getFrom() << board.legal_moves[debug_square][i].getTo() << std::endl;
            // }

            // Move player_move;

            // std::cout << "Hash of board = " << board.get_board_hash() << std::endl;
            
            // if (board.turn_player == 1){
            //     std::string move_input;
                
            //     // std::cout << "White to move" << std::endl;
                
            //     if (machine_vs_machine){
            //         std::tuple<std::string, double> result = minmax(board, depth, true, -1000000, 1000000);
            //         player_move = Move(std::get<0>(result).substr(0,2), std::get<0>(result).substr(2,2));
            //         std::cout << "Best move: " << player_move.getFrom() << player_move.getTo() << " with value " << std::get<1>(result) << std::endl;
            //     }else if (human_vs_human){
            //         player_move = human_move();
            //     }else if (!human_vs_human && human_white){
            //         player_move = human_move();
            //     }else{
            //         std::tuple<std::string, double> result = minmax(board, depth, true, -1000000, 1000000);
            //         player_move = Move(std::get<0>(result).substr(0,2), std::get<0>(result).substr(2,2));
            //         std::cout << "Best move: " << player_move.getFrom() << player_move.getTo() << " with value " << std::get<1>(result) << std::endl;
            //     }
            //     // player_move = get_random_move(board.legal_moves);
            //     // std::cout << "White move: " << player_move.getFrom() << player_move.getTo() << std::endl;

            // }else if (board.turn_player == -1){
            //     // std::cout << "Black to move" << std::endl;

            //     if (machine_vs_machine){
            //         std::tuple<std::string, double> result = minmax(board, depth, true, -1000000, 1000000);
            //         player_move = Move(std::get<0>(result).substr(0,2), std::get<0>(result).substr(2,2));
            //         std::cout << "Best move: " << player_move.getFrom() << player_move.getTo() << " with value " << std::get<1>(result) << std::endl;
            //     }else if (human_vs_human){
            //         player_move = human_move();
            //     }else if (!human_vs_human && !human_white){
            //         player_move = human_move();
            //     }else{
            //         std::tuple<std::string, double> result = minmax(board, depth, false, -1000000, 1000000);
            //         player_move = Move(std::get<0>(result).substr(0,2), std::get<0>(result).substr(2,2));
            //         std::cout << "Best move: " << player_move.getFrom() << player_move.getTo() << " with value " << std::get<1>(result) << std::endl;
            //     }


            //     // player_move = get_random_move(board.legal_moves);
            //     // std::cout << "Black move: " << player_move.getFrom() << player_move.getTo() << std::endl;
            // }

            // board.make_move(player_move);
        }

        // std::cout << "Final position: " << board.get_fen() << std::endl;
        // board.show();

        // std::cout << "Game over" << std::endl;

        // if (stop_program){
        //     break;
        // }
    // }

// }

Move get_random_move(std::unordered_map<std::string, std::vector<Move>> all_legal_moves){

    std::vector<Move> legal_moves_flattened = flatten_moves(all_legal_moves);
    std::random_device rd;  // Obtain a random number from hardware
    std::mt19937 gen(rd()); // Seed the generator
    std::uniform_int_distribution<> distr(0, legal_moves_flattened.size()-1);

    return legal_moves_flattened[distr(gen)];
}

std::vector<Move> flatten_moves(std::unordered_map<std::string, std::vector<Move>> all_legal_moves){
    std::vector<Move> moves;
    int count = 0;
    for (auto const& [key, value] : all_legal_moves){
        moves.insert(moves.end(), value.begin(), value.end());

        // for (int i=0; i<value.size(); i++){
        //     std::cout << value[i].getFrom() << value[i].getTo() << std::endl;
        // }

    }
    return moves;
}

Move human_move(){
    std::string move_input;
    std::cout << "Enter move: ";
    std::cin >> move_input;

    if (move_input.length() == 5){
        Move move(move_input.substr(0,2), move_input.substr(2,2), move_input[4]);
        return move;
    }else{
        Move move(move_input.substr(0,2), move_input.substr(2,2));
        return move;
    }
}

int king_count(std::array<std::array<int, 8>, 8> board){
    int count = 0;
    for (int i=0; i<8; i++){
        for (int j=0; j<8; j++){
            if (board[i][j] == 6 || board[i][j] == -6){
                count++;
            }
        }
    }
    return count;
}
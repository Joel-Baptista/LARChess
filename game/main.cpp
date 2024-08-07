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
#include <future>
#include <chrono>
#include <ctime>  
#include <unistd.h>

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

std::string game[] = {
"1k6/4K3/8/1PQpP2p/3P3P/3P4/8/8 b  - 4 54",
"8/1k2K3/8/1PQpP2p/3P3P/3P4/8/8 w  - 5 55",
"8/1k2K3/2Q5/1P1pP2p/3P3P/3P4/8/8 b  - 6 55",
"1k6/4K3/2Q5/1P1pP2p/3P3P/3P4/8/8 w  - 7 56",
"1k6/4K3/8/1PQpP2p/3P3P/3P4/8/8 b  - 8 56",
"8/1k2K3/8/1PQpP2p/3P3P/3P4/8/8 w  - 9 57",
"8/1k2K3/2Q5/1P1pP2p/3P3P/3P4/8/8 b  - 10 57",
"1k6/4K3/2Q5/1P1pP2p/3P3P/3P4/8/8 w  - 11 58",
"1k6/4K3/3Q4/1P1pP2p/3P3P/3P4/8/8 b  - 12 58",
"2k5/4K3/3Q4/1P1pP2p/3P3P/3P4/8/8 w  - 13 59",
"2k5/3QK3/8/1P1pP2p/3P3P/3P4/8/8 b  - 14 59",
"1k6/3QK3/8/1P1pP2p/3P3P/3P4/8/8 w  - 15 60",
"1k6/3QK3/1P6/3pP2p/3P3P/3P4/8/8 b  - 0 60",
"k7/3QK3/1P6/3pP2p/3P3P/3P4/8/8 w  - 1 61",
"k2K4/3Q4/1P6/3pP2p/3P3P/3P4/8/8 b  - 2 61",
"1k1K4/3Q4/1P6/3pP2p/3P3P/3P4/8/8 w  - 3 62",
"1k1K4/3Q4/1P2P3/3p3p/3P3P/3P4/8/8 b  - 0 62",
"k2K4/3Q4/1P2P3/3p3p/3P3P/3P4/8/8 w  - 1 63",
"k2K4/3QP3/1P6/3p3p/3P3P/3P4/8/8 b  - 0 63",
"1k1K4/3QP3/1P6/3p3p/3P3P/3P4/8/8 w  - 1 64",
"1k1KQ3/3Q4/1P6/3p3p/3P3P/3P4/8/8 b  - 0 64",
"k2KQ3/3Q4/1P6/3p3p/3P3P/3P4/8/8 w  - 1 65",
"k2KQ3/8/1P6/3Q3p/3P3P/3P4/8/8 b  - 0 65",
"1k1KQ3/8/1P6/3Q3p/3P3P/3P4/8/8 w  - 1 66",
"1k1KQ3/3Q4/1P6/7p/3P3P/3P4/8/8 b  - 2 66",
"k2KQ3/3Q4/1P6/7p/3P3P/3P4/8/8 w  - 3 67",
"k2KQ3/3Q4/1P6/3P3p/7P/3P4/8/8 b  - 0 67",
"1k1KQ3/3Q4/1P6/3P3p/7P/3P4/8/8 w  - 1 68",
"1k1KQ3/3Q4/1P6/3P3p/3P3P/8/8/8 b  - 0 68",
"k2KQ3/3Q4/1P6/3P3p/3P3P/8/8/8 w  - 1 69",
"k2KQ3/3Q4/1P1P4/7p/3P3P/8/8/8 b  - 0 69",
"1k1KQ3/3Q4/1P1P4/7p/3P3P/8/8/8 w  - 1 70",
"1k1KQ3/3Q4/1P1P4/3P3p/7P/8/8/8 b  - 0 70",
"k2KQ3/3Q4/1P1P4/3P3p/7P/8/8/8 w  - 1 71",
"k2KQ3/Q7/1P1P4/3P3p/7P/8/8/8 b  - 2 71",
};

struct BotResult{
    std::string move;
    double evaluation;
    bool processed = false;
};
Move get_random_move(std::unordered_map<std::string, std::vector<Move>> all_legal_moves);
std::vector<Move> flatten_moves(std::unordered_map<std::string, std::vector<Move>> all_legal_moves);
Move human_move();
int king_count(std::array<std::array<int, 8>, 8> board);
BotResult bot_callback(Board board, int depth, int turn_player);

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

    board.set_from_fen("1k1KQ3/3Q4/1P1P4/7p/3P3P/8/8/8 w  - 1 70");
    bool replay_game = true;
    bool human_vs_human = true;
    bool bot_vs_bot = true;
    int human_player = 1;
    int depth = 4;
    bool game_finished = false;
    BotResult result;
    bool processing_flag = false;
    std::future<BotResult> future;
    // board.reset();
    chessGUI.set_board(board.board);
    
    int game_count = 0;

    // /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {   
        /* Render here */
        GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f)); // Set the clear color

        // Get the current window size

        renderer.Clear(); // Clear the color buffer
        ImGui_ImplGlfwGL3_NewFrame();

        // std::cout << "1.Render Cleared" << std::endl;

        chessGUI.OnUpdate(0.0f);

        // std::cout << "2.OnUpdate" << std::endl;
        chessGUI.OnRender();

        // std::cout << "3.OnRender" << std::endl;
        ImGui::Begin("Test");

        chessGUI.OnImGuiRender();
        ImGui::End();

        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

        // std::cout << "4.ImGUI" << std::endl;

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        if (replay_game)
        {
            board.set_from_fen(game[game_count]);
            chessGUI.set_board(board.board);
            game_count++;
            
            auto start = std::chrono::system_clock::now();

            std::chrono::duration<double> elapsed_seconds = start - start;

            while(elapsed_seconds.count() < 0.5) 
            {
                auto end = std::chrono::system_clock::now();
                elapsed_seconds = end-start;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            continue;
        }

        // std::cout << "5.PollEvents" << std::endl;
        // std::cout << "Engine" << std::endl;
        if (!game_finished){
            if (human_vs_human){
                // std::cout << "Quering the GUI" << std::endl;
                std::string gui_move = chessGUI.get_player_move();
                // std::cout << "GUI move" << gui_move << std::endl;
                if (gui_move != ""){
                    Move player_move(gui_move.substr(0,2), gui_move.substr(2,2));
                    board.make_move(player_move);
                    chessGUI.set_board(board.board);
                }
            }else if (board.turn_player == human_player && !bot_vs_bot){
                std::string gui_move = chessGUI.get_player_move();
                if (gui_move != ""){
                    Move player_move(gui_move.substr(0,2), gui_move.substr(2,2));
                    board.make_move(player_move);
                    chessGUI.set_board(board.board);
                }
            }else if (board.turn_player == -1 * human_player || bot_vs_bot){
                // std::thread bt(bot_callback, std::ref(result), std::ref(board), std::ref(depth), std::ref(human_player));

                if (!processing_flag){
                    future = std::async(std::launch::async, bot_callback, board, depth, board.turn_player);
                    processing_flag = true;
                    chessGUI.lock_board();
                }

                if (future.wait_for(std::chrono::milliseconds(10)) == std::future_status::ready) {
                    result = future.get();
                } 

                if (result.processed){
                    Move player_move(result.move.substr(0,2), result.move.substr(2,2));
                    // std::cout << "Best move: " << player_move.getFrom() << player_move.getTo() << " with value " << result.evaluation << std::endl;
                    board.make_move(player_move);
                    std::cout << board.get_fen() << std::endl;
                    chessGUI.set_board(board.board);
                    chessGUI.unlock_board();
                    processing_flag = false;
                    result.move = "";
                    result.evaluation = 0.0;
                    result.processed = false;
                }
            }
        }

        if (board.is_terminal()){
            game_finished = true;
        }

        // std::cout << "6.Make Move" << std::endl;
        //  std::cout << "----------------------------------------------------------" << std::endl;
    }

     std::cout << "7.End Program" << std::endl;
    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();

}

BotResult bot_callback(Board board, int depth, int turn_player){
    BotResult result;

    auto start = std::chrono::system_clock::now(); // Bot should have a minimum compute duration to not break visualization
    // Some computation here

    // std::cout << "Depth: " << depth << std::endl;
    // std::cout << "Maximizing: " << (bool)(turn_player > 0) << std::endl;

    auto output = minmax(board, depth, (turn_player > 0), -1000000, 1000000);

    result.move = output.move;
    result.evaluation = output.evaluation;
    result.processed = true;

    // std::cout << "Move: " << (result.move) << " Evaluation: " << result.evaluation << std::endl;
    auto end = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end-start;

    while(elapsed_seconds.count() < 0.5) 
    {
        auto end = std::chrono::system_clock::now();
        elapsed_seconds = end-start;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return result;
}

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
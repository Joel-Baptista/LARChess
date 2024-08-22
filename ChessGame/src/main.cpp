// Chess Engine
#include "../../ChessEngine/src/board.h"
#include "../../ChessEngine/src/move.h"
#include "../../utils/utils.h"
#include "../../ChessEngine/src/minmax.h"

// Misc Imports
#include <iostream>
#include <random>
#include <fstream>
#include <sstream>
#include <thread>
#include <future>

#include "../../ChessGUI/src/ChessGUI.h"

// ChessGUI
// #include "ChessGUI.h"

struct BotResult{
    std::string move;
    double evaluation;
    bool processed = false;
};
Move get_random_move(std::unordered_map<std::string, std::vector<Move>> all_legal_moves);
std::vector<Move> flatten_moves(std::unordered_map<std::string, std::vector<Move>> all_legal_moves);
Move human_move();
int king_count(std::array<std::array<int, 8>, 8> board);
BotResult bot_callback(Board board, int depth, int human_player);

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

    ChessGUI chessGUI(window, "/home/joel/projects/YACE/ChessGUI/res");

    Board board;

    board.set_from_fen("6k2/4Q3/6K2/8/8/8/8/8 b - 0 22");
    bool human_vs_human = false;
    int human_player = 1;
    int depth = 4;
    BotResult result;
    bool processing_flag = false;
    std::future<BotResult> future;
    board.reset();
    chessGUI.set_board(board.board);
    
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

        // std::cout << "5.PollEvents" << std::endl;
        // std::cout << "Engine" << std::endl;

        if (human_vs_human){
            // std::cout << "Quering the GUI" << std::endl;
            std::string gui_move = chessGUI.get_player_move();
            // std::cout << "GUI move" << gui_move << std::endl;
            if (gui_move != ""){
                Move player_move(gui_move.substr(0,2), gui_move.substr(2,2));
                board.make_move(player_move);
                chessGUI.set_board(board.board);
            }
        }else if (board.turn_player == human_player){
            std::string gui_move = chessGUI.get_player_move();
            if (gui_move != ""){
                Move player_move(gui_move.substr(0,2), gui_move.substr(2,2));
                board.make_move(player_move);
                chessGUI.set_board(board.board);
            }
        }else if (board.turn_player == -1 * human_player){
            // std::thread bt(bot_callback, std::ref(result), std::ref(board), std::ref(depth), std::ref(human_player));

            if (!processing_flag){
                future = std::async(std::launch::async, bot_callback, board, depth, human_player);
                processing_flag = true;
                chessGUI.lock_board();
            }

            if (future.wait_for(std::chrono::milliseconds(10)) == std::future_status::ready) {
                std::cout << "Future is ready" << std::endl;
                result = future.get();
                std::cout << "Result: " << result.move << std::endl;
                std::cout << "Processed: " << result.processed << std::endl;
            } 

            if (result.processed){
                std::cout << "Play move" << std::endl;
                Move player_move(result.move.substr(0,2), result.move.substr(2,2));
                std::cout << "Best move: " << player_move.getFrom() << player_move.getTo() << " with value " << result.evaluation << std::endl;
                board.make_move(player_move);
                chessGUI.set_board(board.board);
                chessGUI.unlock_board();
                processing_flag = false;
                result.move = "";
                result.evaluation = 0.0;
                result.processed = false;
            }
        
        }

        if (board.is_checkmate()){
            break;
        }

        // std::cout << "6.Make Move" << std::endl;
        //  std::cout << "----------------------------------------------------------" << std::endl;
    }

     std::cout << "7.End Program" << std::endl;
    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();

}

BotResult bot_callback(Board board, int depth, int human_player){
    BotResult result;
    std::cout << "Inside thread" << std::endl;
    auto output = minmax(board, depth, (human_player < 0), -1000000, 1000000);
    std::cout << "Minmax calculated" << std::endl;
    result.move = output.move;
    result.evaluation = output.evaluation;
    result.processed = true;

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
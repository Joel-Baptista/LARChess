// Chess Engine
#include "../../BBChessEngine/src/bit_board.h"
#include "../../AlphaZero/src/alphabot.h"

// Misc Imports
#include <iostream>
#include <random>
#include <fstream>
#include <sstream>
#include <thread>
#include <future>
#include <mutex>
#include <condition_variable>
#include <queue>

// ChessGUI
#include "../../ChessGUI/src/ChessGUI.h"


#include "include/json.hpp"
using json = nlohmann::json; 

struct BotResult{
    int move;
    float evaluation;
    bool processed = false;
};

// Backend to frontend queue
std::queue<std::array<std::array<int, 8>, 8>> backendToFrontendQueue;
std::mutex mtxBackendToFrontend;
std::condition_variable cvBackendToFrontend;

// Frontend to backend queue
std::queue<std::string> frontendToBackendQueue;
std::mutex mtxFrontendToBackend;
std::condition_variable cvFrontendToBackend;

std::queue<bool> lockBoardQueue;
std::mutex mtxLockBoard;
std::condition_variable cvLockBoard;

bool backendDone = false;
bool frontendDone = false;

void backend(bool human_vs_human, int human_player, int depth, std::string initial_position)
{

    BitBoard board;

    board.parse_fen(initial_position.c_str());

    { // Use scopes to elegantly eliminate the lock guard
        std::lock_guard<std::mutex> lock(mtxBackendToFrontend);
        backendToFrontendQueue.push(board.bitboard_to_board());
        cvBackendToFrontend.notify_one();
    }

    while (!frontendDone)
    {
        if (board.get_side() == !human_player && !human_vs_human)
        {   
            {
                std::lock_guard<std::mutex> lock(mtxLockBoard);
                lockBoardQueue.push(true);
                cvLockBoard.notify_one();
            }


            board.reset_leaf_nodes();
            float eval = board.alpha_beta(depth, -1000000, 1000000, true);
            board.make_bot_move(board.get_bot_best_move());
            {
                std::lock_guard<std::mutex> lock(mtxBackendToFrontend);
                backendToFrontendQueue.push(board.bitboard_to_board());
                cvBackendToFrontend.notify_one();
            }

            {
                std::lock_guard<std::mutex> lock(mtxLockBoard);
                lockBoardQueue.push(false);
                cvLockBoard.notify_one();
            }
        }
        else
        {
            std::string gui_move;
            {
                std::unique_lock<std::mutex> lock(mtxFrontendToBackend);
                cvFrontendToBackend.wait(lock, [] { return !frontendToBackendQueue.empty(); });
                std::string receivedData = frontendToBackendQueue.front();
                frontendToBackendQueue.pop();
                gui_move = receivedData;
            }

            if (gui_move != ""){
                board.make_player_move(gui_move.c_str());  
                { 
                    std::lock_guard<std::mutex> lock(mtxBackendToFrontend);
                    backendToFrontendQueue.push(board.bitboard_to_board());
                    cvBackendToFrontend.notify_one();
                }
            }
        }
    }
}

void alphabackend(bool human_vs_human, int human_player, std::string initial_position, json config)
{
    int num_searches = config.value("num_searches", 5);
    float dichirlet_alpha = config.value("dichirlet_alpha", 0.5);
    float dichirlet_epsilon = config.value("dichirlet_epsilon", 0.3);
    float C = config.value("C", 1.41);
    float temperature = config.value("temperature", 1.0);
    int num_resblocks = config.value("num_resblocks", 7);
    int num_channels = config.value("num_channels", 256);
    std::string model_path = config.value("model_path", "");
    std::string device = config.value("device", "cpu");

    AlphaBot bot(std::make_shared<Game>(), 
                 num_searches, 
                 C, 
                 temperature,
                 dichirlet_alpha, 
                 dichirlet_epsilon, 
                 num_resblocks, 
                 num_channels, 
                 device);

    if (model_path != "")
        bot.load_model(model_path);
        
    bot.m_Game->m_Board->parse_fen(initial_position.c_str());

    { // Use scopes to elegantly eliminate the lock guard
        std::lock_guard<std::mutex> lock(mtxBackendToFrontend);
        backendToFrontendQueue.push(bot.m_Game->m_Board->bitboard_to_board());
        cvBackendToFrontend.notify_one();
    }

    while (!frontendDone)
    {
        if (bot.m_Game->m_Board->get_side() == !human_player && !human_vs_human)
        {   
            {
                std::lock_guard<std::mutex> lock(mtxLockBoard);
                lockBoardQueue.push(true);
                cvLockBoard.notify_one();
            }

            // std::cout << "Bot is thinking..." << std::endl;
            std::string move = bot.predict();
            // std::cout << "Bot move: " << move << std::endl;
            bot.make_bot_move(move);
            // std::cout << "Bot move made" << std::endl;

            {
                std::lock_guard<std::mutex> lock(mtxBackendToFrontend);
                backendToFrontendQueue.push(bot.m_Game->m_Board->bitboard_to_board());
                cvBackendToFrontend.notify_one();
            }

            {
                std::lock_guard<std::mutex> lock(mtxLockBoard);
                lockBoardQueue.push(false);
                cvLockBoard.notify_one();
            }
        }
        else
        {
            std::string gui_move;
            {
                std::unique_lock<std::mutex> lock(mtxFrontendToBackend);
                cvFrontendToBackend.wait(lock, [] { return !frontendToBackendQueue.empty(); });
                std::string receivedData = frontendToBackendQueue.front();
                frontendToBackendQueue.pop();
                gui_move = receivedData;
            }
            if (gui_move != ""){
                bot.m_Game->m_Board->make_player_move(gui_move.c_str());
                { 
                    std::lock_guard<std::mutex> lock(mtxBackendToFrontend);
                    backendToFrontendQueue.push(bot.m_Game->m_Board->bitboard_to_board());
                    cvBackendToFrontend.notify_one();
                }
            }
        }
    }
}

int frontend()
{
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

    {
        std::unique_lock<std::mutex> lock(mtxBackendToFrontend);
        cvBackendToFrontend.wait(lock, [] { return !backendToFrontendQueue.empty() || backendDone; });
        std::array<std::array<int, 8>, 8> board_data = backendToFrontendQueue.front();
        backendToFrontendQueue.pop();
        chessGUI.set_board(board_data);
    }

    
    // /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window) && !backendDone)
    {   
        auto st = get_time_ms();

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
        // glfwWaitEvents();

        if (!lockBoardQueue.empty()) {
            std::lock_guard<std::mutex> lock(mtxLockBoard);
            bool lockBoard = lockBoardQueue.front();
            lockBoardQueue.pop();
            if (lockBoard){
                chessGUI.lock_board();
            } else {
                chessGUI.unlock_board();
            }
        }

        if (!backendToFrontendQueue.empty()) {
            std::lock_guard<std::mutex> lock(mtxBackendToFrontend);
            std::array<std::array<int, 8>, 8> board_data = backendToFrontendQueue.front();
            backendToFrontendQueue.pop();
            chessGUI.set_board(board_data);
        }

        if (chessGUI.is_move_stored()){ 
            std::lock_guard<std::mutex> lock(mtxFrontendToBackend);
            frontendToBackendQueue.push(chessGUI.get_player_move()); // Just an example of processing
            chessGUI.reset_move_stored();
            cvFrontendToBackend.notify_one();
        }

        // while (get_time_ms() - st < 1000.0f / 30.0f)
        // {
        //     std::this_thread::sleep_for(std::chrono::milliseconds(1));
        // }
    }

     std::cout << "7.End Program" << std::endl;
    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();

    return 0;
}



int main() {
    std::ifstream config_file("../cfg/config.json");
    if (!config_file.is_open()) {
        std::cerr << "Could not open the config file!" << std::endl;
        return 1;
    }

    // Parse the JSON data
    json config;
    try {
        config_file >> config;
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return 1;
    }

    // Extract values from the JSON object
    int depth = config.value("depth", 4);
    bool human_vs_human = config.value("human_vs_human", false);
    std::string initial_position = config.value("initial_position", initial_position);
    std::string human_player = config.value("human_player", "w");
    std::string bot = config.value("bot", "minmax");

    std::ifstream alpha_config_file("../cfg/alpha_config.json");
    if (!alpha_config_file.is_open()) {
        std::cerr << "Could not open the config file!" << std::endl;
        return 1;
    }

    // Parse the JSON data
    json alpha_config;
    try {
        alpha_config_file >> alpha_config;
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return 1;
    }

    if (bot == "minmax")
    {
        std::cout << "Starting MinMax Backend" << std::endl;
        std::thread frontendThread(frontend);
        std::thread backendThread(backend, human_vs_human, human_player == "w" ? 0 : 1, depth, initial_position);
    
        frontendThread.join();
        backendThread.join();
    
    }
    else
    {
        std::cout << "Starting AlphaZero Backend" << std::endl;
        std::thread frontendThread(frontend);
        std::thread backendThread(alphabackend, human_vs_human, human_player == "w" ? 0 : 1, initial_position, alpha_config);
    
        frontendThread.join();
        backendThread.join();
    }

    
}


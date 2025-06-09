#include <iostream>
#include "aof/aof.h"
#include "aof/mccfr.h"
#include <random>
#include <chrono>
#include <ctime>
#include <fstream>

int main(int argc, char* argv[]) {
    std::cout << "Program started\n";
    
    try {
        std::cout << "Creating random device...\n";
        std::random_device rd;
        std::cout << "Creating generator...\n";
        std::mt19937 gen(rd());
        
        std::cout << "Creating game...\n";

        std::pair<float, float> stakes = {0.10f, 0.25f};

        GameParameters params = getGameParameters(stakes);

        float smallBlind = 0.4;
        float bigBlind = 1.0;

        AoFGame game(smallBlind, bigBlind, params.rake_per_hand, params.jackpot_fee_per_hand, params.jackpot_payout_percentage);

        
        std::cout << "Creating initial state...\n";
        AoFState state = game.newInitialState();
        
        std::cout << "Starting MCCFR training...\n";
        int iterations = 1e7;
        if (argc > 1) {
            iterations = std::stoi(argv[1]);
        }
        trainMCCFR(game, iterations);
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm* now_tm = std::localtime(&now_time);
        
        std::string filename = "corrected_final_infosets_" + 
                               std::to_string(now_tm->tm_year + 1900) + "_" +
                               std::to_string(now_tm->tm_mon + 1) + "_" +
                               std::to_string(now_tm->tm_mday) + "_" +
                               std::to_string(now_tm->tm_hour) + "_" +
                               std::to_string(now_tm->tm_min) + "_" +
                               std::to_string(now_tm->tm_sec) + ".txt";
        saveInfoSetsToFile(filename);
        
    } catch (const std::exception& e) {
        std::cerr << "Error caught: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error caught" << std::endl;
        return 1;
    }
    
    std::cout << "Program completed\n";
    return 0;
}
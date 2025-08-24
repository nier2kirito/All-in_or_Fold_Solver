#include <iostream>
#include <chrono>
#include <iomanip>
#include "aof/game.hpp"
#include "aof/game_config.hpp"
#include "mccfr/trainer.hpp"

int main() {
    std::cout << "=== Basic MCCFR Training Example ===\n\n";
    
    try {
        // Create game with default blinds
        double smallBlind = 0.4;
        double bigBlind = 1.0;
        
        aof::Game game(smallBlind, bigBlind);
        
        std::cout << "Game Configuration:\n";
        std::cout << "  Small Blind: " << game.getSmallBlind() << "\n";
        std::cout << "  Big Blind: " << game.getBigBlind() << "\n";
        std::cout << "  Players: " << aof::GameConfig::NUM_PLAYERS << "\n";
        std::cout << "  Starting Stack: " << aof::GameConfig::STARTING_STACK_BB << " BB\n\n";
        
        // Create trainer
        mccfr::Trainer trainer(game);
        
        // Configure training for quick example
        mccfr::TrainingConfig config;
        config.iterations = 100000;  // Smaller number for quick demo
        config.enableProgressOutput = true;
        config.progressUpdateInterval = 10000;
        config.outputPrefix = "example_strategy";
        
        std::cout << "Starting training with " << config.iterations << " iterations...\n";
        
        auto startTime = std::chrono::steady_clock::now();
        auto utilities = trainer.train(config);
        auto endTime = std::chrono::steady_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
        
        std::cout << "\n=== Training Complete ===\n";
        std::cout << "Training time: " << duration.count() << " seconds\n";
        std::cout << "Information sets learned: " << trainer.getStats().informationSetsCount << "\n";
        std::cout << "Final utilities:\n";
        
        for (std::size_t i = 0; i < utilities.size(); ++i) {
            std::cout << "  Player " << i << ": " 
                      << std::fixed << std::setprecision(6) << utilities[i] << "\n";
        }
        
        std::cout << "\nStrategy file saved with timestamp.\n";
        std::cout << "Use strategy_analysis example to analyze the results.\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

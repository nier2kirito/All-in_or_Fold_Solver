/**
 * @file data_logging_example.cpp
 * @brief Example showing how to use data logging in MCCFR training
 */

#include "mccfr/trainer.hpp"
#include "aof/game.hpp"
#include <iostream>

int main() {
    // Create a game instance
    aof::Game game(0.4, 1.0);  // Small blind: 0.4, Big blind: 1.0
    
    // Create trainer
    mccfr::Trainer trainer(game);
    
    // Configure training with data logging
    mccfr::TrainingConfig config;
    config.iterations = 5000;                    // Number of iterations
    config.enableProgressOutput = true;          // Show progress
    config.enableUtilityTracking = true;        // Track utilities
    config.utilityUpdateInterval = 500;         // Report every 500 iterations
    
    // Data logging configuration
    config.enableDataLogging = true;            // Enable data logging
    config.dataLogFile = "detailed_training.csv"; // Output filename
    config.dataLogInterval = 50;                // Log every 50 iterations
    
    std::cout << "=== MCCFR Training with Data Logging ===\n";
    std::cout << "Configuration:\n";
    std::cout << "  Iterations: " << config.iterations << "\n";
    std::cout << "  Data log file: " << config.dataLogFile << "\n";
    std::cout << "  Data log interval: " << config.dataLogInterval << "\n";
    std::cout << "========================================\n\n";
    
    // Run training
    auto finalUtilities = trainer.train(config);
    
    // Print final results
    std::cout << "\n=== Final Results ===\n";
    std::cout << "Final utilities: [";
    for (std::size_t i = 0; i < finalUtilities.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << finalUtilities[i];
    }
    std::cout << "]\n";
    
    // Calculate final sum
    double sum = 0.0;
    for (double utility : finalUtilities) {
        sum += utility;
    }
    std::cout << "Final sum: " << sum << "\n";
    
    std::cout << "\nTo plot the results, run:\n";
    std::cout << "python plot_training_data.py " << config.dataLogFile << "\n";
    
    return 0;
}

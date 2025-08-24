/**
 * @file realtime_training.cpp
 * @brief Example showing real-time visualization during MCCFR training
 */

#include "mccfr/trainer.hpp"
#include "aof/game.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    // Create a game instance
    aof::Game game(0.4, 1.0);  // Small blind: 0.4, Big blind: 1.0
    
    // Create trainer
    mccfr::Trainer trainer(game);
    
    // Configure training for real-time visualization
    mccfr::TrainingConfig config;
    config.iterations = 10000;                    // Longer training for better visualization
    config.enableProgressOutput = false;         // Disable console spam for cleaner output
    config.enableUtilityTracking = false;        // Disable periodic utility reports
    
    // Real-time visualization configuration
    config.enableDataLogging = true;             // Enable data logging
    config.dataLogFile = "realtime_training.csv"; // Output filename
    config.dataLogInterval = 5;                  // Log every 5 iterations (more frequent)
    config.enableRealtimeVisualization = true;   // Enable real-time mode
    
    std::cout << "=== Real-time MCCFR Training Visualization ===\n";
    std::cout << "Configuration:\n";
    std::cout << "  Iterations: " << config.iterations << "\n";
    std::cout << "  Data log file: " << config.dataLogFile << "\n";
    std::cout << "  Data log interval: " << config.dataLogInterval << " iterations\n";
    std::cout << "===============================================\n\n";
    
    std::cout << "INSTRUCTIONS:\n";
    std::cout << "1. Open another terminal\n";
    std::cout << "2. Run: python realtime_visualizer.py --csv " << config.dataLogFile << "\n";
    std::cout << "3. Watch the plots update in real-time as training progresses!\n\n";
    
    std::cout << "Press Enter to start training (make sure visualizer is running first)...";
    std::cin.get();
    
    std::cout << "Starting training with real-time visualization...\n\n";
    
    // Add a small delay to let the user start the visualizer
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Run training
    auto startTime = std::chrono::steady_clock::now();
    auto finalUtilities = trainer.train(config);
    auto endTime = std::chrono::steady_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // Print final results
    std::cout << "\n=== Training Complete ===\n";
    std::cout << "Total time: " << duration.count() << " ms\n";
    std::cout << "Final utilities: [";
    for (std::size_t i = 0; i < finalUtilities.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << std::fixed << std::setprecision(6) << finalUtilities[i];
    }
    std::cout << "]\n";
    
    // Calculate final sum
    double sum = 0.0;
    for (double utility : finalUtilities) {
        sum += utility;
    }
    std::cout << "Final sum: " << std::scientific << sum << "\n";
    
    std::cout << "\nThe real-time visualizer should now show the complete training curve!\n";
    std::cout << "Data saved to: " << config.dataLogFile << "\n";
    
    return 0;
}

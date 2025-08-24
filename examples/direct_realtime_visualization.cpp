/**
 * @file direct_realtime_visualization.cpp
 * @brief Example demonstrating direct real-time visualization during MCCFR training
 * 
 * This example shows how to use the integrated real-time visualization system
 * that streams data directly from C++ to Python plots without intermediate files.
 */

#include "mccfr/trainer.hpp"
#include "aof/game.hpp"
#include <iostream>
#include <chrono>

int main() {
    std::cout << "=== Direct Real-time MCCFR Visualization Demo ===\n\n";
    
    // Create a game instance
    aof::Game game(0.4, 1.0);  // Small blind: 0.4, Big blind: 1.0
    
    // Create trainer
    mccfr::Trainer trainer(game);
    
    // Configure training with direct real-time visualization
    mccfr::TrainingConfig config;
    config.iterations = 2000;                    // Moderate training for good visualization
    config.enableProgressOutput = true;         // Show progress
    config.enableUtilityTracking = false;       // Disable console spam
    
    // Configure direct real-time visualization
    config.realtimeConfig.enabled = true;              // Enable direct visualization
    config.realtimeConfig.updateInterval = 5;          // Update every 5 iterations
    config.realtimeConfig.maxDataPoints = 1000;        // Keep 1000 data points
    config.realtimeConfig.showConsoleStats = true;     // Show periodic stats
    config.realtimeConfig.windowTitle = "Direct MCCFR Visualization Demo";
    
    // Also enable data logging for comparison
    config.enableDataLogging = true;
    config.dataLogFile = "direct_demo.csv";
    config.dataLogInterval = 5;
    
    std::cout << "Configuration:\n";
    std::cout << "  Iterations: " << config.iterations << "\n";
    std::cout << "  Real-time updates: every " << config.realtimeConfig.updateInterval << " iterations\n";
    std::cout << "  Max data points: " << config.realtimeConfig.maxDataPoints << "\n";
    std::cout << "  Data log file: " << config.dataLogFile << "\n";
    std::cout << "  Window title: " << config.realtimeConfig.windowTitle << "\n\n";
    
    std::cout << "Features of this demo:\n";
    std::cout << "  ✓ Direct C++ to Python communication (no intermediate files)\n";
    std::cout << "  ✓ Live plots that update automatically\n";
    std::cout << "  ✓ Real-time MAE and utility tracking\n";
    std::cout << "  ✓ Zero-sum verification indicator\n";
    std::cout << "  ✓ Console statistics every 50 iterations\n\n";
    
    std::cout << "Watch for:\n";
    std::cout << "  • Decreasing MAE (convergence)\n";
    std::cout << "  • Stable player utilities\n";
    std::cout << "  • Zero-sum verification (✓ indicator)\n";
    std::cout << "  • Live plot window opening automatically\n\n";
    
    std::cout << "Press Enter to start training with direct real-time visualization...";
    std::cin.get();
    
    // Run training with direct visualization
    auto startTime = std::chrono::steady_clock::now();
    auto finalUtilities = trainer.train(config);
    auto endTime = std::chrono::steady_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // Print final results
    std::cout << "\n=== Demo Complete ===\n";
    std::cout << "Total training time: " << duration.count() << " ms\n";
    std::cout << "Final utilities: [";
    for (std::size_t i = 0; i < finalUtilities.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << std::fixed << std::setprecision(6) << finalUtilities[i];
    }
    std::cout << "]\n";
    
    // Calculate and verify final sum
    double sum = 0.0;
    for (double utility : finalUtilities) {
        sum += utility;
    }
    std::cout << "Final sum: " << std::scientific << sum;
    if (std::abs(sum) < 1e-10) {
        std::cout << " ✓ (perfect zero-sum!)\n";
    } else {
        std::cout << " ⚠ (not quite zero-sum)\n";
    }
    
    std::cout << "\nWhat happened:\n";
    std::cout << "  • C++ trainer streamed data directly to Python via pipe\n";
    std::cout << "  • Python matplotlib updated plots in real-time\n";
    std::cout << "  • No intermediate CSV files were created or used\n";
    std::cout << "  • Visualization continued after training completed\n";
    std::cout << "  • Data was also logged to '" << config.dataLogFile << "' for comparison\n\n";
    
    std::cout << "The visualization window should still be open for you to examine!\n";
    std::cout << "This demonstrates true real-time streaming visualization.\n";
    
    return 0;
}

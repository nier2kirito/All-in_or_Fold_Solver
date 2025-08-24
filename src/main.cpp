#include <iostream>
#include <exception>
#include <string>
#include <chrono>
#include <iomanip>

#include "aof/game.hpp"
#include "aof/game_config.hpp"
#include "mccfr/trainer.hpp"

/**
 * @brief Print usage information
 */
void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [OPTIONS]\n\n";
    std::cout << "All-or-Fold MCCFR Trainer - Train optimal strategies for All-or-Fold poker\n\n";
    std::cout << "OPTIONS:\n";
    std::cout << "  -i, --iterations <num>     Number of training iterations (default: 1000000)\n";
    std::cout << "  -s, --small-blind <amount> Small blind amount (default: 0.4)\n";
    std::cout << "  -b, --big-blind <amount>   Big blind amount (default: 1.0)\n";
    std::cout << "  -o, --output <prefix>      Output file prefix (default: strategy)\n";
    std::cout << "  -q, --quiet               Suppress progress output\n";
    std::cout << "  --realtime                Enable real-time visualization mode\n";
    std::cout << "  --log-interval <num>      Data logging interval (default: 10)\n";
    std::cout << "  -h, --help                Show this help message\n\n";
    std::cout << "EXAMPLES:\n";
    std::cout << "  " << programName << "                    # Train with default settings\n";
    std::cout << "  " << programName << " -i 5000000         # Train for 5M iterations\n";
    std::cout << "  " << programName << " -s 0.1 -b 0.2      # Use 10c/20c stakes\n";
    std::cout << "  " << programName << " -o my_strategy -q  # Custom output, quiet mode\n";
}

/**
 * @brief Parse command line arguments
 */
struct Config {
    int iterations = 1000000;
    double smallBlind = 0.4;
    double bigBlind = 1.0;
    std::string outputPrefix = "strategy";
    bool quiet = false;
    bool showHelp = false;
    bool enableRealtime = false;
    int logInterval = 10;
};

Config parseArguments(int argc, char* argv[]) {
    Config config;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            config.showHelp = true;
            return config;
        } else if (arg == "-q" || arg == "--quiet") {
            config.quiet = true;
        } else if ((arg == "-i" || arg == "--iterations") && i + 1 < argc) {
            config.iterations = std::stoi(argv[++i]);
        } else if ((arg == "-s" || arg == "--small-blind") && i + 1 < argc) {
            config.smallBlind = std::stod(argv[++i]);
        } else if ((arg == "-b" || arg == "--big-blind") && i + 1 < argc) {
            config.bigBlind = std::stod(argv[++i]);
        } else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            config.outputPrefix = argv[++i];
        } else if (arg == "--realtime") {
            config.enableRealtime = true;
        } else if (arg == "--log-interval" && i + 1 < argc) {
            config.logInterval = std::stoi(argv[++i]);
        } else {
            throw std::invalid_argument("Unknown argument: " + arg);
        }
    }
    
    return config;
}

/**
 * @brief Validate configuration parameters
 */
void validateConfig(const Config& config) {
    if (config.iterations <= 0) {
        throw std::invalid_argument("Iterations must be positive");
    }
    
    if (config.smallBlind <= 0 || config.bigBlind <= 0) {
        throw std::invalid_argument("Blinds must be positive");
    }
    
    if (config.smallBlind >= config.bigBlind) {
        throw std::invalid_argument("Small blind must be less than big blind");
    }
    
    if (config.outputPrefix.empty()) {
        throw std::invalid_argument("Output prefix cannot be empty");
    }
}

/**
 * @brief Print configuration summary
 */
void printConfig(const Config& config) {
    std::cout << "=== All-or-Fold MCCFR Training ===\n";
    std::cout << "Configuration:\n";
    std::cout << "  Iterations:   " << config.iterations << "\n";
    std::cout << "  Small Blind:  " << config.smallBlind << "\n";
    std::cout << "  Big Blind:    " << config.bigBlind << "\n";
    std::cout << "  Output:       " << config.outputPrefix << "_*.txt\n";
    std::cout << "  Players:      " << aof::GameConfig::NUM_PLAYERS << "\n";
    std::cout << "  Starting BB:  " << aof::GameConfig::STARTING_STACK_BB << "\n";
    std::cout << "================================\n\n";
}

int main(int argc, char* argv[]) {
    try {
        // Parse command line arguments
        Config config = parseArguments(argc, argv);
        
        if (config.showHelp) {
            printUsage(argv[0]);
            return 0;
        }
        
        validateConfig(config);
        
        if (!config.quiet) {
            printConfig(config);
        }
        
        // Create game with specified parameters
        aof::Stakes stakes{config.smallBlind, config.bigBlind};
        aof::GameParameters gameParams;
        
        try {
            gameParams = aof::GameConfig::getGameParameters(stakes);
            if (!config.quiet) {
                std::cout << "Using rake structure for stakes " 
                          << config.smallBlind << "/" << config.bigBlind << ":\n";
                std::cout << "  Rake per hand: " << gameParams.rake_per_hand << "\n";
                std::cout << "  Jackpot fee: " << gameParams.jackpot_fee_per_hand << "\n";
                std::cout << "  Jackpot payout: " << (gameParams.jackpot_payout_percentage * 100) << "%\n\n";
            }
        } catch (const std::invalid_argument&) {
            // Use default parameters for unsupported stakes
            gameParams = {0.0, 0.0, 0.0};
            if (!config.quiet) {
                std::cout << "Using default parameters (no rake) for custom stakes.\n\n";
            }
        }
        
        aof::Game game(config.smallBlind, config.bigBlind, gameParams);
        
        // Create trainer
        mccfr::Trainer trainer(game);
        
        // Configure training
        mccfr::TrainingConfig trainingConfig;
        trainingConfig.iterations = config.iterations;
        trainingConfig.enableProgressOutput = !config.quiet;
        trainingConfig.outputPrefix = config.outputPrefix;
        trainingConfig.progressUpdateInterval = std::max(1, config.iterations / 100);
        
        // Configure utility tracking
        trainingConfig.enableUtilityTracking = !config.quiet;
        trainingConfig.utilityUpdateInterval = std::max(1, config.iterations / 20); // More frequent utility updates
        trainingConfig.showUtilityVariance = true;
        
        // Configure real-time visualization (no CSV logging)
        trainingConfig.enableDataLogging = false;  // Disabled - using web-based visualization only
        
        // Configure direct real-time visualization (no intermediate files)
        trainingConfig.realtimeConfig.enabled = config.enableRealtime;
        trainingConfig.realtimeConfig.updateInterval = std::max(1, std::min(config.logInterval, 5));
        trainingConfig.realtimeConfig.maxDataPoints = 2000;
        trainingConfig.realtimeConfig.showConsoleStats = !config.quiet;
        trainingConfig.realtimeConfig.windowTitle = "MCCFR Training Progress";
        
        if (config.enableRealtime) {
            // Optimize for real-time visualization
            trainingConfig.enableUtilityTracking = false; // Reduce console spam
            if (!config.quiet) {
                std::cout << "🚀 Web-based real-time visualization enabled!\n";
                std::cout << "Open http://localhost:8080 in your browser to view live plots!\n\n";
            }
        }
        
        // Train the model
        auto startTime = std::chrono::steady_clock::now();
        auto finalUtilities = trainer.train(trainingConfig);
        auto endTime = std::chrono::steady_clock::now();
        
        if (!config.quiet) {
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
            std::cout << "\n=== Training Summary ===\n";
            std::cout << "Total training time: " << duration.count() << " seconds\n";
            std::cout << "Information sets learned: " << trainer.getStats().informationSetsCount << "\n";
            std::cout << "Average utilities per player:\n";
            for (std::size_t i = 0; i < finalUtilities.size(); ++i) {
                std::cout << "  Player " << i << ": " << std::fixed 
                          << std::setprecision(6) << finalUtilities[i] << "\n";
            }
            std::cout << "========================\n";
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Use --help for usage information." << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }
}

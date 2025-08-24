#pragma once

#include "node.hpp"
#include "../aof/game.hpp"
#include "../aof/game_state.hpp"
#include "realtime_visualizer.hpp"
#include <unordered_map>
#include <memory>
#include <string>
#include <functional>
#include <chrono>

namespace mccfr {

/**
 * @brief Progress callback function type
 * @param iteration Current iteration number
 * @param totalIterations Total number of iterations
 * @param elapsedTime Time elapsed since training started
 */
using ProgressCallback = std::function<void(int iteration, int totalIterations, 
                                          std::chrono::milliseconds elapsedTime)>;

/**
 * @brief Configuration for MCCFR training
 */
struct TrainingConfig {
    int iterations = 1000000;              ///< Number of training iterations
    int progressUpdateInterval = 10000;    ///< How often to report progress
    bool enableProgressOutput = true;      ///< Whether to print progress
    std::string outputPrefix = "strategy"; ///< Prefix for output files
    
    // Utility tracking options
    bool enableUtilityTracking = true;     ///< Whether to track and display utilities
    int utilityUpdateInterval = 10000;     ///< How often to report utility statistics
    bool showUtilityVariance = true;       ///< Whether to calculate and show variance
    
    // Data logging options
    bool enableDataLogging = true;         ///< Whether to log training data
    std::string dataLogFile = "training_data.csv";  ///< Data log filename
    int dataLogInterval = 10;              ///< How often to log data points
    bool enableRealtimeVisualization = false;  ///< Enable real-time visualization mode
    
    // Direct real-time visualization (no intermediate files)
    RealtimeVisualizer::Config realtimeConfig;  ///< Real-time visualizer configuration
    
    /// Optional callback for custom progress handling
    ProgressCallback progressCallback = nullptr;
};

/**
 * @brief Monte Carlo Counterfactual Regret Minimization trainer
 * 
 * Implements the MCCFR algorithm to learn Nash equilibrium strategies
 * for All-or-Fold poker through self-play.
 */
class Trainer {
public:
    /**
     * @brief Construct trainer for given game
     * @param game Game instance to train on
     */
    explicit Trainer(const aof::Game& game);

    /**
     * @brief Train using MCCFR algorithm
     * @param config Training configuration
     * @return Final average utilities for each player
     */
    aof::PlayerUtilities train(const TrainingConfig& config = TrainingConfig());

    /**
     * @brief Get learned strategy for an information set
     * @param infoSet Information set identifier
     * @return Average strategy, or empty vector if not found
     */
    std::vector<double> getStrategy(const std::string& infoSet) const;

    /**
     * @brief Get all learned strategies
     * @return Map from information sets to average strategies
     */
    std::unordered_map<std::string, std::vector<double>> getAllStrategies() const;

    /**
     * @brief Save strategies to file
     * @param filename Output filename
     * @param includeVisitCounts Whether to include visit count statistics
     */
    void saveStrategies(const std::string& filename, bool includeVisitCounts = true) const;

    /**
     * @brief Load strategies from file
     * @param filename Input filename
     * @return True if successful
     */
    bool loadStrategies(const std::string& filename);

    /**
     * @brief Reset all learned strategies
     */
    void reset();

    /**
     * @brief Get training statistics
     */
    struct TrainingStats {
        int totalIterations = 0;
        std::chrono::milliseconds totalTime{0};
        std::size_t informationSetsCount = 0;
        aof::PlayerUtilities finalUtilities;
    };
    
    const TrainingStats& getStats() const noexcept { return stats_; }

private:
    const aof::Game& game_;
    std::unordered_map<std::string, Node> nodeMap_;
    TrainingStats stats_;
    std::unique_ptr<RealtimeVisualizer> visualizer_;

    /**
     * @brief Core MCCFR recursive function
     * @param state Current game state
     * @param player Player being trained (0-3)
     * @param reachProb Reach probabilities for all players
     * @return Expected utility for the training player
     */
    double mccfr(std::unique_ptr<aof::GameState> state, 
                int player, 
                std::vector<double>& reachProb);

    /**
     * @brief Update progress display
     * @param iteration Current iteration
     * @param totalIterations Total iterations
     * @param startTime Training start time
     * @param config Training configuration
     */
    void updateProgress(int iteration, int totalIterations,
                       std::chrono::steady_clock::time_point startTime,
                       const TrainingConfig& config) const;

    /**
     * @brief Format time duration for display
     * @param duration Duration to format
     * @return Human-readable time string
     */
    std::string formatDuration(std::chrono::milliseconds duration) const;

    /**
     * @brief Calculate variance of player utilities
     * @param utilities Player utilities vector
     * @return Variance value
     */
    double calculateUtilityVariance(const aof::PlayerUtilities& utilities) const;

    /**
     * @brief Print utility statistics with position labels
     * @param utilities Current average utilities
     * @param iteration Current iteration number
     * @param variance Utility variance
     */
    void printUtilityStats(const aof::PlayerUtilities& utilities, 
                          int iteration, 
                          double variance) const;

    /**
     * @brief Get exact utilities from a complete game simulation
     * @param state Initial game state
     * @return Exact utilities for all players (guaranteed zero-sum)
     */
    aof::PlayerUtilities getExactUtilities(std::unique_ptr<aof::GameState> state);
};

} // namespace mccfr

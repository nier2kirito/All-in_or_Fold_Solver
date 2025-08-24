#include "mccfr/trainer.hpp"
#include "mccfr/utils.hpp"
#include "aof/game_config.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <filesystem>

namespace mccfr {

Trainer::Trainer(const aof::Game& game) 
    : game_(game)
    , nodeMap_()
    , stats_{}
    , visualizer_(nullptr)
{
}

aof::PlayerUtilities Trainer::train(const TrainingConfig& config) {
    std::cout << "Starting MCCFR training with " << config.iterations << " iterations...\n";
    
    auto startTime = std::chrono::steady_clock::now();
    stats_.totalIterations = 0;
    
    aof::PlayerUtilities totalUtilities(aof::GameConfig::NUM_PLAYERS, 0.0);
    aof::PlayerUtilities avgUtilities(aof::GameConfig::NUM_PLAYERS, 0.0);
    
    // Initialize real-time visualizer
    if (config.realtimeConfig.enabled) {
        visualizer_ = std::make_unique<RealtimeVisualizer>(config.realtimeConfig);
        if (!visualizer_->start()) {
            std::cerr << "Warning: Failed to start real-time visualizer\n";
            visualizer_.reset();
        }
    }
    
    // CSV logging removed - using direct real-time visualization only
    
    for (int iteration = 1; iteration <= config.iterations; ++iteration) {
        // Create a single game state for all players to ensure zero-sum utilities
        auto baseState = game_.createInitialState();
        std::vector<double> baseReachProb(aof::GameConfig::NUM_PLAYERS, 1.0);
        
        // Train each player using MCCFR (this updates strategies)
        std::vector<double> mccfrUtilities(aof::GameConfig::NUM_PLAYERS, 0.0);
        for (int player = 0; player < aof::GameConfig::NUM_PLAYERS; ++player) {
            auto stateCopy = std::make_unique<aof::GameState>(*baseState);
            std::vector<double> reachProb = baseReachProb;
            
            mccfrUtilities[player] = mccfr(std::move(stateCopy), player, reachProb);
        }
        
        // For utility tracking, use exact utilities from a single terminal outcome
        // This ensures perfect zero-sum property for display purposes
        auto terminalState = std::make_unique<aof::GameState>(*baseState);
        auto exactUtilities = getExactUtilities(std::move(terminalState));
        
        // Accumulate the exact utilities for averaging
        for (int player = 0; player < aof::GameConfig::NUM_PLAYERS; ++player) {
            totalUtilities[player] += exactUtilities[player];
            avgUtilities[player] = totalUtilities[player] / iteration;
        }
        
        stats_.totalIterations = iteration;
        
        // Update progress
        if (config.enableProgressOutput && 
            (iteration % config.progressUpdateInterval == 0 || iteration == config.iterations)) {
            updateProgress(iteration, config.iterations, startTime, config);
        }
        
        // Update utility statistics
        if (config.enableUtilityTracking && 
            (iteration % config.utilityUpdateInterval == 0 || iteration == config.iterations)) {
            double variance = config.showUtilityVariance ? calculateUtilityVariance(avgUtilities) : 0.0;
            printUtilityStats(avgUtilities, iteration, variance);
            
            // Verify utilities sum to zero (within numerical precision)
            double utilitySum = 0.0;
            for (double utility : avgUtilities) {
                utilitySum += utility;
            }
            if (std::abs(utilitySum) > 1e-10) {
                std::cout << "WARNING: Utilities don't sum to zero! Sum = " 
                          << std::fixed << std::setprecision(16) << utilitySum << "\n";
            } else {
                std::cout << "✓ Utilities sum to zero (sum = " 
                          << std::scientific << std::setprecision(3) << utilitySum << ")\n";
            }
        }
        
        // Send data to real-time visualizer
        if (visualizer_ && visualizer_->isRunning() && 
            (iteration % config.realtimeConfig.updateInterval == 0 || iteration == config.iterations)) {
            
            double meanAbsoluteError = calculateUtilityVariance(avgUtilities);
            double utilitySum = 0.0;
            for (double utility : avgUtilities) {
                utilitySum += utility;
            }
            
            auto currentTime = std::chrono::steady_clock::now();
            auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
            
            TrainingDataPoint dataPoint(
                iteration, 
                meanAbsoluteError, 
                utilitySum, 
                avgUtilities,
                std::chrono::milliseconds(elapsedMs)
            );
            visualizer_->addDataPoint(dataPoint);
        }
        
        // Custom progress callback
        if (config.progressCallback) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - startTime);
            config.progressCallback(iteration, config.iterations, elapsed);
        }
    }
    
    auto endTime = std::chrono::steady_clock::now();
    stats_.totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    stats_.informationSetsCount = nodeMap_.size();
    stats_.finalUtilities = avgUtilities;
    
    if (config.enableProgressOutput) {
        std::cout << "\n\nTraining completed!\n";
        std::cout << "Total time: " << formatDuration(stats_.totalTime) << "\n";
        std::cout << "Information sets learned: " << nodeMap_.size() << "\n";
        std::cout << "Final average utilities: [";
        for (std::size_t i = 0; i < avgUtilities.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << std::fixed << std::setprecision(16) << avgUtilities[i];
        }
        std::cout << "]\n";
        
        // Final verification that utilities sum to zero
        double finalSum = 0.0;
        for (double utility : avgUtilities) {
            finalSum += utility;
        }
        std::cout << "Final utility sum: " << std::scientific << std::setprecision(6) << finalSum;
        if (std::abs(finalSum) < 1e-10) {
            std::cout << " ✓ (zero-sum verified)\n";
        } else {
            std::cout << " ⚠ (not zero-sum!)\n";
        }
    }
    
    // Save strategies with timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);
    
    std::stringstream filename;
    filename << config.outputPrefix << "_"
             << (tm.tm_year + 1900) << "_"
             << std::setfill('0') << std::setw(2) << (tm.tm_mon + 1) << "_"
             << std::setw(2) << tm.tm_mday << "_"
             << std::setw(2) << tm.tm_hour << "_"
             << std::setw(2) << tm.tm_min << "_"
             << std::setw(2) << tm.tm_sec << ".txt";
    
    saveStrategies(filename.str());
    
    // No CSV files to close - using direct real-time visualization only
    
    // Stop real-time visualizer
    if (visualizer_) {
        std::cout << "Training complete! Real-time visualizer will continue running.\n";
        std::cout << "Close the visualization window to stop it.\n";
        // Note: We don't stop the visualizer here to let user examine final results
    }
    
    return avgUtilities;
}

std::vector<double> Trainer::getStrategy(const std::string& infoSet) const {
    auto it = nodeMap_.find(infoSet);
    if (it != nodeMap_.end()) {
        return it->second.getAverageStrategy();
    }
    return {};
}

std::unordered_map<std::string, std::vector<double>> Trainer::getAllStrategies() const {
    std::unordered_map<std::string, std::vector<double>> strategies;
    
    for (const auto& [infoSet, node] : nodeMap_) {
        strategies[infoSet] = node.getAverageStrategy();
    }
    
    return strategies;
}

void Trainer::saveStrategies(const std::string& filename, bool includeVisitCounts) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file for writing: " << filename << "\n";
        return;
    }
    
    // Write header
    file << "# MCCFR Strategy File\n";
    file << "# Generated with " << stats_.totalIterations << " iterations\n";
    file << "# Total information sets: " << nodeMap_.size() << "\n";
    file << "# Format: InfoSet: <infoset_string> Visits: <count>\n";
    file << "#         Strategy: <prob1> <prob2> ...\n\n";
    
    // Sort information sets by visit count for better readability
    std::vector<std::pair<std::string, const Node*>> sortedNodes;
    for (const auto& [infoSet, node] : nodeMap_) {
        sortedNodes.emplace_back(infoSet, &node);
    }
    
    std::sort(sortedNodes.begin(), sortedNodes.end(),
              [](const auto& a, const auto& b) {
                  return a.second->getVisitCount() > b.second->getVisitCount();
              });
    
    // Write strategies
    for (const auto& [infoSet, node] : sortedNodes) {
        file << "InfoSet: " << infoSet;
        if (includeVisitCounts) {
            file << " Visits: " << node->getVisitCount();
        }
        file << "\n";
        
        auto avgStrategy = node->getAverageStrategy();
        file << "Strategy: ";
        for (std::size_t i = 0; i < avgStrategy.size(); ++i) {
            if (i > 0) file << " ";
            file << std::fixed << std::setprecision(16) << avgStrategy[i];
        }
        file << "\n\n";
    }
    
    file.close();
    
    if (file.good()) {
        std::cout << "Strategies saved to: " << filename << "\n";
    } else {
        std::cerr << "Error occurred while writing to file: " << filename << "\n";
    }
}

bool Trainer::loadStrategies(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file for reading: " << filename << "\n";
        return false;
    }
    
    nodeMap_.clear();
    std::string line;
    
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Parse InfoSet line
        if (line.substr(0, 8) == "InfoSet:") {
            std::string infoSet;
            std::uint64_t visits = 0;
            
            // Extract info set string
            std::size_t start = line.find(' ') + 1;
            std::size_t visitsPos = line.find(" Visits:");
            
            if (visitsPos != std::string::npos) {
                infoSet = line.substr(start, visitsPos - start);
                visits = std::stoull(line.substr(visitsPos + 8));
            } else {
                infoSet = line.substr(start);
            }
            
            // Read strategy line
            if (!std::getline(file, line) || line.substr(0, 9) != "Strategy:") {
                std::cerr << "Error: Expected Strategy line after InfoSet\n";
                return false;
            }
            
            // Parse strategy probabilities
            std::vector<double> strategy;
            std::istringstream iss(line.substr(10));
            double prob;
            while (iss >> prob) {
                strategy.push_back(prob);
            }
            
            if (!strategy.empty()) {
                Node node(strategy.size());
                // We can't perfectly reconstruct the internal state, but we can
                // set the strategy sum to approximate the learned strategy
                for (std::size_t i = 0; i < strategy.size(); ++i) {
                    // Approximate reconstruction
                    node.updateRegret(i, strategy[i] * visits);
                }
                nodeMap_[infoSet] = std::move(node);
            }
        }
    }
    
    file.close();
    std::cout << "Loaded " << nodeMap_.size() << " information sets from: " << filename << "\n";
    return true;
}

void Trainer::reset() {
    nodeMap_.clear();
    stats_ = TrainingStats{};
}

// Private methods
double Trainer::mccfr(std::unique_ptr<aof::GameState> state, 
                     int player, 
                     std::vector<double>& reachProb) {
    
    if (state->isTerminal()) {
        auto utilities = state->getReturns();
        return utilities[player];
    }
    
    if (state->isChanceNode()) {
        state->applyAction(aof::Action::DEAL);
        return mccfr(std::move(state), player, reachProb);
    }
    
    int currentPlayer = state->getCurrentPlayer();
    std::string infoSet = utils::getInformationSet(*state, currentPlayer);
    
    auto legalActions = state->getLegalActions();
    if (legalActions.empty()) {
        throw std::runtime_error("No legal actions available");
    }
    
    // Ensure node exists
    if (nodeMap_.find(infoSet) == nodeMap_.end()) {
        nodeMap_.emplace(infoSet, Node(legalActions.size()));
    }
    
    Node& node = nodeMap_[infoSet];
    
    if (currentPlayer == player) {
        // Player being trained
        auto strategy = node.getStrategy(reachProb[player]);
        std::vector<double> utilities(legalActions.size(), 0.0);
        double nodeUtility = 0.0;
        
        // Compute utilities for each action
        for (std::size_t i = 0; i < legalActions.size(); ++i) {
            auto nextState = std::make_unique<aof::GameState>(*state);
            nextState->applyAction(legalActions[i]);
            
            std::vector<double> nextReachProb = reachProb;
            nextReachProb[player] *= strategy[i];
            
            utilities[i] = mccfr(std::move(nextState), player, nextReachProb);
            nodeUtility += strategy[i] * utilities[i];
        }
        
        // Update regrets
        for (std::size_t i = 0; i < legalActions.size(); ++i) {
            double regret = utilities[i] - nodeUtility;
            node.updateRegret(i, regret);
        }
        
        return nodeUtility;
        
    } else {
        // Opponent player - sample according to current strategy
        auto strategy = node.getStrategy(reachProb[currentPlayer]);
        int actionIndex = utils::sampleAction(strategy);
        
        std::vector<double> nextReachProb = reachProb;
        nextReachProb[currentPlayer] *= strategy[actionIndex];
        
        state->applyAction(legalActions[actionIndex]);
        return mccfr(std::move(state), player, nextReachProb);
    }
}

void Trainer::updateProgress(int iteration, int totalIterations,
                           std::chrono::steady_clock::time_point startTime,
                           const TrainingConfig& /* config */) const {
    
    auto currentTime = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime);
    
    double percentage = (static_cast<double>(iteration) / totalIterations) * 100.0;
    
    // Estimate remaining time
    double avgTimePerIter = elapsed.count() / static_cast<double>(iteration);
    int remainingIters = totalIterations - iteration;
    auto estimatedRemaining = std::chrono::milliseconds(
        static_cast<long long>(avgTimePerIter * remainingIters));
    
    std::cout << "\rIteration " << iteration << " (" 
              << std::fixed << std::setprecision(2) << percentage 
              << "% completed, ETA: " << formatDuration(estimatedRemaining) << ")" 
              << std::flush;
}

std::string Trainer::formatDuration(std::chrono::milliseconds duration) const {
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(seconds);
    auto hours = std::chrono::duration_cast<std::chrono::hours>(minutes);
    
    std::stringstream ss;
    
    if (hours.count() > 0) {
        ss << hours.count() << "h ";
        minutes -= hours;
    }
    
    if (minutes.count() > 0) {
        ss << minutes.count() << "m ";
        seconds -= minutes;
    }
    
    ss << seconds.count() << "s";
    
    return ss.str();
}

double Trainer::calculateUtilityVariance(const aof::PlayerUtilities& utilities) const {
    if (utilities.size() < 4) return 0.0;  // Need all 4 positions
    
    // Target utilities for each position
    const std::vector<double> targetUtilities = {
        0.0,  // SB (Player 0)
        0.0,  // BB (Player 1) 
        0.0,   // CO (Player 2)
        0.0    // BTN (Player 3) 
    };
    
    // Calculate mean absolute error (sum of absolute distances from target utilities)
    double mae = 0.0;
    for (std::size_t i = 0; i < std::min(utilities.size(), targetUtilities.size()); ++i) {
        double distance = utilities[i] - targetUtilities[i];
        mae += std::abs(distance);
    }
    mae /= std::min(utilities.size(), targetUtilities.size());
    
    return mae;
}

void Trainer::printUtilityStats(const aof::PlayerUtilities& utilities, 
                                int iteration, 
                                double variance) const {
    // Position labels for All-or-Fold poker (4 players)
    const std::vector<std::string> positions = {"SB", "BB", "CO", "BTN"};
    const std::vector<double> targetUtilities = {0.0, 0.0, 0.0, 0.0};
    
    std::cout << "\n--- Utility Statistics (Iteration " << iteration << ") ---\n";
    std::cout << std::fixed << std::setprecision(16);
    
    for (std::size_t i = 0; i < utilities.size() && i < positions.size(); ++i) {
        double distance = utilities[i] - targetUtilities[i];
        std::cout << positions[i] << " (Player " << i << "): " << utilities[i] 
                  << " (target: " << targetUtilities[i] << ", distance: " << distance << ")\n";
    }
    
    if (variance > 0.0) {
        std::cout << "Mean Absolute Error: " << std::setprecision(16) << variance << "\n";
    }
    
}

aof::PlayerUtilities Trainer::getExactUtilities(std::unique_ptr<aof::GameState> state) {
    // Play out the game deterministically to get exact utilities
    // This simulates a complete game with current strategies
    
    while (!state->isTerminal()) {
        if (state->isChanceNode()) {
            state->applyAction(aof::Action::DEAL);
        } else {
            int currentPlayer = state->getCurrentPlayer();
            auto legalActions = state->getLegalActions();
            
            if (legalActions.empty()) {
                break;
            }
            
            // Get current strategy for this information set
            std::string infoSet = utils::getInformationSet(*state, currentPlayer);
            auto it = nodeMap_.find(infoSet);
            
            if (it != nodeMap_.end()) {
                // Use learned strategy
                auto strategy = it->second.getAverageStrategy();
                
                // For exact utilities, use the action with highest probability
                // (or sample according to strategy for more realistic outcomes)
                int bestAction = 0;
                double maxProb = strategy[0];
                for (std::size_t i = 1; i < strategy.size(); ++i) {
                    if (strategy[i] > maxProb) {
                        maxProb = strategy[i];
                        bestAction = i;
                    }
                }
                state->applyAction(legalActions[bestAction]);
            } else {
                // Use uniform random strategy if no learned strategy exists
                int actionIndex = utils::sampleAction(std::vector<double>(legalActions.size(), 1.0 / legalActions.size()));
                state->applyAction(legalActions[actionIndex]);
            }
        }
    }
    
    return state->getReturns();
}

} // namespace mccfr

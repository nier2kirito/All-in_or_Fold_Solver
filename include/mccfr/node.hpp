#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace mccfr {

/**
 * @brief Represents an information set node in the MCCFR algorithm
 * 
 * Each node tracks regret sums and cumulative strategies for a specific
 * information set (game situation from a player's perspective).
 */
class Node {
public:
    /**
     * @brief Construct node with specified number of actions
     * @param numActions Number of legal actions at this information set
     */
    explicit Node(int numActions);

    /**
     * @brief Default constructor (creates node with 3 actions for AoF)
     */
    Node();

    /**
     * @brief Get current strategy based on regret matching
     * @param realizationWeight Weight for updating cumulative strategy
     * @return Current mixed strategy (probability distribution over actions)
     */
    std::vector<double> getStrategy(double realizationWeight);

    /**
     * @brief Get average strategy over all iterations
     * @return Time-averaged strategy
     */
    std::vector<double> getAverageStrategy() const;

    /**
     * @brief Update regret for a specific action
     * @param action Action index
     * @param regret Regret value to add
     */
    void updateRegret(int action, double regret);

    /**
     * @brief Get number of times this node was visited
     */
    std::uint64_t getVisitCount() const noexcept { return visitCount_; }

    /**
     * @brief Reset all statistics (for testing/retraining)
     */
    void reset();

    // Direct access for serialization/analysis
    const std::vector<double>& getRegretSum() const noexcept { return regretSum_; }
    const std::vector<double>& getStrategySum() const noexcept { return strategySum_; }

private:
    std::vector<double> regretSum_;     ///< Cumulative regret for each action
    std::vector<double> strategy_;      ///< Current strategy (working buffer)
    std::vector<double> strategySum_;   ///< Cumulative strategy for averaging
    std::uint64_t visitCount_;          ///< Number of times node was visited

    /**
     * @brief Normalize strategy using regret matching
     */
    void normalizeStrategy();
};

} // namespace mccfr


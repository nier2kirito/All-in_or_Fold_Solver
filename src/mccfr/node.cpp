#include "mccfr/node.hpp"
#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace mccfr {

Node::Node(int numActions)
    : regretSum_(numActions, 0.0)
    , strategy_(numActions, 0.0)
    , strategySum_(numActions, 0.0)
    , visitCount_(0)
{
    if (numActions <= 0) {
        throw std::invalid_argument("Number of actions must be positive");
    }
}

Node::Node() : Node(3) {  // Default to 3 actions for All-or-Fold (FOLD, ALL_IN, DEAL)
}

std::vector<double> Node::getStrategy(double realizationWeight) {
    ++visitCount_;
    
    normalizeStrategy();
    
    // Update cumulative strategy
    for (std::size_t i = 0; i < strategy_.size(); ++i) {
        strategySum_[i] += realizationWeight * strategy_[i];
    }
    
    return strategy_;
}

std::vector<double> Node::getAverageStrategy() const {
    std::vector<double> avgStrategy(strategySum_.size(), 0.0);
    
    double sum = std::accumulate(strategySum_.begin(), strategySum_.end(), 0.0);
    
    if (sum > 0) {
        for (std::size_t i = 0; i < avgStrategy.size(); ++i) {
            avgStrategy[i] = strategySum_[i] / sum;
        }
    } else {
        // Uniform distribution if no strategy accumulated
        double uniform = 1.0 / avgStrategy.size();
        std::fill(avgStrategy.begin(), avgStrategy.end(), uniform);
    }
    
    return avgStrategy;
}

void Node::updateRegret(int action, double regret) {
    if (action < 0 || action >= static_cast<int>(regretSum_.size())) {
        throw std::invalid_argument("Invalid action index");
    }
    
    regretSum_[action] += regret;
}

void Node::reset() {
    std::fill(regretSum_.begin(), regretSum_.end(), 0.0);
    std::fill(strategy_.begin(), strategy_.end(), 0.0);
    std::fill(strategySum_.begin(), strategySum_.end(), 0.0);
    visitCount_ = 0;
}

void Node::normalizeStrategy() {
    // Use regret matching: positive regrets become strategy weights
    double normalizingSum = 0.0;
    
    for (std::size_t i = 0; i < regretSum_.size(); ++i) {
        strategy_[i] = std::max(regretSum_[i], 0.0);
        normalizingSum += strategy_[i];
    }
    
    // Normalize to probability distribution
    if (normalizingSum > 0) {
        for (std::size_t i = 0; i < strategy_.size(); ++i) {
            strategy_[i] /= normalizingSum;
        }
    } else {
        // Uniform distribution if all regrets are non-positive
        double uniform = 1.0 / strategy_.size();
        std::fill(strategy_.begin(), strategy_.end(), uniform);
    }
}

} // namespace mccfr

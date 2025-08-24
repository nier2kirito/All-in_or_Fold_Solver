#include "aof/game_config.hpp"
#include <unordered_map>
#include <algorithm>

namespace aof {

// Hash function for Stakes pairs
std::size_t GameConfig::StakesHash::operator()(const Stakes& stakes) const noexcept {
    // Combine hash of both values
    std::hash<double> hasher;
    return hasher(stakes.first) ^ (hasher(stakes.second) << 1);
}

// Static lookup table for game parameters
const std::unordered_map<Stakes, GameParameters, GameConfig::StakesHash> 
GameConfig::STAKES_PARAMETERS = {
    {{0.05, 0.10}, {0.02, 0.02, 0.00005}},    // 5c/10c
    {{0.10, 0.20}, {0.03, 0.03, 0.0001}},     // 10c/20c  
    {{0.10, 0.25}, {0.04, 0.04, 0.0001}},     // 10c/25c
    {{0.20, 0.40}, {0.05, 0.05, 0.0002}},     // 20c/40c
    {{0.25, 0.50}, {0.06, 0.06, 0.0002}},     // 25c/50c
    {{0.50, 1.00}, {0.05, 0.05, 0.0005}},     // 50c/$1
    {{1.00, 2.00}, {0.05, 0.05, 0.001}},      // $1/$2
    {{2.00, 4.00}, {0.05, 0.05, 0.0015}},     // $2/$4
    {{5.00, 10.00}, {0.05, 0.05, 0.0025}},    // $5/$10
    {{10.00, 20.00}, {0.05, 0.05, 0.005}},    // $10/$20
    {{25.00, 50.00}, {0.05, 0.05, 0.0075}},   // $25/$50
    {{50.00, 100.00}, {0.05, 0.05, 0.01}},    // $50/$100
    {{100.00, 200.00}, {0.025, 0.025, 0.01}}, // $100/$200
    {{200.00, 400.00}, {0.025, 0.025, 0.0125}}, // $200/$400
    {{500.00, 1000.00}, {0.025, 0.025, 0.015}}, // $500/$1000
    {{1000.00, 2000.00}, {0.025, 0.025, 0.02}}  // $1000/$2000
};

GameParameters GameConfig::getGameParameters(const Stakes& stakes) {
    auto it = STAKES_PARAMETERS.find(stakes);
    if (it == STAKES_PARAMETERS.end()) {
        throw std::invalid_argument("Unsupported stakes: " + 
                                  std::to_string(stakes.first) + "/" + 
                                  std::to_string(stakes.second));
    }
    return it->second;
}

std::vector<Stakes> GameConfig::getSupportedStakes() {
    std::vector<Stakes> stakes;
    stakes.reserve(STAKES_PARAMETERS.size());
    
    for (const auto& pair : STAKES_PARAMETERS) {
        stakes.push_back(pair.first);
    }
    
    // Sort by big blind amount
    std::sort(stakes.begin(), stakes.end(), 
              [](const Stakes& a, const Stakes& b) {
                  return a.second < b.second;
              });
    
    return stakes;
}

} // namespace aof

#include "mccfr/utils.hpp"
#include "aof/card.hpp"
#include <sstream>
#include <random>
#include <thread>
#include <unordered_map>

namespace mccfr {
namespace utils {

std::string getInformationSet(const aof::GameState& state, int player) {
    std::stringstream ss;
    ss << "P" << player << ":";
    
    // Add status of other players based on position
    if (player <= 1) {
        // Players 0 and 1 (blinds) see all other players' statuses
        for (int p = 0; p < 4; ++p) {
            if (p != player) {
                ss << "[P" << p;
                const auto& folded = state.getFoldedPlayers();
                const auto& allIn = state.getAllInPlayers();
                
                if (folded[p]) {
                    ss << ":F";  // Folded
                } else if (allIn.count(p) > 0) {
                    ss << ":A";  // All-in
                } else {
                    ss << ":P";  // Playing/Pending
                }
                ss << "]";
            }
        }
    } else {
        // Players 2 and 3 only see previous players' actions
        for (int p = 0; p < player; ++p) {
            ss << "[P" << p;
            const auto& folded = state.getFoldedPlayers();
            const auto& allIn = state.getAllInPlayers();
            
            if (folded[p]) {
                ss << ":F";
            } else if (allIn.count(p) > 0) {
                ss << ":A";
            } else {
                ss << ":P";
            }
            ss << "]";
        }
    }
    
    // Add abstracted hole cards
    const auto& holeCards = state.getHoleCards();
    if (holeCards.size() >= static_cast<std::size_t>((player + 1) * 2)) {
        int firstCardIdx = player * 2;
        int secondCardIdx = player * 2 + 1;
        
        std::string abstractedCards = abstractHoleCards(holeCards[firstCardIdx], 
                                                       holeCards[secondCardIdx]);
        ss << abstractedCards << " ";
    }
    
    // Add pot information
    ss << "Pot:" << state.getPot();
    
    return ss.str();
}

int sampleAction(const std::vector<double>& strategy, std::mt19937& rng) {
    if (strategy.empty()) {
        throw std::invalid_argument("Strategy cannot be empty");
    }
    
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double r = dist(rng);
    
    double cumulative = 0.0;
    for (std::size_t i = 0; i < strategy.size(); ++i) {
        cumulative += strategy[i];
        if (r < cumulative) {
            return static_cast<int>(i);
        }
    }
    
    // Fallback to last action (handles floating point precision issues)
    return static_cast<int>(strategy.size() - 1);
}

int sampleAction(const std::vector<double>& strategy) {
    // Thread-local random number generator
    thread_local std::mt19937 rng(std::random_device{}());
    return sampleAction(strategy, rng);
}

std::string abstractHoleCards(const aof::Card& card1, const aof::Card& card2) {
    // Get rank values for comparison
    int rank1 = card1.getRankValue();
    int rank2 = card2.getRankValue();
    
    // Order cards by rank (higher first)
    const aof::Card* highCard = &card1;
    const aof::Card* lowCard = &card2;
    
    if (rank1 < rank2) {
        std::swap(highCard, lowCard);
    }
    
    std::stringstream ss;
    ss << highCard->getRank();
    
    // For pairs, just show the rank twice
    if (highCard->getRankValue() == lowCard->getRankValue()) {
        ss << lowCard->getRank();
    } else {
        ss << lowCard->getRank();
        // Add suited/offsuit indicator
        ss << (aof::card_utils::areSuited(*highCard, *lowCard) ? "s" : "o");
    }
    
    return ss.str();
}

std::string getActionHistory(const aof::GameState& state, int upToPlayer) {
    std::stringstream ss;
    
    const auto& folded = state.getFoldedPlayers();
    const auto& allIn = state.getAllInPlayers();
    
    for (int p = 0; p < upToPlayer && p < 4; ++p) {
        if (p > 0) ss << ",";
        
        if (folded[p]) {
            ss << "F";
        } else if (allIn.count(p) > 0) {
            ss << "A";
        } else {
            ss << "P";
        }
    }
    
    return ss.str();
}

} // namespace utils
} // namespace mccfr


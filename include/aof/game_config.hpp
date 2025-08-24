#pragma once

#include "types.hpp"
#include <unordered_map>
#include <stdexcept>
#include <vector>
#include <algorithm>

namespace aof {

/**
 * @brief Game configuration constants and utilities
 */
class GameConfig {
public:
    // Game constants
    static constexpr double STARTING_STACK_BB = 8.0;  ///< Starting stack in big blinds
    static constexpr int NUM_PLAYERS = 4;             ///< Number of players in AoF
    static constexpr int HOLE_CARDS_PER_PLAYER = 2;   ///< Hole cards per player
    static constexpr int COMMUNITY_CARDS = 5;         ///< Community cards dealt
    static constexpr int DECK_SIZE = 52;              ///< Standard deck size

    /// Custom hash function for Stakes pairs
    struct StakesHash {
        std::size_t operator()(const Stakes& stakes) const noexcept;
    };

    /**
     * @brief Get game parameters for given stakes
     * @param stakes Pair of (small_blind, big_blind)
     * @return GameParameters with rake and jackpot settings
     * @throws std::invalid_argument if stakes are not supported
     */
    static GameParameters getGameParameters(const Stakes& stakes);

    /**
     * @brief Get all supported stakes
     * @return Vector of supported stakes pairs
     */
    static std::vector<Stakes> getSupportedStakes();

private:
    /// Lookup table for stake-based game parameters
    static const std::unordered_map<Stakes, GameParameters, StakesHash> STAKES_PARAMETERS;
};

} // namespace aof

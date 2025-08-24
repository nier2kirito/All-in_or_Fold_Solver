#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace aof {

/**
 * @brief Actions available in All-or-Fold poker
 */
enum class Action : std::uint8_t {
    FOLD = 0,    ///< Player folds their hand
    ALL_IN = 1,  ///< Player goes all-in with remaining chips
    DEAL = 2     ///< Chance node - deal cards
};

/**
 * @brief Poker hand rankings (higher values are better)
 */
enum class HandRank : std::uint8_t {
    HIGH_CARD = 0,
    PAIR = 1,
    TWO_PAIR = 2,
    THREE_OF_A_KIND = 3,
    STRAIGHT = 4,
    FLUSH = 5,
    FULL_HOUSE = 6,
    FOUR_OF_A_KIND = 7,
    STRAIGHT_FLUSH = 8
};

/**
 * @brief Game configuration parameters
 */
struct GameParameters {
    double rake_per_hand;              ///< Rake amount per hand
    double jackpot_fee_per_hand;       ///< Jackpot fee per hand
    double jackpot_payout_percentage;  ///< Jackpot payout percentage
};

/**
 * @brief Stakes configuration (small blind, big blind)
 */
using Stakes = std::pair<double, double>;

/**
 * @brief Player utilities/payoffs
 */
using PlayerUtilities = std::vector<double>;

/**
 * @brief Action probabilities for a strategy
 */
using Strategy = std::vector<double>;

} // namespace aof


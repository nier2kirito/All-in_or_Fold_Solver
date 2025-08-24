#pragma once

#include "../aof/game_state.hpp"
#include <string>
#include <vector>
#include <random>

namespace mccfr {

/**
 * @brief Utility functions for MCCFR algorithm
 */
namespace utils {

/**
 * @brief Generate information set string for a player
 * 
 * Creates a unique identifier for the game situation from the perspective
 * of the specified player, including:
 * - Player's hole cards (abstracted)
 * - Actions of previous players
 * - Current pot size
 * 
 * @param state Current game state
 * @param player Player index (0-3)
 * @return Information set identifier string
 */
std::string getInformationSet(const aof::GameState& state, int player);

/**
 * @brief Sample an action according to strategy probabilities
 * @param strategy Probability distribution over actions
 * @param rng Random number generator
 * @return Sampled action index
 */
int sampleAction(const std::vector<double>& strategy, std::mt19937& rng);

/**
 * @brief Sample an action using thread-local RNG
 * @param strategy Probability distribution over actions  
 * @return Sampled action index
 */
int sampleAction(const std::vector<double>& strategy);

/**
 * @brief Create abstracted representation of hole cards
 * 
 * Reduces the complexity of information sets by grouping similar hands:
 * - Orders cards by rank (higher first)
 * - Indicates if suited or offsuit
 * 
 * @param card1 First hole card
 * @param card2 Second hole card
 * @return Abstracted string representation (e.g., "As", "Ko", "77")
 */
std::string abstractHoleCards(const aof::Card& card1, const aof::Card& card2);

/**
 * @brief Get player action history as string
 * @param state Current game state
 * @param upToPlayer Only include players up to this index
 * @return String encoding of actions taken
 */
std::string getActionHistory(const aof::GameState& state, int upToPlayer);

} // namespace utils

} // namespace mccfr


#pragma once

#include "types.hpp"
#include "game_state.hpp"
#include <vector>
#include <memory>

namespace aof {

/**
 * @brief Main All-or-Fold poker game class
 */
class Game {
public:
    /**
     * @brief Construct game with blinds and parameters
     * @param smallBlind Small blind amount
     * @param bigBlind Big blind amount  
     * @param gameParams Game parameters (rake, jackpot, etc.)
     * @param initialStacksBB Starting stacks in big blinds (default: 8BB each)
     */
    Game(double smallBlind, 
         double bigBlind,
         const GameParameters& gameParams = {},
         const std::vector<double>& initialStacksBB = {});

    /**
     * @brief Create a new initial game state
     * @return Fresh game state ready for play
     */
    std::unique_ptr<GameState> createInitialState() const;

    // Getters
    double getSmallBlind() const noexcept { return smallBlind_; }
    double getBigBlind() const noexcept { return bigBlind_; }
    const GameParameters& getGameParameters() const noexcept { return gameParams_; }
    const std::vector<double>& getInitialStacks() const noexcept { return initialStacks_; }

    /**
     * @brief Get initial stack for specific player
     * @param player Player index (0-3)
     * @return Initial stack size in chips
     */
    double getInitialStack(int player) const;

    /**
     * @brief Validate game configuration
     * @throws std::invalid_argument if configuration is invalid
     */
    void validateConfiguration() const;

private:
    double smallBlind_;
    double bigBlind_;
    GameParameters gameParams_;
    std::vector<double> initialStacks_;  // In actual chip amounts

    /**
     * @brief Initialize default starting stacks
     */
    void initializeDefaultStacks();
};

} // namespace aof


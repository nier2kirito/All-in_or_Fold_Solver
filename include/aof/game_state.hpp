#pragma once

#include "types.hpp"
#include "card.hpp"
#include <vector>
#include <set>
#include <memory>
#include <string>

namespace aof {

// Forward declaration
class Game;

/**
 * @brief Represents the complete state of an All-or-Fold poker game
 */
class GameState {
public:
    /**
     * @brief Construct initial game state
     * @param game Pointer to parent game instance
     */
    explicit GameState(const Game* game);

    /**
     * @brief Copy constructor for state cloning
     */
    GameState(const GameState& other);

    /**
     * @brief Assignment operator
     */
    GameState& operator=(const GameState& other);

    // State queries
    /**
     * @brief Check if game has ended
     */
    bool isTerminal() const noexcept { return gameOver_; }

    /**
     * @brief Check if this is a chance node (cards need to be dealt)
     */
    bool isChanceNode() const noexcept;

    /**
     * @brief Get current player to act (-1 if game over)
     */
    int getCurrentPlayer() const noexcept;

    /**
     * @brief Get legal actions for current player
     */
    std::vector<Action> getLegalActions() const;

    // State information access
    /**
     * @brief Get current pot size
     */
    double getPot() const noexcept { return pot_; }

    /**
     * @brief Get player stack sizes
     */
    const std::vector<double>& getPlayerStacks() const noexcept { return playerStacks_; }

    /**
     * @brief Get folded status for all players
     */
    const std::vector<bool>& getFoldedPlayers() const noexcept { return folded_; }

    /**
     * @brief Get all-in players
     */
    const std::set<int>& getAllInPlayers() const noexcept { return allInPlayers_; }

    /**
     * @brief Get hole cards for all players
     */
    const std::vector<Card>& getHoleCards() const noexcept { return holeCards_; }

    /**
     * @brief Get community cards
     */
    const std::vector<Card>& getCommunityCards() const noexcept { return communityCards_; }

    // State modification
    /**
     * @brief Apply an action and advance the game state
     * @param action Action to apply
     * @throws std::invalid_argument if action is not legal
     */
    void applyAction(Action action);

    /**
     * @brief Get final payoffs for all players (only valid if terminal)
     * @return Payoffs for each player
     * @throws std::runtime_error if game is not terminal
     */
    PlayerUtilities getReturns() const;

    /**
     * @brief Get string representation of state for debugging
     */
    std::string toString() const;

    /**
     * @brief Create a deep copy of this state
     */
    GameState clone() const { return *this; }

private:
    // Game reference
    const Game* game_;

    // Game state
    bool gameOver_;
    int nextPlayer_;
    double pot_;
    
    // Player information
    std::vector<double> initialStacks_;
    std::vector<double> playerStacks_;
    std::vector<bool> folded_;
    std::set<int> allInPlayers_;

    // Cards
    std::unique_ptr<Deck> deck_;
    std::vector<Card> holeCards_;
    std::vector<Card> communityCards_;

    // Side pots for all-in scenarios
    std::vector<std::pair<double, std::vector<int>>> sidePots_;

    // Private methods
    /**
     * @brief Deal hole cards to all players
     */
    void dealHoleCards();

    /**
     * @brief Advance to next active player
     */
    void advanceToNextPlayer();

    /**
     * @brief Handle end-of-game procedures
     */
    void handleGameEnd();

    /**
     * @brief Calculate side pots for all-in situations
     */
    void calculateSidePots();

    /**
     * @brief Determine winners and distribute pots
     */
    void distributePots();

    /**
     * @brief Check if game should end (≤1 active players or all active are all-in)
     */
    bool shouldGameEnd() const;

    /**
     * @brief Get number of active (non-folded) players
     */
    int getActivePlayerCount() const;
};

} // namespace aof


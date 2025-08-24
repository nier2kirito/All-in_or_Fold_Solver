#include "aof/game.hpp"
#include "aof/game_config.hpp"
#include <stdexcept>

namespace aof {

Game::Game(double smallBlind, 
           double bigBlind,
           const GameParameters& gameParams,
           const std::vector<double>& initialStacksBB)
    : smallBlind_(smallBlind)
    , bigBlind_(bigBlind)
    , gameParams_(gameParams)
    , initialStacks_(GameConfig::NUM_PLAYERS)
{
    if (smallBlind <= 0 || bigBlind <= 0) {
        throw std::invalid_argument("Blinds must be positive");
    }
    
    if (smallBlind >= bigBlind) {
        throw std::invalid_argument("Small blind must be less than big blind");
    }
    
    // Initialize stacks
    if (initialStacksBB.empty()) {
        initializeDefaultStacks();
    } else {
        if (initialStacksBB.size() != GameConfig::NUM_PLAYERS) {
            throw std::invalid_argument("Must specify stacks for all " + 
                                      std::to_string(GameConfig::NUM_PLAYERS) + " players");
        }
        
        for (std::size_t i = 0; i < initialStacksBB.size(); ++i) {
            if (initialStacksBB[i] <= 0) {
                throw std::invalid_argument("All stacks must be positive");
            }
            initialStacks_[i] = initialStacksBB[i] * bigBlind_;
        }
    }
    
    validateConfiguration();
}

std::unique_ptr<GameState> Game::createInitialState() const {
    return std::make_unique<GameState>(this);
}

double Game::getInitialStack(int player) const {
    if (player < 0 || player >= GameConfig::NUM_PLAYERS) {
        throw std::invalid_argument("Invalid player index: " + std::to_string(player));
    }
    return initialStacks_[player];
}

void Game::validateConfiguration() const {
    // Check that stacks are sufficient for blinds
    if (initialStacks_[0] < smallBlind_) {
        throw std::invalid_argument("Small blind player stack too small");
    }
    
    if (initialStacks_[1] < bigBlind_) {
        throw std::invalid_argument("Big blind player stack too small");
    }
    
    // Validate game parameters
    if (gameParams_.rake_per_hand < 0 || gameParams_.rake_per_hand > 1.0) {
        throw std::invalid_argument("Rake per hand must be between 0 and 1");
    }
    
    if (gameParams_.jackpot_fee_per_hand < 0 || gameParams_.jackpot_fee_per_hand > 1.0) {
        throw std::invalid_argument("Jackpot fee per hand must be between 0 and 1");
    }
    
    if (gameParams_.jackpot_payout_percentage < 0 || gameParams_.jackpot_payout_percentage > 1.0) {
        throw std::invalid_argument("Jackpot payout percentage must be between 0 and 1");
    }
}

void Game::initializeDefaultStacks() {
    double defaultStack = GameConfig::STARTING_STACK_BB * bigBlind_;
    std::fill(initialStacks_.begin(), initialStacks_.end(), defaultStack);
}

} // namespace aof


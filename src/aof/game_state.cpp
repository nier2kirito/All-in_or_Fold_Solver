#include "aof/game_state.hpp"
#include "aof/game.hpp"
#include "aof/game_config.hpp"
#include "aof/poker_evaluator.hpp"
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <iostream>

namespace aof {

GameState::GameState(const Game* game)
    : game_(game)
    , gameOver_(false)
    , nextPlayer_(0)
    , pot_(game->getSmallBlind() + game->getBigBlind())  // Initial pot with blinds
    , initialStacks_(GameConfig::NUM_PLAYERS)
    , playerStacks_(GameConfig::NUM_PLAYERS)
    , folded_(GameConfig::NUM_PLAYERS, false)
    , deck_(std::make_unique<Deck>())
    , holeCards_()
    , communityCards_()
    , sidePots_()
{
    // Initialize stacks
    for (int i = 0; i < GameConfig::NUM_PLAYERS; ++i) {
        initialStacks_[i] = game->getInitialStack(i);
        playerStacks_[i] = initialStacks_[i];
    }
    
    // Deduct blinds from stacks
    playerStacks_[0] -= game->getSmallBlind();  // Small blind
    playerStacks_[1] -= game->getBigBlind();    // Big blind
}

GameState::GameState(const GameState& other)
    : game_(other.game_)
    , gameOver_(other.gameOver_)
    , nextPlayer_(other.nextPlayer_)
    , pot_(other.pot_)
    , initialStacks_(other.initialStacks_)
    , playerStacks_(other.playerStacks_)
    , folded_(other.folded_)
    , allInPlayers_(other.allInPlayers_)
    , deck_(other.deck_ ? std::make_unique<Deck>(*other.deck_) : nullptr)
    , holeCards_(other.holeCards_)
    , communityCards_(other.communityCards_)
    , sidePots_(other.sidePots_)
{
}

GameState& GameState::operator=(const GameState& other) {
    if (this != &other) {
        game_ = other.game_;
        gameOver_ = other.gameOver_;
        nextPlayer_ = other.nextPlayer_;
        pot_ = other.pot_;
        initialStacks_ = other.initialStacks_;
        playerStacks_ = other.playerStacks_;
        folded_ = other.folded_;
        allInPlayers_ = other.allInPlayers_;
        deck_ = other.deck_ ? std::make_unique<Deck>(*other.deck_) : nullptr;
        holeCards_ = other.holeCards_;
        communityCards_ = other.communityCards_;
        sidePots_ = other.sidePots_;
    }
    return *this;
}

bool GameState::isChanceNode() const noexcept {
    return holeCards_.empty() && !gameOver_;
}

int GameState::getCurrentPlayer() const noexcept {
    if (gameOver_) {
        return -1;
    }
    return nextPlayer_;
}

std::vector<Action> GameState::getLegalActions() const {
    if (isChanceNode()) {
        return {Action::DEAL};
    }
    
    if (gameOver_) {
        return {};
    }
    
    int player = getCurrentPlayer();
    if (player < 0 || folded_[player]) {
        return {};
    }
    
    return {Action::FOLD, Action::ALL_IN};
}

void GameState::applyAction(Action action) {
    if (isChanceNode()) {
        if (action != Action::DEAL) {
            throw std::invalid_argument("Only DEAL action allowed at chance node");
        }
        dealHoleCards();
        nextPlayer_ = 2;  // Start with player 2 (after blinds)
        return;
    }
    
    if (gameOver_) {
        throw std::invalid_argument("Cannot apply action to terminal state");
    }
    
    int player = getCurrentPlayer();
    if (player < 0) {
        throw std::invalid_argument("No current player");
    }
    
    auto legalActions = getLegalActions();
    if (std::find(legalActions.begin(), legalActions.end(), action) == legalActions.end()) {
        throw std::invalid_argument("Illegal action");
    }
    
    // Apply the action
    switch (action) {
        case Action::FOLD:
            folded_[player] = true;
            break;
            
        case Action::ALL_IN: {
            double amount = playerStacks_[player];
            pot_ += amount;
            playerStacks_[player] = 0.0;
            allInPlayers_.insert(player);
            break;
        }
        
        default:
            throw std::invalid_argument("Invalid action for non-chance node");
    }
    
    advanceToNextPlayer();
    
    if (shouldGameEnd()) {
        gameOver_ = true;
        handleGameEnd();
    }
}

PlayerUtilities GameState::getReturns() const {
    if (!gameOver_) {
        throw std::runtime_error("Cannot get returns for non-terminal state");
    }
    
    PlayerUtilities returns(GameConfig::NUM_PLAYERS, 0.0);
    
    // Calculate net change for each player (winnings - initial investment)
    std::vector<double> investments(GameConfig::NUM_PLAYERS, 0.0);
    
    // Calculate actual investment for each player
    for (int i = 0; i < GameConfig::NUM_PLAYERS; ++i) {
        investments[i] = initialStacks_[i] - playerStacks_[i];
    }
    
    // Distribute side pots to winners
    PokerEvaluator evaluator;
    for (const auto& [potAmount, contributors] : sidePots_) {
        if (contributors.empty()) continue;
        
        // Find best hand among contributors
        std::vector<std::pair<HandScore, int>> playerScores;
        
        for (int player : contributors) {
            if (!folded_[player]) {
                std::vector<Card> playerHoleCards = {
                    holeCards_[player * 2],
                    holeCards_[player * 2 + 1]
                };
                
                HandScore score = evaluator.evaluateHand(playerHoleCards, communityCards_);
                playerScores.emplace_back(score, player);
            }
        }
        
        if (playerScores.empty()) continue;
        
        // Find winners (players with best hand)
        std::sort(playerScores.begin(), playerScores.end(),
                 [&evaluator](const auto& a, const auto& b) {
                     return evaluator.compareHands(a.first, b.first) > 0;
                 });
        
        std::vector<int> winners;
        winners.push_back(playerScores[0].second);
        
        // Find ties
        for (std::size_t i = 1; i < playerScores.size(); ++i) {
            if (evaluator.compareHands(playerScores[0].first, playerScores[i].first) == 0) {
                winners.push_back(playerScores[i].second);
            } else {
                break;
            }
        }
        
        // Split pot among winners
        double sharePerWinner = potAmount / winners.size();
        for (int winner : winners) {
            returns[winner] += sharePerWinner;
        }
    }
    
    // Verify zero-sum property: total investments should equal total winnings
    double totalInvestments = 0, totalWinnings = 0;
    for (double inv : investments) totalInvestments += inv;
    for (double win : returns) totalWinnings += win;
    if (std::abs(totalInvestments - totalWinnings) > 1e-6) {
        std::cout << "WARNING: Zero-sum violation in terminal state! "
                  << "Invested=" << totalInvestments << ", Won=" << totalWinnings 
                  << ", Diff=" << (totalInvestments - totalWinnings) << std::endl;
    }
    
    // Subtract investments to get net returns
    for (int i = 0; i < GameConfig::NUM_PLAYERS; ++i) {
        returns[i] -= investments[i];
    }
    
    return returns;
}

std::string GameState::toString() const {
    std::stringstream ss;
    ss << "GameState:\n";
    ss << "  Game Over: " << (gameOver_ ? "true" : "false") << "\n";
    ss << "  Current Player: " << nextPlayer_ << "\n";
    ss << "  Pot: " << pot_ << "\n";
    
    ss << "  Player Stacks: [";
    for (std::size_t i = 0; i < playerStacks_.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << playerStacks_[i];
    }
    ss << "]\n";
    
    ss << "  Folded: [";
    for (std::size_t i = 0; i < folded_.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << (folded_[i] ? "true" : "false");
    }
    ss << "]\n";
    
    ss << "  All-in Players: {";
    bool first = true;
    for (int player : allInPlayers_) {
        if (!first) ss << ", ";
        ss << player;
        first = false;
    }
    ss << "}\n";
    
    if (!holeCards_.empty()) {
        ss << "  Hole Cards: [";
        for (std::size_t i = 0; i < holeCards_.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << holeCards_[i].toString();
        }
        ss << "]\n";
    }
    
    if (!communityCards_.empty()) {
        ss << "  Community Cards: [";
        for (std::size_t i = 0; i < communityCards_.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << communityCards_[i].toString();
        }
        ss << "]\n";
    }
    
    return ss.str();
}

// Private methods
void GameState::dealHoleCards() {
    if (!holeCards_.empty()) {
        return;  // Already dealt
    }
    
    holeCards_ = deck_->dealCards(GameConfig::NUM_PLAYERS * GameConfig::HOLE_CARDS_PER_PLAYER);
}

void GameState::advanceToNextPlayer() {
    do {
        nextPlayer_ = (nextPlayer_ + 1) % GameConfig::NUM_PLAYERS;
    } while (folded_[nextPlayer_] && getActivePlayerCount() > 1);
}

void GameState::handleGameEnd() {
    // Deal community cards if not already dealt
    if (communityCards_.empty()) {
        communityCards_ = deck_->dealCards(GameConfig::COMMUNITY_CARDS);
    }
    
    calculateSidePots();
}

void GameState::calculateSidePots() {
    sidePots_.clear();
    
    // Calculate total contribution per player
    std::vector<std::pair<double, int>> contributions;  // (amount, player)
    
    for (int i = 0; i < GameConfig::NUM_PLAYERS; ++i) {
        double totalContribution = initialStacks_[i] - playerStacks_[i];
        if (totalContribution > 0) {
            contributions.emplace_back(totalContribution, i);
        }
    }
    
    // Sort by contribution amount
    std::sort(contributions.begin(), contributions.end());
    
    // Create side pots
    double prevAmount = 0.0;
    std::set<int> eligiblePlayers;
    
    // All non-folded players are initially eligible
    for (int i = 0; i < GameConfig::NUM_PLAYERS; ++i) {
        if (!folded_[i]) {
            eligiblePlayers.insert(i);
        }
    }
    
    // Count total number of players who contributed at each level
    std::vector<int> allPlayers;
    for (int i = 0; i < GameConfig::NUM_PLAYERS; ++i) {
        if (initialStacks_[i] - playerStacks_[i] > 0) {
            allPlayers.push_back(i);
        }
    }
    
    std::set<int> remainingContributors(allPlayers.begin(), allPlayers.end());
    
    for (const auto& [amount, player] : contributions) {
        if (amount > prevAmount && !eligiblePlayers.empty()) {
            // Pot increment includes contributions from ALL players at this level
            double potIncrement = (amount - prevAmount) * remainingContributors.size();
            if (potIncrement > 0) {
                sidePots_.emplace_back(potIncrement, 
                                      std::vector<int>(eligiblePlayers.begin(), eligiblePlayers.end()));
            }
        }
        
        // Player is no longer eligible for future side pots (but contributed to this level)
        eligiblePlayers.erase(player);
        remainingContributors.erase(player);
        prevAmount = amount;
    }
}

bool GameState::shouldGameEnd() const {
    int activeCount = getActivePlayerCount();
    
    // Game ends if ≤1 active players
    if (activeCount <= 1) {
        return true;
    }
    
    // Game ends if all active players are all-in
    for (int i = 0; i < GameConfig::NUM_PLAYERS; ++i) {
        if (!folded_[i] && allInPlayers_.count(i) == 0) {
            return false;  // Found active player who hasn't gone all-in
        }
    }
    
    return true;  // All active players are all-in
}

int GameState::getActivePlayerCount() const {
    int count = 0;
    for (int i = 0; i < GameConfig::NUM_PLAYERS; ++i) {
        if (!folded_[i]) {
            ++count;
        }
    }
    return count;
}

} // namespace aof


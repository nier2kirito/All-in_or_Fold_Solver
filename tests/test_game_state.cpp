#include <iostream>
#include <cassert>
#include "aof/game.hpp"
#include "aof/game_state.hpp"

void testInitialState() {
    std::cout << "Testing initial game state..." << std::endl;
    
    aof::Game game(0.4, 1.0);
    auto state = game.createInitialState();
    
    // Initial state should be a chance node
    assert(state->isChanceNode());
    assert(!state->isTerminal());
    assert(state->getCurrentPlayer() == 0);  // Initial player before dealing
    
    // Check initial pot (blinds)
    assert(state->getPot() == 1.4);  // 0.4 + 1.0
    
    // Check stacks
    auto stacks = state->getPlayerStacks();
    assert(stacks.size() == 4);
    assert(stacks[0] == 7.6);  // 8.0 - 0.4 (small blind)
    assert(stacks[1] == 7.0);  // 8.0 - 1.0 (big blind)
    assert(stacks[2] == 8.0);  // No blind
    assert(stacks[3] == 8.0);  // No blind
    
    // Check folded status
    auto folded = state->getFoldedPlayers();
    for (bool f : folded) {
        assert(!f);  // No one should be folded initially
        (void)f; // Suppress unused variable warning
    }
    
    // Legal actions should be DEAL only
    auto actions = state->getLegalActions();
    assert(actions.size() == 1);
    assert(actions[0] == aof::Action::DEAL);
    
    std::cout << "Initial state tests passed!" << std::endl;
}

void testCardDealing() {
    std::cout << "Testing card dealing..." << std::endl;
    
    aof::Game game(0.4, 1.0);
    auto state = game.createInitialState();
    
    // Deal cards
    state->applyAction(aof::Action::DEAL);
    
    // Should no longer be chance node
    assert(!state->isChanceNode());
    assert(!state->isTerminal());
    
    // Should have hole cards
    auto holeCards = state->getHoleCards();
    assert(holeCards.size() == 8);  // 4 players * 2 cards each
    
    // All cards should be valid
    for (const auto& card : holeCards) {
        assert(card.isValid());
        (void)card; // Suppress unused variable warning
    }
    
    // Current player should be 2 (after blinds)
    assert(state->getCurrentPlayer() == 2);
    
    // Legal actions should be FOLD and ALL_IN
    auto actions = state->getLegalActions();
    assert(actions.size() == 2);
    assert(std::find(actions.begin(), actions.end(), aof::Action::FOLD) != actions.end());
    assert(std::find(actions.begin(), actions.end(), aof::Action::ALL_IN) != actions.end());
    
    std::cout << "Card dealing tests passed!" << std::endl;
}

void testPlayerActions() {
    std::cout << "Testing player actions..." << std::endl;
    
    aof::Game game(0.4, 1.0);
    auto state = game.createInitialState();
    
    // Deal cards
    state->applyAction(aof::Action::DEAL);
    
    // Player 2 folds
    assert(state->getCurrentPlayer() == 2);
    state->applyAction(aof::Action::FOLD);
    
    // Check folded status
    auto folded = state->getFoldedPlayers();
    assert(folded[2]);
    assert(!folded[0] && !folded[1] && !folded[3]);
    
    // Player 3 goes all-in
    assert(state->getCurrentPlayer() == 3);
    double initialPot = state->getPot();
    double player3Stack = state->getPlayerStacks()[3];
    
    state->applyAction(aof::Action::ALL_IN);
    
    // Check all-in status
    auto allInPlayers = state->getAllInPlayers();
    assert(allInPlayers.count(3) > 0);
    
    // Check pot increased
    assert(state->getPot() == initialPot + player3Stack);
    (void)initialPot; (void)player3Stack; // Suppress unused variable warnings
    
    // Check player 3 stack is now 0
    assert(state->getPlayerStacks()[3] == 0.0);
    
    std::cout << "Player actions tests passed!" << std::endl;
}

void testGameTermination() {
    std::cout << "Testing game termination..." << std::endl;
    
    aof::Game game(0.4, 1.0);
    auto state = game.createInitialState();
    
    // Deal cards
    state->applyAction(aof::Action::DEAL);
    
    // All players except one fold
    state->applyAction(aof::Action::FOLD);  // Player 2 folds
    state->applyAction(aof::Action::FOLD);  // Player 3 folds
    state->applyAction(aof::Action::FOLD);  // Player 0 folds
    
    // Game should be terminal now
    assert(state->isTerminal());
    assert(state->getCurrentPlayer() == -1);
    
    // Should have community cards dealt
    auto communityCards = state->getCommunityCards();
    assert(communityCards.size() == 5);
    
    // Can get returns
    auto returns = state->getReturns();
    assert(returns.size() == 4);
    
    // Player 1 (big blind) should win the pot
    assert(returns[1] > 0);  // Winner gets positive return
    
    std::cout << "Game termination tests passed!" << std::endl;
}

void testAllInScenario() {
    std::cout << "Testing all-in scenario..." << std::endl;
    
    aof::Game game(0.4, 1.0);
    auto state = game.createInitialState();
    
    // Deal cards
    state->applyAction(aof::Action::DEAL);
    
    // All remaining players go all-in
    state->applyAction(aof::Action::ALL_IN);  // Player 2
    state->applyAction(aof::Action::ALL_IN);  // Player 3
    state->applyAction(aof::Action::ALL_IN);  // Player 0
    state->applyAction(aof::Action::ALL_IN);  // Player 1
    
    // Game should be terminal
    assert(state->isTerminal());
    
    // All players should be all-in
    auto allInPlayers = state->getAllInPlayers();
    assert(allInPlayers.size() == 4);
    
    // Should have community cards
    auto communityCards = state->getCommunityCards();
    assert(communityCards.size() == 5);
    
    // Can get returns (sum should be approximately 0 due to rake)
    auto returns = state->getReturns();
    double totalReturns = 0;
    for (double ret : returns) {
        totalReturns += ret;
    }
    
    // Total returns should be close to 0 (might be slightly negative due to rounding)
    assert(std::abs(totalReturns) < 0.01);
    
    std::cout << "All-in scenario tests passed!" << std::endl;
}

void testStateCloning() {
    std::cout << "Testing state cloning..." << std::endl;
    
    aof::Game game(0.4, 1.0);
    auto state1 = game.createInitialState();
    
    // Deal cards and make some actions
    state1->applyAction(aof::Action::DEAL);
    state1->applyAction(aof::Action::FOLD);  // Player 2 folds
    
    // Clone the state
    auto state2 = std::make_unique<aof::GameState>(state1->clone());
    
    // Both states should be identical
    assert(state1->getCurrentPlayer() == state2->getCurrentPlayer());
    assert(state1->getPot() == state2->getPot());
    assert(state1->getFoldedPlayers() == state2->getFoldedPlayers());
    assert(state1->getHoleCards() == state2->getHoleCards());
    
    // Modify one state
    state1->applyAction(aof::Action::ALL_IN);  // Player 3 goes all-in
    
    // States should now be different
    assert(state1->getCurrentPlayer() != state2->getCurrentPlayer());
    assert(state1->getPot() != state2->getPot());
    
    std::cout << "State cloning tests passed!" << std::endl;
}

void runGameStateTests() {
    try {
        testInitialState();
        testCardDealing();
        testPlayerActions();
        testGameTermination();
        testAllInScenario();
        testStateCloning();
        
        std::cout << "\nAll game state tests passed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Game state test failed with exception: " << e.what() << std::endl;
        throw;
    }
}


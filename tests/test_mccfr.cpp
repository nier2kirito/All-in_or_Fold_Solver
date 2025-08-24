#include <iostream>
#include <cassert>
#include <cmath>
#include "mccfr/node.hpp"
#include "mccfr/utils.hpp"
#include "mccfr/trainer.hpp"
#include "aof/game.hpp"

void testNode() {
    std::cout << "Testing MCCFR Node..." << std::endl;
    
    // Create node with 2 actions (FOLD, ALL_IN)
    mccfr::Node node(2);
    
    // Initial strategy should be uniform
    auto initialStrategy = node.getStrategy(1.0);
    assert(initialStrategy.size() == 2);
    assert(std::abs(initialStrategy[0] - 0.5) < 0.001);
    assert(std::abs(initialStrategy[1] - 0.5) < 0.001);
    
    // Update regrets
    node.updateRegret(0, -1.0);  // Negative regret for FOLD
    node.updateRegret(1, 2.0);   // Positive regret for ALL_IN
    
    // Strategy should now favor ALL_IN
    auto newStrategy = node.getStrategy(1.0);
    assert(newStrategy[1] > newStrategy[0]);  // ALL_IN probability > FOLD probability
    assert(std::abs(newStrategy[0] + newStrategy[1] - 1.0) < 0.001);  // Should sum to 1
    
    // Test average strategy
    auto avgStrategy = node.getAverageStrategy();
    assert(avgStrategy.size() == 2);
    assert(std::abs(avgStrategy[0] + avgStrategy[1] - 1.0) < 0.001);
    
    // Test visit count
    assert(node.getVisitCount() > 0);
    
    // Test reset
    node.reset();
    assert(node.getVisitCount() == 0);
    auto resetStrategy = node.getStrategy(1.0);
    assert(std::abs(resetStrategy[0] - 0.5) < 0.001);
    assert(std::abs(resetStrategy[1] - 0.5) < 0.001);
    
    std::cout << "Node tests passed!" << std::endl;
}

void testUtils() {
    std::cout << "Testing MCCFR utilities..." << std::endl;
    
    // Test action sampling
    std::vector<double> strategy = {0.2, 0.8};  // 20% FOLD, 80% ALL_IN
    
    // Sample many times and check distribution
    int foldCount = 0, allInCount = 0;
    int numSamples = 10000;
    
    for (int i = 0; i < numSamples; ++i) {
        int action = mccfr::utils::sampleAction(strategy);
        assert(action >= 0 && action < 2);
        
        if (action == 0) foldCount++;
        else allInCount++;
    }
    
    // Check approximate distribution (within 5% tolerance)
    double foldRatio = static_cast<double>(foldCount) / numSamples;
    double allInRatio = static_cast<double>(allInCount) / numSamples;
    
    assert(std::abs(foldRatio - 0.2) < 0.05);
    assert(std::abs(allInRatio - 0.8) < 0.05);
    (void)foldRatio; (void)allInRatio; // Suppress unused variable warnings
    
    // Test information set generation
    aof::Game game(0.4, 1.0);
    auto state = game.createInitialState();
    state->applyAction(aof::Action::DEAL);
    
    for (int player = 0; player < 4; ++player) {
        std::string infoSet = mccfr::utils::getInformationSet(*state, player);
        assert(!infoSet.empty());
        assert(infoSet.find("P" + std::to_string(player)) == 0);  // Should start with player ID
    }
    
    std::cout << "Utils tests passed!" << std::endl;
}

void testTrainer() {
    std::cout << "Testing MCCFR Trainer..." << std::endl;
    
    aof::Game game(0.4, 1.0);
    mccfr::Trainer trainer(game);
    
    // Quick training run
    mccfr::TrainingConfig config;
    config.iterations = 1000;  // Small number for testing
    config.enableProgressOutput = false;  // Quiet for tests
    config.outputPrefix = "test_strategy";
    
    auto utilities = trainer.train(config);
    
    // Check results
    assert(utilities.size() == 4);
    
    // In a zero-sum game, utilities should sum to approximately 0
    double totalUtility = 0;
    for (double u : utilities) {
        totalUtility += u;
    }
    assert(std::abs(totalUtility) < 1.0);  // Should be close to 0
    
    // Check that strategies were learned
    auto stats = trainer.getStats();
    assert(stats.totalIterations == 1000);
    assert(stats.informationSetsCount > 0);
    
    // Test strategy retrieval
    auto allStrategies = trainer.getAllStrategies();
    assert(!allStrategies.empty());
    
    for (const auto& [infoSet, strategy] : allStrategies) {
        assert(!infoSet.empty());
        assert(strategy.size() == 2);  // FOLD, ALL_IN
        
        // Strategy should be a valid probability distribution
        double sum = strategy[0] + strategy[1];
        assert(std::abs(sum - 1.0) < 0.001);
        assert(strategy[0] >= 0 && strategy[0] <= 1);
        assert(strategy[1] >= 0 && strategy[1] <= 1);
        (void)sum; // Suppress unused variable warning
    }
    
    std::cout << "Trainer tests passed!" << std::endl;
}

void testInformationSetGeneration() {
    std::cout << "Testing information set generation..." << std::endl;
    
    aof::Game game(0.4, 1.0);
    auto state = game.createInitialState();
    state->applyAction(aof::Action::DEAL);
    
    // Test different game situations
    std::string infoSet0 = mccfr::utils::getInformationSet(*state, 0);
    std::string infoSet2 = mccfr::utils::getInformationSet(*state, 2);
    
    // Different players should have different info sets
    assert(infoSet0 != infoSet2);
    
    // Make some actions and check info sets change
    state->applyAction(aof::Action::FOLD);  // Player 2 folds
    
    std::string newInfoSet0 = mccfr::utils::getInformationSet(*state, 0);
    std::string newInfoSet3 = mccfr::utils::getInformationSet(*state, 3);
    
    // Info sets should reflect the new game state
    assert(newInfoSet0 != infoSet0);  // Should be different after action
    
    // Player 3's info set should show that player 2 folded
    assert(newInfoSet3.find("F") != std::string::npos);  // Should contain fold indicator
    
    std::cout << "Information set generation tests passed!" << std::endl;
}

void testConvergence() {
    std::cout << "Testing strategy convergence..." << std::endl;
    
    aof::Game game(0.4, 1.0);
    mccfr::Trainer trainer(game);
    
    // Train for longer to test convergence
    mccfr::TrainingConfig config;
    config.iterations = 10000;  // More iterations for convergence test
    config.enableProgressOutput = false;
    config.outputPrefix = "convergence_test";
    
    auto utilities1 = trainer.train(config);
    
    // Train again and check if strategies are similar
    mccfr::Trainer trainer2(game);
    auto utilities2 = trainer2.train(config);
    
    // Utilities should be in similar range (not exact due to randomness)
    for (std::size_t i = 0; i < utilities1.size(); ++i) {
        double diff = std::abs(utilities1[i] - utilities2[i]);
        assert(diff < 2.0);  // Should be reasonably close
        (void)diff; // Suppress unused variable warning
    }
    
    std::cout << "Convergence tests passed!" << std::endl;
}

void runMCCFRTests() {
    try {
        testNode();
        testUtils();
        testTrainer();
        testInformationSetGeneration();
        testConvergence();
        
        std::cout << "\nAll MCCFR tests passed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "MCCFR test failed with exception: " << e.what() << std::endl;
        throw;
    }
}


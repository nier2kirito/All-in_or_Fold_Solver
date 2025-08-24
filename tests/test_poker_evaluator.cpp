#include <iostream>
#include <cassert>
#include <vector>
#include "aof/poker_evaluator.hpp"

void testHandEvaluation() {
    std::cout << "Testing poker hand evaluation..." << std::endl;
    
    aof::PokerEvaluator evaluator;
    
    // Test straight flush
    std::vector<aof::Card> straightFlush = {
        {"A", "s"}, {"K", "s"}, {"Q", "s"}, {"J", "s"}, {"10", "s"}
    };
    auto sfScore = evaluator.evaluateFiveCardHand(straightFlush);
    assert(!sfScore.empty());
    assert(sfScore[0] == static_cast<int>(aof::HandRank::STRAIGHT_FLUSH));
    
    // Test four of a kind
    std::vector<aof::Card> fourKind = {
        {"A", "s"}, {"A", "h"}, {"A", "d"}, {"A", "c"}, {"K", "s"}
    };
    auto fkScore = evaluator.evaluateFiveCardHand(fourKind);
    assert(fkScore[0] == static_cast<int>(aof::HandRank::FOUR_OF_A_KIND));
    
    // Test full house
    std::vector<aof::Card> fullHouse = {
        {"A", "s"}, {"A", "h"}, {"A", "d"}, {"K", "c"}, {"K", "s"}
    };
    auto fhScore = evaluator.evaluateFiveCardHand(fullHouse);
    assert(fhScore[0] == static_cast<int>(aof::HandRank::FULL_HOUSE));
    
    // Test flush
    std::vector<aof::Card> flush = {
        {"A", "s"}, {"J", "s"}, {"9", "s"}, {"7", "s"}, {"5", "s"}
    };
    auto flScore = evaluator.evaluateFiveCardHand(flush);
    assert(flScore[0] == static_cast<int>(aof::HandRank::FLUSH));
    
    // Test straight
    std::vector<aof::Card> straight = {
        {"A", "s"}, {"K", "h"}, {"Q", "d"}, {"J", "c"}, {"10", "s"}
    };
    auto stScore = evaluator.evaluateFiveCardHand(straight);
    assert(stScore[0] == static_cast<int>(aof::HandRank::STRAIGHT));
    
    // Test three of a kind
    std::vector<aof::Card> threeKind = {
        {"A", "s"}, {"A", "h"}, {"A", "d"}, {"K", "c"}, {"Q", "s"}
    };
    auto tkScore = evaluator.evaluateFiveCardHand(threeKind);
    assert(tkScore[0] == static_cast<int>(aof::HandRank::THREE_OF_A_KIND));
    
    // Test two pair
    std::vector<aof::Card> twoPair = {
        {"A", "s"}, {"A", "h"}, {"K", "d"}, {"K", "c"}, {"Q", "s"}
    };
    auto tpScore = evaluator.evaluateFiveCardHand(twoPair);
    assert(tpScore[0] == static_cast<int>(aof::HandRank::TWO_PAIR));
    
    // Test one pair
    std::vector<aof::Card> onePair = {
        {"A", "s"}, {"A", "h"}, {"K", "d"}, {"Q", "c"}, {"J", "s"}
    };
    auto opScore = evaluator.evaluateFiveCardHand(onePair);
    assert(opScore[0] == static_cast<int>(aof::HandRank::PAIR));
    
    // Test high card
    std::vector<aof::Card> highCard = {
        {"A", "s"}, {"K", "h"}, {"Q", "d"}, {"J", "c"}, {"9", "s"}
    };
    auto hcScore = evaluator.evaluateFiveCardHand(highCard);
    assert(hcScore[0] == static_cast<int>(aof::HandRank::HIGH_CARD));
    
    std::cout << "Hand evaluation tests passed!" << std::endl;
}

void testHandComparison() {
    std::cout << "Testing hand comparison..." << std::endl;
    
    aof::PokerEvaluator evaluator;
    
    // Straight flush beats four of a kind
    std::vector<aof::Card> straightFlush = {
        {"9", "s"}, {"8", "s"}, {"7", "s"}, {"6", "s"}, {"5", "s"}
    };
    std::vector<aof::Card> fourKind = {
        {"A", "s"}, {"A", "h"}, {"A", "d"}, {"A", "c"}, {"K", "s"}
    };
    
    auto sfScore = evaluator.evaluateFiveCardHand(straightFlush);
    auto fkScore = evaluator.evaluateFiveCardHand(fourKind);
    
    assert(evaluator.compareHands(sfScore, fkScore) > 0);
    assert(evaluator.compareHands(fkScore, sfScore) < 0);
    
    // Higher four of a kind beats lower
    std::vector<aof::Card> fourKings = {
        {"K", "s"}, {"K", "h"}, {"K", "d"}, {"K", "c"}, {"A", "s"}
    };
    
    auto fkScore2 = evaluator.evaluateFiveCardHand(fourKings);
    assert(evaluator.compareHands(fkScore, fkScore2) > 0);  // Aces beat Kings
    
    // Same hand should tie
    std::vector<aof::Card> sameFourKind = {
        {"A", "d"}, {"A", "c"}, {"A", "s"}, {"A", "h"}, {"K", "h"}
    };
    auto sameScore = evaluator.evaluateFiveCardHand(sameFourKind);
    assert(evaluator.compareHands(fkScore, sameScore) == 0);
    
    std::cout << "Hand comparison tests passed!" << std::endl;
}

void testSevenCardEvaluation() {
    std::cout << "Testing 7-card hand evaluation..." << std::endl;
    
    aof::PokerEvaluator evaluator;
    
    // Test with hole cards and community cards
    std::vector<aof::Card> holeCards = {{"A", "s"}, {"A", "h"}};
    std::vector<aof::Card> communityCards = {
        {"A", "d"}, {"K", "c"}, {"Q", "s"}, {"J", "h"}, {"10", "c"}
    };
    
    auto score = evaluator.evaluateHand(holeCards, communityCards);
    assert(!score.empty());
    
    // Should find the best 5-card hand from the 7 available
    // In this case: A-A-A-K-Q (three of a kind, Aces)
    assert(score[0] == static_cast<int>(aof::HandRank::THREE_OF_A_KIND));
    
    // Test error cases
    try {
        evaluator.evaluateHand({{"A", "s"}}, communityCards);  // Wrong number of hole cards
        assert(false && "Should have thrown exception");
    } catch (const std::invalid_argument&) {
        // Expected
    }
    
    try {
        evaluator.evaluateHand(holeCards, {{"A", "s"}});  // Wrong number of community cards
        assert(false && "Should have thrown exception");
    } catch (const std::invalid_argument&) {
        // Expected
    }
    
    std::cout << "7-card evaluation tests passed!" << std::endl;
}

void testSpecialStraights() {
    std::cout << "Testing special straights..." << std::endl;
    
    aof::PokerEvaluator evaluator;
    
    // Test wheel straight (A-2-3-4-5)
    std::vector<aof::Card> wheel = {
        {"A", "s"}, {"2", "h"}, {"3", "d"}, {"4", "c"}, {"5", "s"}
    };
    auto wheelScore = evaluator.evaluateFiveCardHand(wheel);
    assert(wheelScore[0] == static_cast<int>(aof::HandRank::STRAIGHT));
    
    // Test broadway straight (10-J-Q-K-A)
    std::vector<aof::Card> broadway = {
        {"10", "s"}, {"J", "h"}, {"Q", "d"}, {"K", "c"}, {"A", "s"}
    };
    auto broadwayScore = evaluator.evaluateFiveCardHand(broadway);
    assert(broadwayScore[0] == static_cast<int>(aof::HandRank::STRAIGHT));
    
    // Broadway should beat wheel
    assert(evaluator.compareHands(broadwayScore, wheelScore) > 0);
    
    std::cout << "Special straights tests passed!" << std::endl;
}

void runPokerEvaluatorTests() {
    try {
        testHandEvaluation();
        testHandComparison();
        testSevenCardEvaluation();
        testSpecialStraights();
        
        std::cout << "\nAll poker evaluator tests passed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Poker evaluator test failed with exception: " << e.what() << std::endl;
        throw;
    }
}


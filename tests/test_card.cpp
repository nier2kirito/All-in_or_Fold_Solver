#include <iostream>
#include <cassert>
#include <stdexcept>
#include "aof/card.hpp"

void testCardConstruction() {
    std::cout << "Testing card construction..." << std::endl;
    
    // Valid cards
    aof::Card aceSpades("A", "s");
    assert(aceSpades.getRank() == "A");
    assert(aceSpades.getSuit() == "s");
    assert(aceSpades.getRankValue() == 14);
    assert(aceSpades.toString() == "As");
    assert(aceSpades.isValid());
    
    aof::Card twoHearts("2", "h");
    assert(twoHearts.getRankValue() == 2);
    assert(twoHearts.toString() == "2h");
    
    // Invalid cards should throw
    try {
        aof::Card invalid("X", "s");
        assert(false && "Should have thrown exception");
    } catch (const std::invalid_argument&) {
        // Expected
    }
    
    try {
        aof::Card invalid("A", "x");
        assert(false && "Should have thrown exception");
    } catch (const std::invalid_argument&) {
        // Expected
    }
    
    std::cout << "Card construction tests passed!" << std::endl;
}

void testCardComparison() {
    std::cout << "Testing card comparison..." << std::endl;
    
    aof::Card ace("A", "s");
    aof::Card king("K", "h");
    aof::Card ace2("A", "h");
    
    assert(ace != king);
    assert(ace != ace2);  // Different suits
    assert(ace < ace2 || ace2 < ace);  // One should be less than the other
    
    aof::Card sameCcard("A", "s");
    assert(ace == sameCcard);
    
    std::cout << "Card comparison tests passed!" << std::endl;
}

void testDeck() {
    std::cout << "Testing deck operations..." << std::endl;
    
    aof::Deck deck(12345);  // Fixed seed for reproducible tests
    
    assert(deck.size() == 52);
    assert(!deck.isEmpty());
    
    // Deal some cards
    auto card1 = deck.dealCard();
    assert(deck.size() == 51);
    assert(card1.isValid());
    
    auto cards = deck.dealCards(5);
    assert(cards.size() == 5);
    assert(deck.size() == 46);
    
    for (const auto& card : cards) {
        assert(card.isValid());
        (void)card; // Suppress unused variable warning
    }
    
    // Test dealing too many cards
    try {
        deck.dealCards(100);
        assert(false && "Should have thrown exception");
    } catch (const std::runtime_error&) {
        // Expected
    }
    
    // Reset deck
    deck.reset();
    assert(deck.size() == 52);
    
    std::cout << "Deck tests passed!" << std::endl;
}

void testCardUtils() {
    std::cout << "Testing card utilities..." << std::endl;
    
    aof::Card aceSpades("A", "s");
    aof::Card kingSpades("K", "s");
    aof::Card aceHearts("A", "h");
    
    // Test suited check
    assert(aof::card_utils::areSuited(aceSpades, kingSpades));
    assert(!aof::card_utils::areSuited(aceSpades, aceHearts));
    
    // Test abstracted hole cards
    std::string abstracted1 = aof::card_utils::getAbstractedHoleCards(aceSpades, kingSpades);
    std::string abstracted2 = aof::card_utils::getAbstractedHoleCards(aceSpades, aceHearts);
    
    assert(!abstracted1.empty());
    assert(!abstracted2.empty());
    assert(abstracted1 != abstracted2);
    
    // Test pair abstraction
    std::string pairAbstracted = aof::card_utils::getAbstractedHoleCards(aceSpades, aceHearts);
    assert(pairAbstracted.find("AA") != std::string::npos);
    
    std::cout << "Card utilities tests passed!" << std::endl;
}

void runCardTests() {
    try {
        testCardConstruction();
        testCardComparison();
        testDeck();
        testCardUtils();
        
        std::cout << "\nAll card tests passed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Card test failed with exception: " << e.what() << std::endl;
        throw;
    }
}

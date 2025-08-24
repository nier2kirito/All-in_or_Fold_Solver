#pragma once

#include <string>
#include <vector>
#include <array>
#include <random>
#include <unordered_map>

namespace aof {

/**
 * @brief Represents a playing card
 */
class Card {
public:
    /**
     * @brief Construct a card with rank and suit
     * @param rank Card rank ("2"-"A")
     * @param suit Card suit ("h", "d", "c", "s")
     */
    Card(std::string rank, std::string suit);

    /**
     * @brief Default constructor (invalid card)
     */
    Card() = default;

    // Getters
    const std::string& getRank() const noexcept { return rank_; }
    const std::string& getSuit() const noexcept { return suit_; }
    
    /**
     * @brief Get numeric value of rank (2=2, ..., A=14)
     */
    int getRankValue() const noexcept;
    
    /**
     * @brief Get string representation of card
     */
    std::string toString() const;
    
    /**
     * @brief Check if card is valid
     */
    bool isValid() const noexcept;

    // Comparison operators
    bool operator==(const Card& other) const noexcept;
    bool operator!=(const Card& other) const noexcept;
    bool operator<(const Card& other) const noexcept;

private:
    std::string rank_;
    std::string suit_;
    
    /// Rank to numeric value mapping
    static const std::unordered_map<std::string, int> RANK_VALUES;
};

/**
 * @brief Manages a deck of playing cards
 */
class Deck {
public:
    /**
     * @brief Create and shuffle a standard 52-card deck
     * @param rng Random number generator for shuffling
     */
    explicit Deck(std::mt19937& rng);

    /**
     * @brief Create a deck with specific seed
     * @param seed Seed for random number generator
     */
    explicit Deck(std::uint32_t seed = std::random_device{}());

    /**
     * @brief Deal the next card from the deck
     * @return Next card
     * @throws std::runtime_error if deck is empty
     */
    Card dealCard();

    /**
     * @brief Deal multiple cards
     * @param count Number of cards to deal
     * @return Vector of dealt cards
     */
    std::vector<Card> dealCards(int count);

    /**
     * @brief Check if deck is empty
     */
    bool isEmpty() const noexcept { return cards_.empty(); }

    /**
     * @brief Get remaining card count
     */
    std::size_t size() const noexcept { return cards_.size(); }

    /**
     * @brief Reset and reshuffle the deck
     */
    void reset();

private:
    std::vector<Card> cards_;
    std::mt19937 rng_;
    
    /**
     * @brief Create a standard 52-card deck
     */
    static std::vector<Card> createStandardDeck();
    
    /**
     * @brief Shuffle the current deck
     */
    void shuffle();
};

/**
 * @brief Utility functions for card operations
 */
namespace card_utils {

/**
 * @brief Check if two cards are suited (same suit)
 */
bool areSuited(const Card& card1, const Card& card2) noexcept;

/**
 * @brief Get abstracted representation of hole cards for information sets
 * @param card1 First hole card
 * @param card2 Second hole card
 * @return Abstracted string representation
 */
std::string getAbstractedHoleCards(const Card& card1, const Card& card2);

/**
 * @brief Get all possible ranks
 */
const std::vector<std::string>& getAllRanks();

/**
 * @brief Get all possible suits  
 */
const std::vector<std::string>& getAllSuits();

} // namespace card_utils

} // namespace aof


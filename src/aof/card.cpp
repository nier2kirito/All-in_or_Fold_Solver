#include "aof/card.hpp"
#include <algorithm>
#include <stdexcept>
#include <sstream>

namespace aof {

// Static rank values mapping
const std::unordered_map<std::string, int> Card::RANK_VALUES = {
    {"2", 2}, {"3", 3}, {"4", 4}, {"5", 5}, {"6", 6}, {"7", 7}, {"8", 8},
    {"9", 9}, {"10", 10}, {"J", 11}, {"Q", 12}, {"K", 13}, {"A", 14}
};

// Card implementation
Card::Card(std::string rank, std::string suit) 
    : rank_(std::move(rank)), suit_(std::move(suit)) {
    if (!isValid()) {
        throw std::invalid_argument("Invalid card: " + rank_ + suit_);
    }
}

int Card::getRankValue() const noexcept {
    auto it = RANK_VALUES.find(rank_);
    return (it != RANK_VALUES.end()) ? it->second : 0;
}

std::string Card::toString() const {
    return rank_ + suit_;
}

bool Card::isValid() const noexcept {
    static const std::vector<std::string> validSuits = {"h", "d", "c", "s"};
    
    return RANK_VALUES.count(rank_) > 0 && 
           std::find(validSuits.begin(), validSuits.end(), suit_) != validSuits.end();
}

bool Card::operator==(const Card& other) const noexcept {
    return rank_ == other.rank_ && suit_ == other.suit_;
}

bool Card::operator!=(const Card& other) const noexcept {
    return !(*this == other);
}

bool Card::operator<(const Card& other) const noexcept {
    int thisValue = getRankValue();
    int otherValue = other.getRankValue();
    
    if (thisValue != otherValue) {
        return thisValue < otherValue;
    }
    return suit_ < other.suit_;
}

// Deck implementation
Deck::Deck(std::mt19937& rng) : rng_(rng) {
    cards_ = createStandardDeck();
    shuffle();
}

Deck::Deck(std::uint32_t seed) : rng_(seed) {
    cards_ = createStandardDeck();
    shuffle();
}

Card Deck::dealCard() {
    if (isEmpty()) {
        throw std::runtime_error("Cannot deal from empty deck");
    }
    
    Card card = cards_.back();
    cards_.pop_back();
    return card;
}

std::vector<Card> Deck::dealCards(int count) {
    if (count < 0) {
        throw std::invalid_argument("Cannot deal negative number of cards");
    }
    
    if (static_cast<std::size_t>(count) > cards_.size()) {
        throw std::runtime_error("Not enough cards in deck");
    }
    
    std::vector<Card> dealt;
    dealt.reserve(count);
    
    for (int i = 0; i < count; ++i) {
        dealt.push_back(dealCard());
    }
    
    return dealt;
}

void Deck::reset() {
    cards_ = createStandardDeck();
    shuffle();
}

std::vector<Card> Deck::createStandardDeck() {
    const std::vector<std::string> ranks = {"2", "3", "4", "5", "6", "7", "8", "9", 
                                           "10", "J", "Q", "K", "A"};
    const std::vector<std::string> suits = {"h", "d", "c", "s"};
    
    std::vector<Card> deck;
    deck.reserve(52);
    
    for (const auto& rank : ranks) {
        for (const auto& suit : suits) {
            deck.emplace_back(rank, suit);
        }
    }
    
    return deck;
}

void Deck::shuffle() {
    std::shuffle(cards_.begin(), cards_.end(), rng_);
}

// Utility functions
namespace card_utils {

bool areSuited(const Card& card1, const Card& card2) noexcept {
    return card1.getSuit() == card2.getSuit();
}

std::string getAbstractedHoleCards(const Card& card1, const Card& card2) {
    // Order cards by rank (higher first)
    const Card* highCard = &card1;
    const Card* lowCard = &card2;
    
    if (card1.getRankValue() < card2.getRankValue()) {
        std::swap(highCard, lowCard);
    }
    
    std::stringstream ss;
    ss << highCard->getRank();
    
    // For pairs, just show the rank once
    if (highCard->getRankValue() == lowCard->getRankValue()) {
        ss << lowCard->getRank();
    } else {
        ss << lowCard->getRank();
        ss << (areSuited(*highCard, *lowCard) ? "s" : "o");
    }
    
    return ss.str();
}

const std::vector<std::string>& getAllRanks() {
    static const std::vector<std::string> ranks = {
        "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"
    };
    return ranks;
}

const std::vector<std::string>& getAllSuits() {
    static const std::vector<std::string> suits = {"h", "d", "c", "s"};
    return suits;
}

} // namespace card_utils

} // namespace aof


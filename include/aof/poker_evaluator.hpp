#pragma once

#include "card.hpp"
#include "types.hpp"
#include <vector>
#include <array>

namespace aof {

/**
 * @brief Hand evaluation score (rank + tiebreakers)
 */
using HandScore = std::vector<int>;

/**
 * @brief Evaluates poker hands according to standard rules
 */
class PokerEvaluator {
public:
    /**
     * @brief Evaluate the best 5-card hand from hole cards and community cards
     * @param holeCards Player's hole cards (exactly 2 cards)
     * @param communityCards Community cards (exactly 5 cards)
     * @return Hand score for comparison
     * @throws std::invalid_argument if card counts are wrong
     */
    HandScore evaluateHand(const std::vector<Card>& holeCards, 
                          const std::vector<Card>& communityCards) const;

    /**
     * @brief Evaluate a specific 5-card hand
     * @param hand Exactly 5 cards
     * @return Hand score for comparison
     * @throws std::invalid_argument if not exactly 5 cards
     */
    HandScore evaluateFiveCardHand(const std::vector<Card>& hand) const;

    /**
     * @brief Compare two hand scores
     * @param score1 First hand score
     * @param score2 Second hand score
     * @return >0 if score1 wins, <0 if score2 wins, 0 if tie
     */
    static int compareHands(const HandScore& score1, const HandScore& score2);

private:
    /**
     * @brief Generate all 5-card combinations from 7 cards
     * @param cards Input cards (exactly 7)
     * @return All possible 5-card combinations
     */
    std::vector<std::vector<Card>> generateCombinations(const std::vector<Card>& cards) const;

    /**
     * @brief Create hand score from rank and tiebreaker values
     * @param handRank Primary hand ranking
     * @param tiebreakers Values for breaking ties
     * @return Complete hand score
     */
    HandScore makeScore(HandRank handRank, const std::vector<int>& tiebreakers) const;

    /**
     * @brief Check if hand is a flush
     * @param hand 5-card hand
     * @return True if all cards have same suit
     */
    bool isFlush(const std::vector<Card>& hand) const;

    /**
     * @brief Check if hand is a straight
     * @param hand 5-card hand
     * @param highCard Output parameter for high card of straight
     * @return True if cards form a straight
     */
    bool isStraight(const std::vector<Card>& hand, int& highCard) const;

    /**
     * @brief Get rank counts and sorted ranks for pair detection
     * @param hand 5-card hand
     * @param rankCounts Output parameter for count of each rank
     * @param sortedRanks Output parameter for ranks sorted by count then value
     */
    void analyzeRanks(const std::vector<Card>& hand,
                     std::array<int, 15>& rankCounts,
                     std::vector<int>& sortedRanks) const;

    /**
     * @brief Helper for generating combinations recursively
     */
    void generateCombinationsHelper(const std::vector<Card>& cards,
                                   int combinationSize,
                                   int start,
                                   std::vector<Card>& currentCombo,
                                   std::vector<std::vector<Card>>& allCombinations) const;
};

} // namespace aof


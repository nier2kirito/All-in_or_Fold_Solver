#include "aof/poker_evaluator.hpp"
#include <algorithm>
#include <stdexcept>
#include <set>

namespace aof {

HandScore PokerEvaluator::evaluateHand(const std::vector<Card>& holeCards,
                                      const std::vector<Card>& communityCards) const {
    if (holeCards.size() != 2) {
        throw std::invalid_argument("Must have exactly 2 hole cards");
    }
    if (communityCards.size() != 5) {
        throw std::invalid_argument("Must have exactly 5 community cards");
    }
    
    // Combine all 7 cards
    std::vector<Card> allCards;
    allCards.reserve(7);
    allCards.insert(allCards.end(), holeCards.begin(), holeCards.end());
    allCards.insert(allCards.end(), communityCards.begin(), communityCards.end());
    
    // Generate all 5-card combinations and find the best
    auto combinations = generateCombinations(allCards);
    HandScore bestScore;
    
    for (const auto& combo : combinations) {
        HandScore score = evaluateFiveCardHand(combo);
        if (bestScore.empty() || compareHands(score, bestScore) > 0) {
            bestScore = score;
        }
    }
    
    return bestScore;
}

HandScore PokerEvaluator::evaluateFiveCardHand(const std::vector<Card>& hand) const {
    if (hand.size() != 5) {
        throw std::invalid_argument("Must have exactly 5 cards");
    }
    
    // Analyze ranks and suits
    std::array<int, 15> rankCounts{};  // Index by rank value (2-14)
    std::vector<int> sortedRanks;
    analyzeRanks(hand, rankCounts, sortedRanks);
    
    bool isFlushHand = isFlush(hand);
    int straightHigh = 0;
    bool isStraightHand = isStraight(hand, straightHigh);
    
    // Determine hand type and create score
    if (isFlushHand && isStraightHand) {
        // Straight flush
        return makeScore(HandRank::STRAIGHT_FLUSH, {straightHigh});
    }
    
    // Check for four of a kind
    for (int rank = 14; rank >= 2; --rank) {
        if (rankCounts[rank] == 4) {
            // Find kicker
            int kicker = 0;
            for (int r = 14; r >= 2; --r) {
                if (rankCounts[r] == 1) {
                    kicker = r;
                    break;
                }
            }
            return makeScore(HandRank::FOUR_OF_A_KIND, {rank, kicker});
        }
    }
    
    // Check for full house
    int trips = 0, pair = 0;
    for (int rank = 14; rank >= 2; --rank) {
        if (rankCounts[rank] == 3 && trips == 0) {
            trips = rank;
        } else if (rankCounts[rank] == 2 && pair == 0) {
            pair = rank;
        }
    }
    if (trips > 0 && pair > 0) {
        return makeScore(HandRank::FULL_HOUSE, {trips, pair});
    }
    
    if (isFlushHand) {
        // Flush - use all 5 cards as tiebreakers
        std::vector<int> flushKickers;
        for (const auto& card : hand) {
            flushKickers.push_back(card.getRankValue());
        }
        std::sort(flushKickers.rbegin(), flushKickers.rend());
        return makeScore(HandRank::FLUSH, flushKickers);
    }
    
    if (isStraightHand) {
        return makeScore(HandRank::STRAIGHT, {straightHigh});
    }
    
    if (trips > 0) {
        // Three of a kind
        std::vector<int> kickers;
        for (int rank = 14; rank >= 2; --rank) {
            if (rankCounts[rank] == 1) {
                kickers.push_back(rank);
            }
        }
        std::sort(kickers.rbegin(), kickers.rend());
        kickers.resize(2);  // Take top 2 kickers
        
        std::vector<int> tiebreakers = {trips};
        tiebreakers.insert(tiebreakers.end(), kickers.begin(), kickers.end());
        return makeScore(HandRank::THREE_OF_A_KIND, tiebreakers);
    }
    
    // Check for pairs
    std::vector<int> pairs;
    for (int rank = 14; rank >= 2; --rank) {
        if (rankCounts[rank] == 2) {
            pairs.push_back(rank);
        }
    }
    
    if (pairs.size() >= 2) {
        // Two pair
        std::sort(pairs.rbegin(), pairs.rend());
        int kicker = 0;
        for (int rank = 14; rank >= 2; --rank) {
            if (rankCounts[rank] == 1) {
                kicker = rank;
                break;
            }
        }
        return makeScore(HandRank::TWO_PAIR, {pairs[0], pairs[1], kicker});
    }
    
    if (pairs.size() == 1) {
        // One pair
        std::vector<int> kickers;
        for (int rank = 14; rank >= 2; --rank) {
            if (rankCounts[rank] == 1) {
                kickers.push_back(rank);
            }
        }
        std::sort(kickers.rbegin(), kickers.rend());
        kickers.resize(3);  // Take top 3 kickers
        
        std::vector<int> tiebreakers = {pairs[0]};
        tiebreakers.insert(tiebreakers.end(), kickers.begin(), kickers.end());
        return makeScore(HandRank::PAIR, tiebreakers);
    }
    
    // High card
    std::vector<int> highCards;
    for (const auto& card : hand) {
        highCards.push_back(card.getRankValue());
    }
    std::sort(highCards.rbegin(), highCards.rend());
    return makeScore(HandRank::HIGH_CARD, highCards);
}

int PokerEvaluator::compareHands(const HandScore& score1, const HandScore& score2) {
    for (std::size_t i = 0; i < std::min(score1.size(), score2.size()); ++i) {
        if (score1[i] > score2[i]) return 1;
        if (score1[i] < score2[i]) return -1;
    }
    return 0;  // Tie
}

std::vector<std::vector<Card>> PokerEvaluator::generateCombinations(
    const std::vector<Card>& cards) const {
    if (cards.size() != 7) {
        throw std::invalid_argument("Must have exactly 7 cards for combinations");
    }
    
    std::vector<std::vector<Card>> combinations;
    std::vector<Card> currentCombo;
    currentCombo.reserve(5);
    
    generateCombinationsHelper(cards, 5, 0, currentCombo, combinations);
    return combinations;
}

void PokerEvaluator::generateCombinationsHelper(
    const std::vector<Card>& cards,
    int combinationSize,
    int start,
    std::vector<Card>& currentCombo,
    std::vector<std::vector<Card>>& allCombinations) const {
    
    if (currentCombo.size() == static_cast<std::size_t>(combinationSize)) {
        allCombinations.push_back(currentCombo);
        return;
    }
    
    for (std::size_t i = start; i < cards.size(); ++i) {
        currentCombo.push_back(cards[i]);
        generateCombinationsHelper(cards, combinationSize, i + 1, currentCombo, allCombinations);
        currentCombo.pop_back();
    }
}

HandScore PokerEvaluator::makeScore(HandRank handRank, 
                                   const std::vector<int>& tiebreakers) const {
    HandScore score;
    score.reserve(1 + tiebreakers.size());
    score.push_back(static_cast<int>(handRank));
    score.insert(score.end(), tiebreakers.begin(), tiebreakers.end());
    return score;
}

bool PokerEvaluator::isFlush(const std::vector<Card>& hand) const {
    if (hand.empty()) return false;
    
    const std::string& suit = hand[0].getSuit();
    return std::all_of(hand.begin(), hand.end(),
                      [&suit](const Card& card) {
                          return card.getSuit() == suit;
                      });
}

bool PokerEvaluator::isStraight(const std::vector<Card>& hand, int& highCard) const {
    std::set<int> ranks;
    for (const auto& card : hand) {
        ranks.insert(card.getRankValue());
    }
    
    if (ranks.size() != 5) {
        return false;  // Need exactly 5 different ranks
    }
    
    std::vector<int> sortedRanks(ranks.begin(), ranks.end());
    
    // Check for regular straight
    bool isStraightSeq = true;
    for (std::size_t i = 1; i < sortedRanks.size(); ++i) {
        if (sortedRanks[i] != sortedRanks[i-1] + 1) {
            isStraightSeq = false;
            break;
        }
    }
    
    if (isStraightSeq) {
        highCard = sortedRanks.back();
        return true;
    }
    
    // Check for A-2-3-4-5 straight (wheel)
    if (sortedRanks.size() == 5 &&
        sortedRanks[0] == 2 && sortedRanks[1] == 3 && 
        sortedRanks[2] == 4 && sortedRanks[3] == 5 && 
        sortedRanks[4] == 14) {
        highCard = 5;  // In wheel straight, 5 is the high card
        return true;
    }
    
    return false;
}

void PokerEvaluator::analyzeRanks(const std::vector<Card>& hand,
                                 std::array<int, 15>& rankCounts,
                                 std::vector<int>& sortedRanks) const {
    rankCounts.fill(0);
    
    // Count ranks
    for (const auto& card : hand) {
        rankCounts[card.getRankValue()]++;
    }
    
    // Create sorted list of ranks by count (descending) then by rank (descending)
    std::vector<std::pair<int, int>> rankCountPairs;  // (count, rank)
    for (int rank = 2; rank <= 14; ++rank) {
        if (rankCounts[rank] > 0) {
            rankCountPairs.emplace_back(rankCounts[rank], rank);
        }
    }
    
    std::sort(rankCountPairs.begin(), rankCountPairs.end(),
              [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
                  if (a.first != b.first) {
                      return a.first > b.first;  // Higher count first
                  }
                  return a.second > b.second;    // Higher rank first
              });
    
    sortedRanks.clear();
    for (const auto& pair : rankCountPairs) {
        sortedRanks.push_back(pair.second);
    }
}

} // namespace aof


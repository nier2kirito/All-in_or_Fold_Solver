#pragma once

#include "node.hpp"
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <fstream>

namespace mccfr {

/**
 * @brief Manages strategy storage, serialization, and analysis
 */
class StrategyManager {
public:
    /**
     * @brief Load strategies from node map
     * @param nodeMap Map of information sets to nodes
     */
    void loadFromNodeMap(const std::unordered_map<std::string, Node>& nodeMap);

    /**
     * @brief Save strategies to file in human-readable format
     * @param filename Output filename
     * @param includeVisitCounts Include visit statistics
     * @return True if successful
     */
    bool saveToFile(const std::string& filename, bool includeVisitCounts = true) const;

    /**
     * @brief Save strategies in binary format for faster loading
     * @param filename Output filename
     * @return True if successful
     */
    bool saveToBinary(const std::string& filename) const;

    /**
     * @brief Load strategies from text file
     * @param filename Input filename
     * @return True if successful
     */
    bool loadFromFile(const std::string& filename);

    /**
     * @brief Load strategies from binary file
     * @param filename Input filename
     * @return True if successful
     */
    bool loadFromBinary(const std::string& filename);

    /**
     * @brief Get strategy for specific information set
     * @param infoSet Information set identifier
     * @return Strategy vector, or empty if not found
     */
    std::vector<double> getStrategy(const std::string& infoSet) const;

    /**
     * @brief Get all strategies
     * @return Map from information sets to strategies
     */
    const std::unordered_map<std::string, std::vector<double>>& getAllStrategies() const noexcept {
        return strategies_;
    }

    /**
     * @brief Get visit counts for information sets
     * @return Map from information sets to visit counts
     */
    const std::unordered_map<std::string, std::uint64_t>& getVisitCounts() const noexcept {
        return visitCounts_;
    }

    /**
     * @brief Clear all stored strategies
     */
    void clear();

    /**
     * @brief Get number of stored strategies
     */
    std::size_t size() const noexcept { return strategies_.size(); }

    /**
     * @brief Check if strategies are loaded
     */
    bool empty() const noexcept { return strategies_.empty(); }

    // Analysis functions
    
    /**
     * @brief Get information sets sorted by visit count
     * @param descending Sort in descending order
     * @return Vector of (infoSet, visitCount) pairs
     */
    std::vector<std::pair<std::string, std::uint64_t>> 
    getInfoSetsByVisitCount(bool descending = true) const;

    /**
     * @brief Find information sets matching pattern
     * @param pattern Substring to search for
     * @return Vector of matching information set identifiers
     */
    std::vector<std::string> findInfoSets(const std::string& pattern) const;

    /**
     * @brief Get statistics about stored strategies
     */
    struct StrategyStats {
        std::size_t totalInfoSets;
        std::uint64_t totalVisits;
        std::uint64_t maxVisits;
        std::uint64_t minVisits;
        double averageVisits;
    };
    
    StrategyStats getStats() const;

private:
    std::unordered_map<std::string, std::vector<double>> strategies_;
    std::unordered_map<std::string, std::uint64_t> visitCounts_;

    /**
     * @brief Parse strategy line from text file
     * @param line Input line
     * @param infoSet Output information set
     * @param strategy Output strategy
     * @param visitCount Output visit count
     * @return True if parsing successful
     */
    bool parseStrategyLine(const std::string& line, 
                          std::string& infoSet,
                          std::vector<double>& strategy,
                          std::uint64_t& visitCount) const;

    /**
     * @brief Write binary header information
     * @param file Output file stream
     */
    void writeBinaryHeader(std::ofstream& file) const;

    /**
     * @brief Read and validate binary header
     * @param file Input file stream
     * @return True if header is valid
     */
    bool readBinaryHeader(std::ifstream& file) const;
};

} // namespace mccfr


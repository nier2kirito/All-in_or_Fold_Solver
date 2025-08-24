#include "mccfr/strategy_manager.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <iomanip>

namespace mccfr {

void StrategyManager::loadFromNodeMap(const std::unordered_map<std::string, Node>& nodeMap) {
    clear();
    
    for (const auto& [infoSet, node] : nodeMap) {
        strategies_[infoSet] = node.getAverageStrategy();
        visitCounts_[infoSet] = node.getVisitCount();
    }
}

bool StrategyManager::saveToFile(const std::string& filename, bool includeVisitCounts) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    // Write header
    file << "# Strategy Manager Export\n";
    file << "# Total strategies: " << strategies_.size() << "\n";
    file << "# Format: InfoSet: <infoset> [Visits: <count>]\n";
    file << "#         Strategy: <prob1> <prob2> ...\n\n";
    
    // Sort by visit count for better organization
    auto sortedInfoSets = getInfoSetsByVisitCount(true);
    
    for (const auto& [infoSet, visitCount] : sortedInfoSets) {
        auto strategyIt = strategies_.find(infoSet);
        if (strategyIt == strategies_.end()) continue;
        
        file << "InfoSet: " << infoSet;
        if (includeVisitCounts) {
            file << " Visits: " << visitCount;
        }
        file << "\n";
        
        file << "Strategy: ";
        const auto& strategy = strategyIt->second;
        for (std::size_t i = 0; i < strategy.size(); ++i) {
            if (i > 0) file << " ";
            file << std::fixed << std::setprecision(6) << strategy[i];
        }
        file << "\n\n";
    }
    
    return file.good();
}

bool StrategyManager::saveToBinary(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    writeBinaryHeader(file);
    
    // Write number of strategies
    std::uint32_t numStrategies = static_cast<std::uint32_t>(strategies_.size());
    file.write(reinterpret_cast<const char*>(&numStrategies), sizeof(numStrategies));
    
    // Write each strategy
    for (const auto& [infoSet, strategy] : strategies_) {
        // Write info set string length and data
        std::uint32_t infoSetLen = static_cast<std::uint32_t>(infoSet.length());
        file.write(reinterpret_cast<const char*>(&infoSetLen), sizeof(infoSetLen));
        file.write(infoSet.c_str(), infoSetLen);
        
        // Write visit count
        auto visitIt = visitCounts_.find(infoSet);
        std::uint64_t visitCount = (visitIt != visitCounts_.end()) ? visitIt->second : 0;
        file.write(reinterpret_cast<const char*>(&visitCount), sizeof(visitCount));
        
        // Write strategy size and data
        std::uint32_t strategySize = static_cast<std::uint32_t>(strategy.size());
        file.write(reinterpret_cast<const char*>(&strategySize), sizeof(strategySize));
        file.write(reinterpret_cast<const char*>(strategy.data()), 
                  strategySize * sizeof(double));
    }
    
    return file.good();
}

bool StrategyManager::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    clear();
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        std::string infoSet;
        std::vector<double> strategy;
        std::uint64_t visitCount = 0;
        
        if (parseStrategyLine(line, infoSet, strategy, visitCount)) {
            // Read strategy line
            if (std::getline(file, line) && line.substr(0, 9) == "Strategy:") {
                std::istringstream iss(line.substr(10));
                strategy.clear();
                double prob;
                while (iss >> prob) {
                    strategy.push_back(prob);
                }
                
                if (!strategy.empty()) {
                    strategies_[infoSet] = strategy;
                    visitCounts_[infoSet] = visitCount;
                }
            }
        }
    }
    
    return !strategies_.empty();
}

bool StrategyManager::loadFromBinary(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    if (!readBinaryHeader(file)) {
        return false;
    }
    
    clear();
    
    // Read number of strategies
    std::uint32_t numStrategies;
    file.read(reinterpret_cast<char*>(&numStrategies), sizeof(numStrategies));
    if (!file.good()) return false;
    
    // Read each strategy
    for (std::uint32_t i = 0; i < numStrategies; ++i) {
        // Read info set string
        std::uint32_t infoSetLen;
        file.read(reinterpret_cast<char*>(&infoSetLen), sizeof(infoSetLen));
        if (!file.good()) return false;
        
        std::string infoSet(infoSetLen, '\0');
        file.read(&infoSet[0], infoSetLen);
        if (!file.good()) return false;
        
        // Read visit count
        std::uint64_t visitCount;
        file.read(reinterpret_cast<char*>(&visitCount), sizeof(visitCount));
        if (!file.good()) return false;
        
        // Read strategy
        std::uint32_t strategySize;
        file.read(reinterpret_cast<char*>(&strategySize), sizeof(strategySize));
        if (!file.good()) return false;
        
        std::vector<double> strategy(strategySize);
        file.read(reinterpret_cast<char*>(strategy.data()), 
                 strategySize * sizeof(double));
        if (!file.good()) return false;
        
        strategies_[infoSet] = std::move(strategy);
        visitCounts_[infoSet] = visitCount;
    }
    
    return true;
}

std::vector<double> StrategyManager::getStrategy(const std::string& infoSet) const {
    auto it = strategies_.find(infoSet);
    return (it != strategies_.end()) ? it->second : std::vector<double>();
}

void StrategyManager::clear() {
    strategies_.clear();
    visitCounts_.clear();
}

std::vector<std::pair<std::string, std::uint64_t>> 
StrategyManager::getInfoSetsByVisitCount(bool descending) const {
    std::vector<std::pair<std::string, std::uint64_t>> result;
    
    for (const auto& [infoSet, visitCount] : visitCounts_) {
        result.emplace_back(infoSet, visitCount);
    }
    
    std::sort(result.begin(), result.end(),
              [descending](const auto& a, const auto& b) {
                  return descending ? (a.second > b.second) : (a.second < b.second);
              });
    
    return result;
}

std::vector<std::string> StrategyManager::findInfoSets(const std::string& pattern) const {
    std::vector<std::string> matches;
    
    for (const auto& [infoSet, strategy] : strategies_) {
        if (infoSet.find(pattern) != std::string::npos) {
            matches.push_back(infoSet);
        }
    }
    
    std::sort(matches.begin(), matches.end());
    return matches;
}

StrategyManager::StrategyStats StrategyManager::getStats() const {
    StrategyStats stats;
    stats.totalInfoSets = strategies_.size();
    
    if (visitCounts_.empty()) {
        stats.totalVisits = 0;
        stats.maxVisits = 0;
        stats.minVisits = 0;
        stats.averageVisits = 0.0;
        return stats;
    }
    
    stats.totalVisits = 0;
    stats.maxVisits = 0;
    stats.minVisits = UINT64_MAX;
    
    for (const auto& [infoSet, visitCount] : visitCounts_) {
        stats.totalVisits += visitCount;
        stats.maxVisits = std::max(stats.maxVisits, visitCount);
        stats.minVisits = std::min(stats.minVisits, visitCount);
    }
    
    stats.averageVisits = static_cast<double>(stats.totalVisits) / visitCounts_.size();
    
    return stats;
}

// Private methods
bool StrategyManager::parseStrategyLine(const std::string& line,
                                       std::string& infoSet,
                                       std::vector<double>& /* strategy */,
                                       std::uint64_t& visitCount) const {
    if (line.substr(0, 8) != "InfoSet:") {
        return false;
    }
    
    std::size_t start = 9; // After "InfoSet: "
    std::size_t visitsPos = line.find(" Visits:");
    
    if (visitsPos != std::string::npos) {
        infoSet = line.substr(start, visitsPos - start);
        try {
            visitCount = std::stoull(line.substr(visitsPos + 8));
        } catch (const std::exception&) {
            visitCount = 0;
        }
    } else {
        infoSet = line.substr(start);
        visitCount = 0;
    }
    
    return true;
}

void StrategyManager::writeBinaryHeader(std::ofstream& file) const {
    // Magic number and version
    const char magic[] = "STRAT";
    const std::uint32_t version = 1;
    
    file.write(magic, 5);
    file.write(reinterpret_cast<const char*>(&version), sizeof(version));
}

bool StrategyManager::readBinaryHeader(std::ifstream& file) const {
    char magic[6] = {0};
    std::uint32_t version;
    
    file.read(magic, 5);
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    
    return file.good() && 
           std::string(magic) == "STRAT" && 
           version == 1;
}

} // namespace mccfr

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include "mccfr/strategy_manager.hpp"

void printTopStrategies(const mccfr::StrategyManager& manager, int count = 10) {
    auto topInfoSets = manager.getInfoSetsByVisitCount(true);
    
    std::cout << "\n=== Top " << count << " Most Visited Information Sets ===\n";
    
    for (int i = 0; i < std::min(count, static_cast<int>(topInfoSets.size())); ++i) {
        const auto& [infoSet, visitCount] = topInfoSets[i];
        auto strategy = manager.getStrategy(infoSet);
        
        std::cout << "\n" << (i + 1) << ". " << infoSet << "\n";
        std::cout << "   Visits: " << visitCount << "\n";
        std::cout << "   Strategy: [";
        
        for (std::size_t j = 0; j < strategy.size(); ++j) {
            if (j > 0) std::cout << ", ";
            std::cout << std::fixed << std::setprecision(3) << strategy[j];
        }
        std::cout << "]\n";
        
        // Interpret strategy (assuming FOLD=0, ALL_IN=1)
        if (strategy.size() >= 2) {
            std::cout << "   Actions: FOLD=" << std::setprecision(1) 
                      << (strategy[0] * 100) << "%, ALL_IN=" 
                      << (strategy[1] * 100) << "%\n";
        }
    }
}

void analyzePlayerStrategies(const mccfr::StrategyManager& manager) {
    std::cout << "\n=== Player Strategy Analysis ===\n";
    
    for (int player = 0; player < 4; ++player) {
        auto playerInfoSets = manager.findInfoSets("P" + std::to_string(player) + ":");
        
        std::cout << "\nPlayer " << player << ": " << playerInfoSets.size() 
                  << " information sets\n";
        
        if (!playerInfoSets.empty()) {
            // Find most aggressive and most conservative strategies
            double maxAllInProb = 0.0;
            double minAllInProb = 1.0;
            std::string mostAggressive, mostConservative;
            
            for (const auto& infoSet : playerInfoSets) {
                auto strategy = manager.getStrategy(infoSet);
                if (strategy.size() >= 2) {
                    double allInProb = strategy[1];  // ALL_IN probability
                    
                    if (allInProb > maxAllInProb) {
                        maxAllInProb = allInProb;
                        mostAggressive = infoSet;
                    }
                    
                    if (allInProb < minAllInProb) {
                        minAllInProb = allInProb;
                        mostConservative = infoSet;
                    }
                }
            }
            
            std::cout << "  Most aggressive: " << std::setprecision(1) 
                      << (maxAllInProb * 100) << "% all-in\n";
            std::cout << "    " << mostAggressive << "\n";
            std::cout << "  Most conservative: " << (minAllInProb * 100) << "% all-in\n";
            std::cout << "    " << mostConservative << "\n";
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <strategy_file.txt>\n";
        std::cout << "\nAnalyze MCCFR strategy files and provide insights.\n";
        return 1;
    }
    
    std::string filename = argv[1];
    
    try {
        mccfr::StrategyManager manager;
        
        std::cout << "Loading strategies from: " << filename << "\n";
        
        if (!manager.loadFromFile(filename)) {
            std::cerr << "Error: Could not load strategy file: " << filename << std::endl;
            return 1;
        }
        
        // Get overall statistics
        auto stats = manager.getStats();
        
        std::cout << "\n=== Strategy File Statistics ===\n";
        std::cout << "Total information sets: " << stats.totalInfoSets << "\n";
        std::cout << "Total visits: " << stats.totalVisits << "\n";
        std::cout << "Average visits per info set: " 
                  << std::fixed << std::setprecision(1) << stats.averageVisits << "\n";
        std::cout << "Max visits: " << stats.maxVisits << "\n";
        std::cout << "Min visits: " << stats.minVisits << "\n";
        
        // Show top strategies
        printTopStrategies(manager, 10);
        
        // Analyze by player
        analyzePlayerStrategies(manager);
        
        // Look for interesting patterns
        std::cout << "\n=== Pattern Analysis ===\n";
        
        auto pairInfoSets = manager.findInfoSets("AA");
        std::cout << "Pocket pairs (AA example): " << pairInfoSets.size() << " situations\n";
        
        auto suitedInfoSets = manager.findInfoSets("s ");
        std::cout << "Suited hands: " << suitedInfoSets.size() << " situations\n";
        
        auto offsuitInfoSets = manager.findInfoSets("o ");
        std::cout << "Offsuit hands: " << offsuitInfoSets.size() << " situations\n";
        
        std::cout << "\nAnalysis complete!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

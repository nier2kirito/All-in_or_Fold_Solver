#include <iostream>

// Declare test functions from other files
void runCardTests();
void runPokerEvaluatorTests();
void runGameStateTests();
void runMCCFRTests();

int main() {
    try {
        std::cout << "=== Running All Tests ===\n\n";
        
        runCardTests();
        runPokerEvaluatorTests();
        runGameStateTests();
        runMCCFRTests();
        
        std::cout << "\n=== ALL TESTS PASSED SUCCESSFULLY! ===\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "\nTest suite failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
